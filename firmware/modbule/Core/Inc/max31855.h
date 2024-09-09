/*
 * max31855.h
 *
 *  Created on: 19. 7. 2020
 *      Author: Jakub Ladman ladmanj@volny.cz
 *      SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef MAX31855_H_
#define MAX31855_H_

#include "spi_helper.h"

typedef struct {
	int32_t tc_temp :14; // D[31:18] 14-Bit Thermocouple Temperature Data	These bits contain the signed 14-bit thermocouple temperature value. See Table 4.
	int32_t cj_temp :12;// D[15:4] 12-Bit Internal Temperature 	Data These bits contain the signed 12-bit value of the reference junction temperature. See Table 5.

	uint8_t bytes[4];

} tc_data_max31855;

tc_data_max31855 read_tc(spi_handle *handle);
void spi_cfg_max31855(spi_handle *handle);
uint8_t detect_max31855(tc_data_max31855 *tc, spi_handle *handle);

#endif /* MAX31855_H_ */
