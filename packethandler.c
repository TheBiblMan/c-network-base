/*
 * packetreader.c
 *
 *  Created on: 16 Sep 2017
 *      Author: Bibl
 */

#include <stdio.h>
#include <winsock2.h>
#include "packetqueue.h"
#include "packethandler.h"

int is_live_connection(struct connection *conn) {
	return conn != NULL;
}

void dissolve_connection(struct connection **connptr) {
	/* close the socket connection */
	struct connection *conn = *connptr;
	SOCKET socket = conn->socket;
	closesocket(socket);

	/* free the actual struct in memory */
	GlobalFree(conn);

	/* set the pointer to the connection pointer
	 * to NULL indicating that the connection at
	 * that index is free. */
	memset(connptr, 0, sizeof(struct connection *));
}

struct connection *create_connection(struct connection *connections[], SOCKET *accept_socket) {
	/* we are storing pointers to connection
	 * structs in our array as these structs
	 * are very large in memory (few KB). most
	 * of this memory is occupied by the underlying
	 * read and write buffers. we may change
	 * them to be pointers to the buffers in
	 * the future, so that they can be dynamically
	 * resized as well. */
	struct connection *conn = GlobalAlloc(GPTR, sizeof(struct connection));

	if(conn != NULL) {
		memset(conn, 0, sizeof(struct connection));

		conn->socket = *accept_socket;

		conn->read_buf.buf = conn->__read_buf;
		conn->read_buf.len = DATA_BUF_LEN;

		conn->write_buf.buf = conn->__write_buf;
		conn->write_buf.len = DATA_BUF_LEN;

		/* the connections array is maintained such
		 * that when a connection is closed, that
		 * space in the array is zeroed. when a new
		 * connection is created, that memory is
		 * set to a pointer to the connection struct. */
		for (int i = 0; i < MAX_CLIENTS; i++) {
			struct connection *c = connections[i];
			/* zeroed memory, no connection in this position */
			if (!is_live_connection(c)) {
				conn->idx = i;
				connections[i] = conn;
				return conn;
			}
		}
	}

	return NULL;
}

void *handler_start(void *arg) {
	struct server_config *config = arg;
	/* initialise the server state */
	config->is_live = 1;
	config->is_error = 0;

	SOCKET listen_socket, accept_socket;

	/* allocate our client connections (pointers) */
	struct connection *connections[MAX_CLIENTS];
	memset(connections, 0, sizeof(connections));

	FD_SET write_set, read_set;
	DWORD num_descs_ready;
	DWORD non_blocking = 1;
	DWORD io_bytes = 0;
	DWORD flags = 0;

	/* server socket/winsock initialisation */
	if((listen_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
		config->error_msg = "WSASocket failed";
		goto error;
	}

	if(bind(listen_socket, (PSOCKADDR) &config->addr, sizeof(config->addr)) == SOCKET_ERROR) {
		config->error_msg = "bind failed";
		goto error;
	}

	if(listen(listen_socket, 5)) {
		config->error_msg = "listen failed";
		goto error;
	}

	/* put the socket into non blocking mode so
	 * that we can use select(). */
	if(ioctlsocket(listen_socket, FIONBIO, &non_blocking) == SOCKET_ERROR) {
		config->error_msg = "ioctlsocket/non blocking mode on listen socket failed";
		goto error;
	}

	/* main event loop */
	while(TRUE) {
		/* clear our sets before each select() call */
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);

		/* set the bit for our server listen socket. notifications
		 * on this socket indicate that a client is trying to
		 * connect (i.e. we need to accept() it). */
		FD_SET(listen_socket, &read_set);

		/* for all active connections, set the read bit unconditionally. */
		for(int i=0; i < MAX_CLIENTS; i++) {
			struct connection *conn = connections[i];

			if(is_live_connection(conn)) {
				SOCKET client_socket = conn->socket;
				FD_SET(client_socket, &read_set);

				// TODO: set for writing when packets are pending.
			}
		}

		/* blocking call, returns when one of the indicated
		 * file descriptors in either the reader or write set
		 * become available. timeout is set to NULL, i.e. no
		 * timeout. */
		num_descs_ready = select(0, &read_set, &write_set, NULL, NULL);

		/* error checking (fatal) */
		if(num_descs_ready == SOCKET_ERROR) {
			// TODO: maybe we need to disconnect all clients?
			//       would it work? the failure could be due
			//       to an error on the write streams.
			config->error_msg = "select failed";
			goto error;
		}

		/* set on the listen socket, i.e. accept */
		if(FD_ISSET(listen_socket, &read_set)) {
			num_descs_ready--;

			accept_socket = accept(listen_socket, NULL, NULL);

			if(accept_socket == INVALID_SOCKET) {
				if(WSAGetLastError() != WSAEWOULDBLOCK) {
					config->error_msg = "accept failed";
					goto error;
				}
			} else {
				/* set the connected client socket to be
				 * non blocking as well. */
				non_blocking = 1;

				if(ioctlsocket(accept_socket, FIONBIO, &non_blocking) == SOCKET_ERROR) {
					config->error_msg = "ioctlsocket/non blocking mode on accepted socket failed";
					goto error;
				}

				/* try to create the connection struct. note that
				 * create_connection also adds the created connection
				 * to the connections array. */
				struct connection *conn;
				if(!(conn = create_connection(connections, &accept_socket))) {
					printf("could not create connection to %d\n", accept_socket);
				} else {
					printf("connected on %d @%d\n", conn->socket, conn->idx);
				}
			}
		}

		/* go through all the connections, but stop early
		 * if we have exhausted our indicated descriptors. */
		for(int i=0; num_descs_ready > 0; i++) {
			struct connection *conn = connections[i];

			if(is_live_connection(conn)) {

				/* note for the following: WSAEWOULDBLOCK is non fatal. */

				/* read on the client socket. */
				if(FD_ISSET(conn->socket, &read_set)) {
					printf("[R] it's a read on %d.\n", conn->socket);
					num_descs_ready--;

					flags = io_bytes = 0;

					if(WSARecv(conn->socket, &conn->read_buf, 1, &io_bytes, &flags, NULL, NULL) == SOCKET_ERROR) {
						if(WSAGetLastError() != WSAEWOULDBLOCK) {
							printf("WSARecv failed with %d on %d\n", WSAGetLastError(), conn->socket);
							dissolve_connection(&conn);
						}
						continue;
					} else {
						// TODO: parse packet
						printf("  of %ld bytes.\n", io_bytes);
						if(io_bytes == 0) {
							// closed
							dissolve_connection(&conn);
							continue;
						}
						// else ok
					}
				}

				/* write on the client socket. */
				if(FD_ISSET(conn->socket, &write_set)) {
					printf("[W] it's a write on %d.\n", conn->socket);
					num_descs_ready--;

					if(WSASend(conn->socket, &conn->write_buf, 1, &io_bytes, 0 , NULL, NULL) == SOCKET_ERROR) {
						if(WSAGetLastError() != WSAEWOULDBLOCK) {
							printf("WSASend failed with %d on %d\n", WSAGetLastError(), conn->socket);
							dissolve_connection(&conn);
						}
						continue;
					} else {
						// TODO: write packet
						printf("  of %ld bytes.\n", io_bytes);
					}
				}
			}
		}
	}

	/* the loop should not exit to here */
	printf("fatal: loop leaked (wth)\n");
	exit(2);

	error: {
		config->is_live = 0;
		config->is_error = 1;
	}
	return NULL;
}
