///*
// * packetqueue.c
// *
// *  Created on: 11 Sep 2017
// *      Author: Bibl
// */
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include "packetqueue.h"
//#include "packetconsts.h"
//
//inline void lock(struct packet_queue *queue) {
////	printf("%p\n", queue->mutex);
//#if SHOULD_SYNC
//		pthread_mutex_lock(queue->mutex);
//#endif
//}
//
//inline void unlock(struct packet_queue *queue) {
//#if SHOULD_SYNC
//		pthread_mutex_unlock(queue->mutex);
//#endif
//}
//
//struct packet_queue *new_packet_queue(void) {
//	struct packet_queue *queue = malloc(sizeof(struct packet_queue));
//	memset(queue, 0, sizeof(struct packet_queue));
//#if SHOULD_SYNC
//		queue->mutex = malloc(sizeof(pthread_mutex_t));
//		pthread_mutex_init(queue->mutex, NULL);
//#endif
//	return queue;
//}
//
//void free_packet_queue(struct packet_queue *queue) {
//#if SHOULD_SYNC
//		pthread_mutex_destroy(queue->mutex);
//		free(queue->mutex);
//#endif
//	struct queue_node *cur = queue->queue_head;
//	while(cur) {
//		struct queue_node *next = cur->next;
//		free(cur);
//		cur = next;
//	}
//
//	free(queue);
//}
//
///* return 0 for failure
// * else returns a non zero value. */
//int add_packet(struct packet_queue *queue, struct packet_t *packet) {
//	struct queue_node *new_node = malloc(sizeof(struct queue_node));
//	if(!new_node) {
//		return 0;
//	}
//
//	lock(queue);
//
//	new_node->next = 0;
//	new_node->packet = packet;
//
//	if(queue->queue_head) {
//		/* append to tail and set the new tail */
//		queue->queue_tail->next = new_node;
//		queue->queue_tail = new_node;
//	} else {
//		/* empty queue */
//		queue->queue_head = queue->queue_tail = new_node;
//	}
//
//	unlock(queue);
//
//	return 1;
//}
//
///* return NULL if there are no pending
// * packets in the queue, else a pointer
// * to the node. */
//struct packet_t *poll_next(struct packet_queue *queue) {
//	lock(queue);
//
//	if(queue->queue_head) {
//		struct queue_node *old_head = queue->queue_head;
//		/* 1. store the next node in the queue.
//		 * 2. free the head node in the queue.
//		 * 3. set the head to the stored next node.
//		 * 4. return the packet. */
//
//		struct packet_t *packet = old_head->packet;
//		queue->queue_head = old_head->next;
//
//		free(old_head);
//		unlock(queue);
//
//		return packet;
//	} else {
//		/* empty queue */
//		unlock(queue);
//		return 0;
//	}
//}
//
///* returns a non zero integer if there
// * are any packets in the queue, else
// * returns 0. */
//int has_pending(struct packet_queue *queue) {
//	int ret;
//
//	lock(queue);
//	ret = queue->queue_head ? 1 : 0;
//	unlock(queue);
//
//	return ret;
//}
//
//int get_size(struct packet_queue *queue) {
//	lock(queue);
//	int len = 0;
//	struct queue_node *cur = queue->queue_head;
//	while(cur) {
//		len++;
//		cur = cur->next;
//	}
//	unlock(queue);
//	return len;
//}
