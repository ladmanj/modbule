/*
 * fsm.c
 *
 *  Created on: 21. 8. 2017
 *      Author: Jakub Ladman ladmanj@volny.cz
 *      SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdint.h>
//#include "console.h"
#include "fsm.h"

#define FALSE	0
#define TRUE	!FALSE

#define NULL (void*)(0)

//#define DBGPFUNC()	xprintf("%s\n", __FUNCTION__)
#define DBGPFUNC() {}

void fsm_init(machine *instance, state_fn *start, void *data) {
	instance->state.next = start;
	instance->state.time = 0;
	instance->state.data = data;
}

void fsm_tick(machine *instance) {
	if (instance->state.next) {
		instance->state.next(&instance->state);
		instance->state.time++;
	}
}

