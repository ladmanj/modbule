/*
 * button.c
 *
 *  Created on: 26. 8. 2017
 *      Author: Jakub Ladman ladmanj@volny.cz
 *      SPDX-License-Identifier: GPL-2.0-or-later */

#include <stdint.h>
//#include "fsm.h"
#include "button.h"

#ifndef FALSE
#define FALSE	0
#endif

#ifndef TRUE
#define TRUE	!FALSE
#endif

#ifndef NULL
#define NULL (void*)(0)
#endif

//#define DBGPFUNC()	xprintf("%s\n", __FUNCTION__)
#define DBGPFUNC() {}

void btn_state0_idle(state_t * state) {
	buttonstruct *btn = (buttonstruct*) state->data;
	DBGPFUNC();

	if ((!btn->now) || (!btn->read_btn))
	{
		state->next = btn_state5_err;
	}
	else if (btn->read_btn(btn->id)) {
		state->next = btn_state1_down;
		btn->start_time = btn->now();
	}
}

void btn_state1_down(state_t * state) {
	buttonstruct *btn = (buttonstruct*) state->data;
	DBGPFUNC();

	if (!btn->read_btn(btn->id)) {
		if ((btn->now() - btn->start_time) < btn->debouncetck) {
			state->next = btn_state0_idle;
			return;
		}
		state->next = btn_state2_up;
		btn->stop_time = btn->now();
	} else {
		if ((btn->now() - btn->start_time) > btn->presstck) {
			btn->is_pressed = TRUE;

			if (btn->press_start) {
				btn->press_start(btn->id);
			}

			if (btn->press_hold) {
				btn->press_hold(btn->id);
			}
			state->next = btn_state4_long;
		}
	}
}

void btn_state2_up(state_t * state) {
	buttonstruct *btn = (buttonstruct*) state->data;
	DBGPFUNC();

	if ((!btn->doubleclick) //double click callback not valid so only possibility is single click
	|| ((btn->now() - btn->start_time) > btn->clicktck)) { //time for second click is out
		if (btn->click) {
			btn->click(btn->id);
		}
		state->next = btn_state0_idle;
	} else {
		if (btn->read_btn(btn->id)) {
			if ((btn->now() - btn->stop_time) > btn->debouncetck) {
				state->next = btn_state3_down2;
				btn->start_time = btn->now();
			}
		}
	}
}

void btn_state3_down2(state_t * state) {
	buttonstruct *btn = (buttonstruct*) state->data;
	DBGPFUNC();

	if (!btn->read_btn(btn->id)) {
		if ((btn->now() - btn->start_time) > btn->debouncetck) {
			if (btn->doubleclick)
				btn->doubleclick(btn->id);
			state->next = btn_state0_idle;
		}
	}
}

void btn_state4_long(state_t * state) {
	buttonstruct *btn = (buttonstruct*) state->data;
	DBGPFUNC();

	if (!btn->read_btn(btn->id)) {
		btn->is_pressed = FALSE;
		if (btn->press_release)
			btn->press_release(btn->id);
		state->next = btn_state0_idle;
	} else {
		btn->is_pressed = TRUE;
		if (btn->press_hold)
			btn->press_hold(btn->id);
	}

}

void btn_state5_err(state_t * state) {
	DBGPFUNC();
}
