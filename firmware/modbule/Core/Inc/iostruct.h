/*
 * iostruct.h
 *
 *  Created on: 19. 7. 2020
 *      Author: Jakub Ladman ladmanj@volny.cz
 *      SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IOSTRUCT_H_
#define IOSTRUCT_H_

#define DATA_VERSION	20244	// YYYYN - year, and number 0 to 9, max. 10 versions a year, sorry ;-)

#include "proglogic.h"

#define MAXBTNS 7
#define MAXCOMP 8

typedef struct
{
	uint16_t version;
	uint16_t statistics[8];
} system_info;

typedef struct
{
	uint8_t index;
	uint8_t mask;
} signal_assignment;

typedef struct
{
	uint16_t index;
	int16_t greater;
	int16_t equal;
	int16_t less;
} comparator_values;

typedef enum {
	hwc_switch		= 0x01,
	hwc_therm 		= 0x02,
	hwc_humid 		= 0x04,
	hwc_auto 		= 0x10,
	hwc_switch_ad 	= hwc_switch | hwc_auto,
	hwc_therm_ad 	= hwc_therm  | hwc_auto,
	hwc_humid_ad 	= hwc_humid  | hwc_auto,
	hwc_hum_th_ad 	= hwc_humid  | hwc_therm | hwc_auto
} hw_config;

typedef struct {
	uint16_t debouncetck;
	uint16_t clicktck;
	uint16_t presstck;
} buttoncfg;

typedef struct
{
	uint32_t	pri_app_crc;	// preferred sector to boot has this CRC value
	uint32_t	sec_app_crc;	// backup sector 			-//-
	uint8_t		slave_addr;
	uint8_t		parity;
	uint16_t	baudrate; // 96 for 9600, 576 for 57600
	uint16_t	hwc;

	pl_config   logic_conf;

	signal_assignment	relay;
	signal_assignment	logic[MAXLOGIC];
	comparator_values	comparators[MAXCOMP];

	buttoncfg	button_cfg[MAXBTNS];

} user_cfg_data;

typedef struct
{
	int16_t			s16_tc_temp;
	int16_t			s16_cj_temp;
} thermocouple;

typedef struct
{
	uint16_t		u16_humidity;
	int16_t			s16_temperature;
	uint16_t		u16_pressure_hi;
	uint16_t		u16_pressure_lo;
} humidsens;

typedef struct
{
	uint16_t		timestamp_hi;
	uint16_t		timestamp_lo;
} timestamp;

typedef struct
{
	thermocouple	temperature;				// 01 .. 02
	humidsens		humidity;					// 03 .. 06
	timestamp 		ts;							// 07 .. 08

	system_info		sysinf;						// 09 .. 17
	uint16_t		boot_buf_addr_hi;			// 18
	uint16_t		boot_buf_addr_lo;			// 19
	uint16_t		boot_buf_size;				// 20
	uint16_t		boot_vector;				// 21
} input_regs;

typedef struct
{
	uint32_t 		rmt_command;				// 01 .. 02
	int16_t			user_comp_val;				// 03
	int16_t			padding;					// 04

	user_cfg_data	config;						// 05 .. 90
} holding_regs;

typedef struct
{
	holding_regs	regs;						// must be the first element of the struct
	uint32_t		crc;						// 90 -> uint32_t[46]
	uint32_t		len;
	uint32_t		adr;
	uint8_t			boot_buff[2048]  __attribute__((aligned(4)));
} holding_with_boot_buf;

typedef enum {
	di_raw,
	di_clt,
	di_dbl,
	di_lpr,
	di_rls,
	di_btn_numsrc
} di_btn_sources;

typedef enum {
	di_cmp_1,
	di_cmp_2,
	di_cmp_3,
	di_cmp_4,
	di_cmp_5,
	di_cmp_6,
	di_cmp_7,
	di_cmp_8,
	di_cmp_numsrc
} di_cmp_sources;

typedef enum {
	di_log_1,
	di_log_2,
	di_log_3,
	di_log_4,
	di_log_5,
	di_log_6,
	di_log_7,
	di_log_8,
	di_log_numsrc
} di_log_sources;


typedef enum {
	in_1,
	in_2,
	in_3,
	in_4,
	in_5,
	in_6,
	in_7,
	di_numbtn,
	di_numlog = 1,
	di_numcmp = 1
} di_channel;


typedef struct
{
	uint8_t btn[di_numbtn][di_btn_numsrc];
	uint8_t cmp[di_numcmp][di_cmp_numsrc];
	uint8_t log[di_numlog][di_log_numsrc];
} disc_inps_by_type;

typedef union
{
	disc_inps_by_type	by_type;
	uint8_t				whole[sizeof(disc_inps_by_type)];
} disc_inps;

typedef enum {
	co_clt_set,
	co_clt_clr,
	co_dbl_set,
	co_dbl_clr,
	co_lpr_set,
	co_lpr_clr,
	co_rls_set,
	co_rls_clr,
	co_numtgt
} co_btn_target;

typedef enum {
	co_bit_01,
	co_bit_02,
	co_bit_03,
	co_bit_04,
	co_bit_05,
	co_bit_06,
	co_bit_07,
	co_bit_08,
	co_numbit
} co_usr_target;

typedef enum {
	co_ch_1,
	co_ch_2,
	co_ch_3,
	co_ch_4,
	co_ch_5,
	co_ch_6,
	co_ch_7,
	co_user_1,
	co_user_2,
	co_numchn
} co_channel;

typedef uint8_t coil_regs[co_numchn][co_numtgt];

typedef enum {
	sg_raw_inputs,
	sg_clicked_toggle,
	sg_dbl_clicked_toggle,
	sg_pressed_toggle,
	sg_released_toggle,
	sg_compare_result,
	sg_logic_output,
	sg_user_data0,
	sg_user_data1,
	sg_group_max
} sig_groups;

typedef enum {
	sim_input1 = 1<<0,
	sim_input2 = 1<<1,
	sim_input3 = 1<<2,
	sim_input4 = 1<<3,
	sim_input5 = 1<<4,
	sim_input6 = 1<<5,
	sim_button = 1<<6,
	sim_input_max = 7
} sig_input_mask;

typedef enum {
	sim_comp1 = 1<<0,
	sim_comp2 = 1<<1,
	sim_comp3 = 1<<2,
	sim_comp4 = 1<<3,
	sim_comp5 = 1<<4,
	sim_comp6 = 1<<5,
	sim_comp7 = 1<<6,
	sim_comp8 = 1<<7,
	sim_comp_max = 8
} sig_comp_mask;

typedef enum {
	sim_log1 = 1<<0,
	sim_log2 = 1<<1,
	sim_log3 = 1<<2,
	sim_log4 = 1<<3,
	sim_log5 = 1<<4,
	sim_log6 = 1<<5,
	sim_log7 = 1<<6,
	sim_log8 = 1<<7,
	sim_log_max = 8
} sig_log_mask;

#endif /* IOSTRUCT_H_ */
