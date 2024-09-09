/*
 * max31855.c
 *
 *  Created on: 19. 7. 2020
 *      Author: Jakub Ladman ladmanj@volny.cz
 *      SPDX-License-Identifier: GPL-2.0-or-later */

#include "max31855.h"

tc_data_max31855 read_tc(spi_handle *handle) {
	tc_data_max31855 tc;

	handle->ncs_gpio_port->BRR = (uint32_t)handle->ncs_gpio_pin;
	HAL_SPI_Receive(handle->spi_port, tc.bytes, sizeof(tc.bytes), 2);
	handle->ncs_gpio_port->BSRR = (uint32_t)handle->ncs_gpio_pin;


	tc.tc_temp = (int32_t) ((tc.bytes[0] << 6) + (tc.bytes[1] >> 2));
	tc.cj_temp = (int32_t) ((tc.bytes[2] << 4) + (tc.bytes[3] >> 4));
	return tc;
}

void spi_cfg_max31855(spi_handle *handle)
{
//	handle->spi_port->Init.CLKPolarity = SPI_POLARITY_LOW;
//	handle->spi_port->Init.CLKPhase = SPI_PHASE_1EDGE;
	handle->spi_port->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
	if (HAL_SPI_Init(handle->spi_port) != HAL_OK)
	{
	   Error_Handler();
	}
}

uint8_t detect_max31855(tc_data_max31855 *tc, spi_handle *spi)
{
	if(tc && spi)
		*tc = read_tc(spi);

	return ((~tc->bytes[1] & (1 << 1)) && (~tc->bytes[3] & (1 << 3))
			&& (tc->bytes[0] | tc->bytes[1] | tc->bytes[2] | tc->bytes[3]));
}
