/*
 * spi_helper.h
 *
 *  Created on: Aug 25, 2024
 *      Author: ladmanj
 */

#ifndef INC_SPI_HELPER_H_
#define INC_SPI_HELPER_H_

#include "main.h"
typedef struct
{
	SPI_HandleTypeDef *spi_port;
	GPIO_TypeDef	  *ncs_gpio_port;
	uint16_t		   ncs_gpio_pin;
} spi_handle;

#endif /* INC_SPI_HELPER_H_ */
