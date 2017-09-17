/*
 * testutils.c
 *
 *  Created on: 17 Sep 2017
 *      Author: Bibl
 */

#include "testutils.h"
#include <stdlib.h>
#include <stdio.h>

void assert(int got, int expected, char *msg) {
	if(got != expected) {
		printf("fatal: assert failed, expected: %d, got: %d, msg> %s\n", expected, got, msg);
		exit(1);
	}
}

