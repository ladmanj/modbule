/*
 * flash.h
 *
 *  Created on: 27. 8. 2017
 *      Author: Jakub Ladman ladmanj@volny.cz
 *      SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FLASH_H_
#define FLASH_H_

#define DSCRD 0
#define OCCUP 1
#define ERROR 2
#define FREE  3
#define FULL  4

#define PGLEN	2048

enum {
	FLASH_BOOL_FALSE=0,
	FLASH_BOOL_TRUE=~0
};
#include "iostruct.h"

typedef struct
{
	uint16_t valid;
	uint16_t free;
	user_cfg_data data;
} flash_cfg_data __attribute__ ((aligned(8)));

enum {
	FLASH_CFG_STORED,
	FLASH_CFG_UNCHANGED,
	FLASH_CFG_ERROR = -1
};

extern __attribute__((section(".wrflash"))) flash_cfg_data writable_flash_area;

flash_cfg_data *find_last_config(flash_cfg_data *ptr, uint8_t *status);
//uint8_t write_data_flash(flash_cfg_data *ram_data, flash_cfg_data *flash_ptr);
uint8_t write_data_flash(uint64_t *ram_data, uint64_t *flash_ptr, uint16_t count);
//uint8_t erase_data_flash(FLASH_EraseInitTypeDef *flash_er_init);
uint8_t data_stat(flash_cfg_data *data);
int write_user_data(user_cfg_data *data);

#endif /* FLASH_H_ */
