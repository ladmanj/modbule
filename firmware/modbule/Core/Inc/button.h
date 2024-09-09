/*
 * button.h
 *
 *  Created on: 26. 8. 2017
 *      Author: Jakub Ladman ladmanj@volny.cz
 *      SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "fsm.h"

state_fn btn_state0_idle, btn_state1_down, btn_state2_up, btn_state3_down2,
		btn_state4_long, btn_state5_err;

typedef uint8_t btn_fn(uint8_t id);
typedef uint32_t time_fn(void);
typedef void btn_callback_fn(uint8_t id);

typedef struct {
	uint8_t id;
	machine fsm;
	btn_fn *read_btn;
	time_fn *now;
	uint32_t start_time;
	uint32_t stop_time;
	uint32_t debouncetck;
	uint32_t clicktck;
	uint32_t presstck;
	uint8_t is_pressed;
	btn_callback_fn *click;
	btn_callback_fn *doubleclick;
	btn_callback_fn *press_start;
	btn_callback_fn *press_hold;
	btn_callback_fn *press_release;
} buttonstruct;

#endif /* BUTTON_H_ */
