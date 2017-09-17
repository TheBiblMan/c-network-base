/*
 * packetconsts.h
 *
 *  Created on: 11 Sep 2017
 *      Author: Bibl
 */

#ifndef PACKETCONSTS_H_
#define PACKETCONSTS_H_

/* packet structure:
 * _________________________________________
 *|               |            |            |
 *| length:ushort | flags:byte | data:bytes |
 *|_______________|____________|____________|
 *
 * all of this data is counted as as the packet
 * size, i.e. the length of the data field is
 * length - sizeof(short) - sizeof(char) bytes
 * long.
 */

#define MAX_PACKET_LEN 1024 * 2

struct packet_t {
	unsigned short int length;
	unsigned char flags;
	unsigned short data_len;
	char *data;
};
#endif /* PACKETCONSTS_H_ */
