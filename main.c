/*
 * main.c
 *
 *  Created on: 12 Sep 2017
 *      Author: Bibl
 */
#include <stdio.h>
#include <winsock2.h>
#include "packetconsts.h"
#include "packetqueue.h"
#include "packethandler.h"

void my_exit_func() {
	printf("exited.\n");
}

int main(int argc, char *argv[]) {
	struct server_config config;
	WSAStartup(MAKEWORD(2, 2), &config.wsadata);

	config.exit_callback = my_exit_func;

	config.addr.sin_family = AF_INET;
	config.addr.sin_port = htons(5150);
	config.addr.sin_addr.s_addr = htonl(INADDR_ANY);

	handler_start(&config);

	if(config.is_error) {
		printf("exited with error: %d\n", WSAGetLastError());
		printf(" msg: %s\n", config.error_msg);
	}


	return 0;
}
