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
	struct connection *conn = GlobalAlloc(GPTR, sizeof(struct connection));

	if(conn != NULL) {
		memset(conn, 0, sizeof(struct connection));

		conn->socket = *accept_socket;

		conn->read_buf.buf = conn->__read_buf;
		conn->read_buf.len = DATA_BUF_LEN;

		conn->write_buf.buf = conn->__write_buf;
		conn->write_buf.len = DATA_BUF_LEN;

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
	config->is_live = 1;
	config->is_error = 0;

	SOCKET listen_socket, accept_socket;

	struct connection *connections[MAX_CLIENTS];
	memset(connections, 0, sizeof(connections));

	FD_SET write_set, read_set;
	DWORD num_descs_ready;
	DWORD non_blocking = 1;
	DWORD io_bytes = 0;
	DWORD flags = 0;

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

	if(ioctlsocket(listen_socket, FIONBIO, &non_blocking) == SOCKET_ERROR) {
		config->error_msg = "ioctlsocket/non blocking mode on listen socket failed";
		goto error;
	}

	while(TRUE) {
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);

		FD_SET(listen_socket, &read_set);

		for(int i=0; i < MAX_CLIENTS; i++) {
			struct connection *conn = connections[i];

			if(is_live_connection(conn)) {
				SOCKET client_socket = conn->socket;
				FD_SET(client_socket, &read_set);

			}
		}

		num_descs_ready = select(0, &read_set, &write_set, NULL, NULL);

		if(num_descs_ready == SOCKET_ERROR) {
			config->error_msg = "select failed";
			goto error;
		}

		if(FD_ISSET(listen_socket, &read_set)) {
			num_descs_ready--;

			accept_socket = accept(listen_socket, NULL, NULL);

			if(accept_socket == INVALID_SOCKET) {
				if(WSAGetLastError() != WSAEWOULDBLOCK) {
					config->error_msg = "accept failed";
					goto error;
				}
			} else {
				non_blocking = 1;

				if(ioctlsocket(accept_socket, FIONBIO, &non_blocking) == SOCKET_ERROR) {
					config->error_msg = "ioctlsocket/non blocking mode on accepted socket failed";
					goto error;
				}

				struct connection *conn;
				if(!(conn = create_connection(connections, &accept_socket))) {
					printf("could not create connection to %d\n", accept_socket);
				} else {
					printf("connected on %d @%d\n", conn->socket, conn->idx);
				}
			}
		}

		struct connection *conn;
		for(int i=0; num_descs_ready > 0; i++) {
			conn = connections[i];
			if(is_live_connection(conn)) {

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
						printf("  of %ld bytes.\n", io_bytes);
						if(io_bytes == 0) {
							// closed
							dissolve_connection(&conn);
							continue;
						}
						// else ok
					}
				}

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
						printf("  of %ld bytes.\n", io_bytes);
					}
				}
			}
		}
	}

	error: {
		config->is_live = 0;
		config->is_error = 1;
	}
	return NULL;
}
