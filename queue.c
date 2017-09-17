/*
 * queue.c
 *
 *  Created on: 17 Sep 2017
 *      Author: Bibl
 */

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "queue.h"

inline void queue_lock(queue_ptr qp) {
//	printf("%p\n", queue->mutex);
#if SHOULD_SYNC
		pthread_mutex_lock(qp->mutex);
#endif
}

inline void queue_unlock(queue_ptr p) {
#if SHOULD_SYNC
		pthread_mutex_unlock(p->mutex);
#endif
}

queue_ptr new_fifo_queue() {
	queue_ptr q = malloc(sizeof(struct queue));
	memset(q, 0, sizeof(struct queue));

#if SHOULD_SYNC
		q->mutex = malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(q->mutex, NULL);
#endif
	return q;
}

void free_queue(queue_ptr q) {
#if SHOULD_SYNC
		pthread_mutex_destroy(q->mutex);
		free(q->mutex);
#endif
	struct queue_node *cur = q->queue_head;
	while(cur) {
		struct queue_node *next = cur->next;
		free(cur);
		cur = next;
	}

	free(q);
}

/* return 0 for failure
 * else returns a non zero value. */
int queue_push(queue_ptr q, generic_data_ptr data) {
	struct queue_node *new_node = malloc(sizeof(struct queue_node));
	if(!new_node) {
		return 0;
	}

	queue_lock(q);

	new_node->next = 0;
	new_node->data = data;

	if(q->queue_head) {
		/* append to tail and set the new tail */
		q->queue_tail->next = new_node;
		q->queue_tail = new_node;
	} else {
		/* empty queue */
		q->queue_head = q->queue_tail = new_node;
	}

	queue_unlock(q);

	return 1;
}

/* return NULL if there are no pending
 * items in the queue, else a pointer
 * to the data. */
generic_data_ptr queue_pop(queue_ptr q) {
	queue_lock(q);

	if(q->queue_head) {
		queue_node_ptr old_head = q->queue_head;
		/* 1. store the old head.
		 * 2. store the data from the old head.
		 * 3. set the head to the next node.
		 * 4. return the data. */

		generic_data_ptr data = old_head->data;
		q->queue_head = old_head->next;

		free(old_head);
		queue_unlock(q);

		return data;
	} else {
		/* empty queue */
		queue_unlock(q);
		return 0;
	}
}

/* returns a non zero integer if there
 * are any items in the queue, else
 * returns 0. */
int queue_has_pending(queue_ptr queue) {
	int ret;

	queue_lock(queue);
	ret = queue->queue_head ? 1 : 0;
	queue_unlock(queue);

	return ret;
}

int queue_get_size(queue_ptr queue) {
	queue_lock(queue);
	int len = 0;
	queue_node_ptr cur = queue->queue_head;
	while(cur) {
		len++;
		cur = cur->next;
	}
	queue_unlock(queue);
	return len;
}
