///*
// * packetqueue.h
// *
// *  Created on: 12 Sep 2017
// *      Author: Bibl
// */
//
//#ifndef PACKETQUEUE_H_
//#define PACKETQUEUE_H_
//
//#define SHOULD_SYNC 1
//
//#if SHOULD_SYNC
//#include <pthread.h>
//#endif
//
//typedef struct queue_node *queue_node_ptr;
//struct queue_node {
//	struct packet_t *packet;
//	queue_node_ptr next;
//};
//
//struct packet_queue {
//	queue_node_ptr queue_head;
//	queue_node_ptr queue_tail;
//#if SHOULD_SYNC
//	pthread_mutex_t *mutex;
//#endif
//};
//
//void lock(struct packet_queue *queue);
//void unlock(struct packet_queue *queue);
//
//struct packet_queue *new_packet_queue(void);
//void free_packet_queue(struct packet_queue *);
//
//int add_packet(struct packet_queue *, struct packet_t *);
//struct packet_t *poll_next(struct packet_queue *);
//int has_pending(struct packet_queue *);
//int get_size(struct packet_queue *);
//
//#endif /* PACKETQUEUE_H_ */
