/*
 * proglogic.h
 *
 *  Created on: 18. 7. 2020
 *      Author: Jakub Ladman ladmanj@volny.cz
 *      SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef PROGLOGIC_H_
#define PROGLOGIC_H_

#include <stdint.h>

#define MAXLOGIC 8
typedef struct
{
	uint16_t invert;
	uint16_t enable;
} input_op;

typedef struct
{
	uint8_t invert;
	uint8_t output;
} output_op;

typedef struct
{
	input_op	pli[MAXLOGIC];
	output_op	plo;
} pl_config;

uint8_t proglogic(uint8_t inputs, pl_config *cfg);

#endif /* PROGLOGIC_H_ */
