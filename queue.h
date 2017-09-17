/*
 * queue.h
 *
 *  Created on: 17 Sep 2017
 *      Author: Bibl
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#define SHOULD_SYNC 1

#if SHOULD_SYNC
#include <pthread.h>
#endif

typedef void * generic_data_ptr;

typedef struct queue_node *queue_node_ptr;

struct queue_node {
	generic_data_ptr data;
	queue_node_ptr next;
};

typedef struct queue {
	queue_node_ptr queue_head;
	queue_node_ptr queue_tail;
#if SHOULD_SYNC
	pthread_mutex_t *mutex;
#endif
} *queue_ptr;

void queue_lock(queue_ptr);
void queue_unlock(queue_ptr);

// TODO: need to distinguish between lifo
//       and fifo and implement.
queue_ptr new_fifo_queue(void);
void free_queue(queue_ptr);

int queue_push(queue_ptr, generic_data_ptr);
generic_data_ptr queue_pop(queue_ptr);
int queue_has_pending(queue_ptr);
int queue_get_size(queue_ptr);

#endif /* QUEUE_H_ */
