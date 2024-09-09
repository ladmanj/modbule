/*
 * fsm.h
 *
 *  Created on: 25. 8. 2017
 *      Author: Jakub Ladman ladmanj@volny.cz
 *      SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef FSM_H_
#define FSM_H_

struct state;
typedef void state_fn(struct state *);

typedef struct state {
	state_fn * next;
	uint16_t time; // data
	void * data;
} state_t;

typedef struct {
	state_t state;
} machine;

void fsm_init(machine *instance, state_fn *start, void *data);
void fsm_tick(machine *instance);

#endif /* FSM_H_ */
