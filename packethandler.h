/*
 * packethandler.h
 *
 *  Created on: 17 Sep 2017
 *      Author: Bibl
 */

#ifndef PACKETHANDLER_H_
#define PACKETHANDLER_H_

#include <winsock2.h>
#include "packetqueue.h"

#define MAX_CLIENTS FD_SETSIZE-1

struct server_config {
	WSADATA wsadata;
	SOCKADDR_IN addr;
	void (*exit_callback)(void);

	/* server controlled variables,
	 *  is_live indicates whether the
	 *  server is currently running.
	 *
	 *  is_error indicates that the
	 *  handler exited because of an
	 *  error. if it is set, error_msg
	 *  contains a description of the
	 *  error.
	 */
	int is_live;
	int is_error;
	char *error_msg;
};

#define DATA_BUF_LEN 1024 * 8

struct connection {
	int idx;
	SOCKET socket;
	char __write_buf[DATA_BUF_LEN], __read_buf[DATA_BUF_LEN];
	WSABUF write_buf, read_buf;

	struct packet_queue send_queue, recv_queue;
};


int is_live_connection(struct connection *conn);
void dissolve_connection(struct connection **connptr);
struct connection *create_connection(struct connection *connections[], SOCKET *accept_socket);
// thread entry method
void *handler_start(void *arg);

#endif /* PACKETHANDLER_H_ */
