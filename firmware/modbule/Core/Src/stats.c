/*
 * stats.c
 *
 *  Created on: 8. 2. 2024
 *      Author: Jakub Ladman ladmanj@volny.cz
 *      SPDX-License-Identifier: GPL-2.0-or-later */

#ifdef DEBUG
#include <stdint.h>

int32_t filter[2] = {0,0};
uint16_t stats[8];

int16_t IIR_Filter(int32_t *filter, int32_t sample, uint32_t bits)
{
        int32_t local_sample = (int32_t) sample << 16;
        *filter += (local_sample - *filter) >> bits;
        return (int16_t) ((*filter + 0x8000) >> 16);
}

void record_max_avg(uint16_t out[], uint16_t meas) {
	out[0] = (meas > stats[0]) ? meas : stats[0];
	out[1] = IIR_Filter(&filter[0], meas, 8);
}
#endif
