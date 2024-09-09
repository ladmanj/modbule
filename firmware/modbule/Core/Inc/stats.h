/*
 * stats.h
 *
 *  Created on: 8. 2. 2024
 *      Author: Jakub Ladman ladmanj@volny.cz
 *      SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef INC_STATS_H_
#define INC_STATS_H_

extern int32_t filter[2];
extern uint16_t stats[8];

int16_t IIR_Filter(int32_t *filter, int32_t sample, uint32_t bits);
void record_max_avg(uint16_t out[], uint16_t meas);

#endif /* INC_STATS_H_ */
