/*
 * bme280.h
 *
 *  Created on: 15. 12. 2023
 *      Author: Jakub Ladman ladmanj@volny.cz
 *      SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef BME280_H_
#define BME280_H_

#include <stdint.h>
#include "spi_helper.h"

/*=========================================================================
    REGISTERS
    -----------------------------------------------------------------------*/
    enum
    {
      BME280_REGISTER_DIG_T1              = 0x88,
      BME280_REGISTER_DIG_T2              = 0x8A,
      BME280_REGISTER_DIG_T3              = 0x8C,

      BME280_REGISTER_DIG_P1              = 0x8E,
      BME280_REGISTER_DIG_P2              = 0x90,
      BME280_REGISTER_DIG_P3              = 0x92,
      BME280_REGISTER_DIG_P4              = 0x94,
      BME280_REGISTER_DIG_P5              = 0x96,
      BME280_REGISTER_DIG_P6              = 0x98,
      BME280_REGISTER_DIG_P7              = 0x9A,
      BME280_REGISTER_DIG_P8              = 0x9C,
      BME280_REGISTER_DIG_P9              = 0x9E,

      BME280_REGISTER_DIG_H1              = 0xA1,
      BME280_REGISTER_DIG_H2              = 0xE1,
      BME280_REGISTER_DIG_H3              = 0xE3,
      BME280_REGISTER_DIG_H4              = 0xE4,
      BME280_REGISTER_DIG_H5              = 0xE5,
      BME280_REGISTER_DIG_H6              = 0xE7,

      BME280_REGISTER_CHIPID             = 0xD0,
      BME280_REGISTER_VERSION            = 0xD1,
      BME280_REGISTER_SOFTRESET          = 0xE0,

      BME280_REGISTER_CAL26              = 0xE1,  // R calibration stored in 0xE1-0xF0

      BME280_REGISTER_CONTROLHUMID       = 0xF2,
	  BME280_REGISTER_STATUS 			 = 0XF3,
      BME280_REGISTER_CONTROL            = 0xF4,
      BME280_REGISTER_CONFIG             = 0xF5,
      BME280_REGISTER_PRESSUREDATA       = 0xF7,
      BME280_REGISTER_TEMPDATA           = 0xFA,
      BME280_REGISTER_HUMIDDATA          = 0xFD,
    };

/*=========================================================================*/


typedef struct {
	uint32_t humidity; 		//
	int32_t temperature;	//
	uint32_t pressure;		//

} bme_data_bme280;

void req_bme();
bme_data_bme280 read_bme();

typedef struct {
  uint16_t dig_T1; // temperature compensation value
  int16_t dig_T2;  // temperature compensation value
  int16_t dig_T3;  // temperature compensation value

  uint16_t dig_P1; // pressure compensation value
  int16_t dig_P2;  // pressure compensation value
  int16_t dig_P3;  // pressure compensation value
  int16_t dig_P4;  // pressure compensation value
  int16_t dig_P5;  // pressure compensation value
  int16_t dig_P6;  // pressure compensation value
  int16_t dig_P7;  // pressure compensation value
  int16_t dig_P8;  // pressure compensation value
  int16_t dig_P9;  // pressure compensation value

  uint8_t dig_H1; // humidity compensation value
  int16_t dig_H2; // humidity compensation value
  uint8_t dig_H3; // humidity compensation value
  int16_t dig_H4; // humidity compensation value
  int16_t dig_H5; // humidity compensation value
  int8_t dig_H6;  // humidity compensation value
} bme280_calib_data;

enum sensor_sampling {
  SAMPLING_NONE = 0b000,
  SAMPLING_X1 = 0b001,
  SAMPLING_X2 = 0b010,
  SAMPLING_X4 = 0b011,
  SAMPLING_X8 = 0b100,
  SAMPLING_X16 = 0b101
};

enum sensor_mode {
  MODE_SLEEP = 0b00,
  MODE_FORCED = 0b01,
  MODE_NORMAL = 0b11
};

enum sensor_filter {
  FILTER_OFF = 0b000,
  FILTER_X2 = 0b001,
  FILTER_X4 = 0b010,
  FILTER_X8 = 0b011,
  FILTER_X16 = 0b100
};

enum standby_duration {
  STANDBY_MS_0_5 = 0b000,
  STANDBY_MS_10 = 0b110,
  STANDBY_MS_20 = 0b111,
  STANDBY_MS_62_5 = 0b001,
  STANDBY_MS_125 = 0b010,
  STANDBY_MS_250 = 0b011,
  STANDBY_MS_500 = 0b100,
  STANDBY_MS_1000 = 0b101
};

enum spimode {
	DEFAULT = 0b0,
	SPI_4W = 0b0,
	SPI_3W = 0b1
};

uint8_t bme280_init(spi_handle *spi);

void bme280_setSampling(uint8_t mode,
                        uint8_t tempSampling,
						uint8_t pressSampling,
						uint8_t humSampling,
						uint8_t filter,
						uint8_t duration,
						uint8_t threewire);

int32_t bme280_readTemperature(uint8_t *data);
uint32_t bme280_readPressure(uint8_t *data);
uint32_t bme280_readHumidity(uint8_t *data);



typedef struct {
  // inactive duration (standby time) in normal mode
  // 000 = 0.5 ms
  // 001 = 62.5 ms
  // 010 = 125 ms
  // 011 = 250 ms
  // 100 = 500 ms
  // 101 = 1000 ms
  // 110 = 10 ms
  // 111 = 20 ms
  unsigned int t_sb : 3; // inactive duration (standby time) in normal mode

  // filter settings
  // 000 = filter off
  // 001 = 2x filter
  // 010 = 4x filter
  // 011 = 8x filter
  // 100 and above = 16x filter
  unsigned int filter : 3; // filter settings

  // unused - don't set
  unsigned int none : 1;     // unused - don't set
  unsigned int spi3w_en : 1; // three wire mode
} bme280_config;

/**************************************************************************/
/*!
    @brief  ctrl_meas register
*/
/**************************************************************************/
typedef struct {
  // temperature oversampling
  // 000 = skipped
  // 001 = x1
  // 010 = x2
  // 011 = x4
  // 100 = x8
  // 101 and above = x16
  unsigned int osrs_t : 3; // temperature oversampling

  // pressure oversampling
  // 000 = skipped
  // 001 = x1
  // 010 = x2
  // 011 = x4
  // 100 = x8
  // 101 and above = x16
  unsigned int osrs_p : 3; // pressure oversampling

  // device mode
  // 00       = sleep
  // 01 or 10 = forced
  // 11       = normal
  unsigned int mode : 2; // device mode
} bme280_ctrl_meas;


/**************************************************************************/
/*!
    @brief  ctrl_hum register
*/
/**************************************************************************/
typedef struct {
  /// unused - don't set
  unsigned int none : 5;

  // pressure oversampling
  // 000 = skipped
  // 001 = x1
  // 010 = x2
  // 011 = x4
  // 100 = x8
  // 101 and above = x16
  unsigned int osrs_h : 3; // pressure oversampling
} bme280_ctrl_hum;


#endif /* BME280_H_ */
