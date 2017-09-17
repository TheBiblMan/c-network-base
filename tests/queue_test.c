/*
 * queue_test.c
 *
 *  Created on: 17 Sep 2017
 *      Author: Bibl
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../queue.h"
#include "queue_test.h"
#include "testutils.h"

// TODO: must be changed when lifo queues are added
void test_generic_queue() {
	queue_ptr queue = new_fifo_queue();
	assert(queue_has_pending(queue), 0, "empty queue pending");
	assert(queue_get_size(queue), 0, "empty queue size");

	int data_count = 100;

	int my_data[data_count];
	memset(my_data, 0, sizeof(my_data));

	for(int i=0; i < data_count; i++) {
		my_data[i] = rand();
		queue_push(queue, (void *) &my_data[i]);
	}

	/* has_pending returns non 0 for success, so we flip */
	assert(!queue_has_pending(queue), 0, "full queue has_pending");
	assert(queue_get_size(queue), data_count, "full queue size");

	for(int i=0; i < data_count-1; i++) {
		assert(!queue_has_pending(queue), 0, "pre-half full has_pending");
		assert(queue_get_size(queue), data_count - i, "pre-half queue size");

		int *data_ptr = queue_pop(queue);
		assert(*data_ptr, my_data[i], "pop cmp in for");

		assert(!queue_has_pending(queue), 0, "post-half full has_pending");
		assert(queue_get_size(queue), data_count - i - 1, "post-half queue size");
	}

	assert(!queue_has_pending(queue), 0, "end empty queue pending");
	assert(queue_get_size(queue), 1, "end empty queue size");

	int *data_ptr = queue_pop(queue);
	assert(*data_ptr, my_data[data_count-1], "pop cmp at end");


	assert(queue_has_pending(queue), 0, "end empty queue pending");
	assert(queue_get_size(queue), 0, "end empty queue size");

	printf("passed.\n");
}
