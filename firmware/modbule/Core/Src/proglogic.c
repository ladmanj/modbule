/*
 * proglogic.c
 *
 *  Created on: 18. 7. 2020
 *      Author: Jakub Ladman ladmanj@volny.cz
 *      SPDX-License-Identifier: GPL-2.0-or-later */

#include "proglogic.h"
#include <string.h>


__attribute__((optimize("unroll-loops")))
uint8_t proglogic(uint8_t inputs, pl_config *cfg)
{
	if(!cfg)
		return 0;

	uint8_t temp = 0;
	uint16_t data;
	int i;
	for(i=0;i<MAXLOGIC;i++)
	{
		data = ((inputs << 8 | cfg->plo.output) ^ cfg->pli[i].invert) & cfg->pli[i].enable;
		temp |= data ? 1 << i : 0;
	}
	cfg->plo.output = temp ^ cfg->plo.invert;
	return cfg->plo.output;
}
