/*
 * flash.c
 *
 *  Created on: 21. 8. 2017
 *      Author: Jakub Ladman ladmanj@volny.cz
 *      SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "modbule_mcu.h"
#include <stdint.h>

#ifdef DEBUG
#include <stdio.h>
#define DBGPUTS(a) {puts((a));}
#else
#define DBGPUTS(a) {}
#endif

#include <string.h>

#include "flash.h"

#ifndef FALSE
#define FALSE	0
#endif
#ifndef TRUE
#define TRUE	1
#endif

#ifdef WWDG
extern WWDG_HandleTypeDef hwwdg;
#endif

uint8_t __attribute__((optimize("Os"))) write_data_flash(uint64_t *ram_data, uint64_t *flash_ptr, uint16_t count)
{
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR );

	uint16_t i;
	uint32_t address;
	uint64_t data __attribute((aligned(8)));

	for(i=0;i<count;i++){
		address = (uint32_t)(&flash_ptr[i]);
		data = ram_data[i];

#ifdef WWDG
		HAL_WWDG_Refresh(&hwwdg);
#endif
		// two writes needed to actually write the flash (why?)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data);
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data);
	}
	HAL_FLASH_Lock();
	return __HAL_FLASH_GET_FLAG( FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR ) ?
			FALSE : TRUE;

}
uint8_t erase_data_flash(FLASH_EraseInitTypeDef *flash_er_init) {
	uint32_t flash_op_status;
	HAL_FLASH_Unlock();

	__HAL_FLASH_CLEAR_FLAG(
			FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR);

	HAL_FLASHEx_Erase(flash_er_init, &flash_op_status);

	while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP));

	HAL_FLASH_Lock();
	return __HAL_FLASH_GET_FLAG( FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR ) ?
			FALSE : TRUE;
}



flash_cfg_data *find_last_config(flash_cfg_data *ptr, uint8_t *status){
	uint16_t i;
	for(i = 0; i < (PGLEN/sizeof(flash_cfg_data)); i++){
		if(!ptr[i].valid){
			continue;
		}
		if(ptr[i].free){
			if(status){
				*status = FREE;
			}
			return (flash_cfg_data*)&ptr[i];
		}else{
			if(status){
				*status = OCCUP;
			}
			return (flash_cfg_data*)&ptr[i];
		}
	}
	if(status){
		*status = FULL;
	}
	return ptr;
}



uint8_t data_stat(flash_cfg_data *data) {

	if(!data)
		return ERROR;
	if((uint32_t)(data - &writable_flash_area) >= PGLEN)
		return FULL;

	return (data->valid ? 1:0) | (data->free ? 2:0);
}


int write_user_data(user_cfg_data *data) {

	FLASH_EraseInitTypeDef flash_er_init = {
			.TypeErase = FLASH_TYPEERASE_PAGES,
			.Page = 31, // last 2K page of regular code flash
			.NbPages = 1 };

	uint8_t status = 0;
	flash_cfg_data *dst = NULL;
	flash_cfg_data src = {
			.valid = ~0,
			.free = 0,
			.data = *data,
	};

	while ((dst = find_last_config(&writable_flash_area, &status))) {
		if (status == ERROR) {
			return FLASH_CFG_ERROR;
		}
		else if (status == FULL) {
			DBGPUTS("erasing");
			status = erase_data_flash(&flash_er_init);
			if(!status){
				return FLASH_CFG_ERROR; //if something went wrong
			}
		}
		else{
			break;
		}
	}
	do {
		switch (data_stat(dst)) {
		case FREE:
			DBGPUTS("free to write");
			status = write_data_flash((uint64_t*)&src,(uint64_t*)dst,sizeof(src)/sizeof(uint64_t));
			if (!status) {
				return FLASH_CFG_ERROR;
			}
			return FLASH_CFG_STORED;
			break;
		case OCCUP:
			DBGPUTS("occuppied");
			if(!memcmp(dst,&src,sizeof(src))){
				DBGPUTS("unchanged");
				return FLASH_CFG_UNCHANGED;
			}
			if(data_stat(dst+1) == FREE){
				DBGPUTS("next is free to write");
				status = write_data_flash((uint64_t*)&src, (uint64_t*)(dst+1),sizeof(src)/sizeof(uint64_t));
				if (!status) {
					return FLASH_CFG_ERROR;
				}
// 				DBGPUTS("discarding old");	// printing in middle of the flash writes ruined the result (why?)
				uint64_t invalidate = 0;
				status = write_data_flash(&invalidate, (uint64_t*)dst,1);
				if (!status) {
					return FLASH_CFG_ERROR;
				}
				DBGPUTS("old discarded");
			}
			// two occupied records
			// invalid state - fall thru
		default:
			DBGPUTS("error");
			return FLASH_CFG_ERROR;
			break;
		}
	} while(1);
}
