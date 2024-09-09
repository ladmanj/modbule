/*
 * bme280.c
 *
 *  Created on: 15. 12. 2023
 *      Author: Jakub Ladman ladmanj@volny.cz
 *      SPDX-License-Identifier: GPL-2.0-or-later */

#include "bme280.h"

#include <stdlib.h>

static spi_handle *spi = NULL;

#define READ(a) 	((a) | 0x80)
#define WRITE(a)	((a) & 0x7f)

static void bme280_readCoefficients(void);
static inline uint8_t bme280_isReadingCalibration(void);

static void write8(uint8_t reg, uint8_t value);
static uint8_t read8(uint8_t reg);
static uint16_t read16(uint8_t reg);
static uint8_t *read_buf(uint8_t *buffer, uint8_t reg, uint8_t len);
static uint16_t read16_LE(uint8_t reg);
static int16_t readS16_LE(uint8_t reg);

static bme280_config _configReg;
static bme280_ctrl_meas _measReg;
static bme280_ctrl_hum _humReg;

void req_bme() {
	if (_measReg.mode == MODE_FORCED) {
		write8(BME280_REGISTER_CONTROL, (_measReg.osrs_t << 5) | (_measReg.osrs_p << 2) | _measReg.mode);
	}
}

static uint8_t buffer[3*3];


bme_data_bme280 read_bme() {
	static bme_data_bme280 bme = {0};
	uint8_t *p;
	if ((p = read_buf(buffer, BME280_REGISTER_PRESSUREDATA, 8))) {
		bme.temperature = bme280_readTemperature(p); //must be read first of the three
		bme.humidity 	= bme280_readHumidity(p);
		bme.pressure 	= bme280_readPressure(p);
	}
	return bme;
}


#define false 0
#define true ~false

static uint8_t _sensorID;
static int32_t t_fine;
static int32_t t_fine_adjust = 0;

static bme280_calib_data _bme280_calib; //!< here calibration data is stored

/*!
 *   @brief  Initialise sensor with given parameters / settings
 *   @returns true on success, false otherwise
 */
uint8_t bme280_init(spi_handle *handle) {
	if(!handle) return false;
	spi = handle;

	// reset the device using soft-reset
	// this makes sure the IIR is off, etc.
	write8(BME280_REGISTER_SOFTRESET, 0xB6);

	// wait for chip to wake up.
	__WFI();
	write8(BME280_REGISTER_CONFIG, 0b00011101);

	// check if sensor, i.e. the chip ID is correct
	_sensorID = read8(BME280_REGISTER_CHIPID);
	if (_sensorID != 0x60)
		return false;

	// if chip is still reading calibration, delay
	while (bme280_isReadingCalibration())
		__WFI();

	bme280_readCoefficients(); // read trimming parameters, see DS 4.2.2

	bme280_setSampling(MODE_NORMAL, SAMPLING_X16, SAMPLING_X16, SAMPLING_X16, FILTER_X16, STANDBY_MS_0_5, SPI_3W);

	return true;
}

void bme280_setSampling(uint8_t mode,
		uint8_t tempSampling,
		uint8_t pressSampling,
		uint8_t humSampling,
		uint8_t filter,
		uint8_t duration,
		uint8_t threewire) {
	_measReg.mode = mode;
	_measReg.osrs_t = tempSampling;
	_measReg.osrs_p = pressSampling;

	_humReg.osrs_h = humSampling;
	_configReg.filter = filter;
	_configReg.t_sb = duration;
	_configReg.spi3w_en = threewire;

	// making sure sensor is in sleep mode before setting configuration
	// as it otherwise may be ignored
	write8(BME280_REGISTER_CONTROL, MODE_SLEEP);

	// you must make sure to also set REGISTER_CONTROL after setting the
	// CONTROLHUMID register, otherwise the values won't be applied (see
	// DS 5.4.3)
	write8(BME280_REGISTER_CONTROLHUMID, _humReg.osrs_h);
	write8(BME280_REGISTER_CONFIG, (_configReg.t_sb << 5) | (_configReg.filter << 2) | _configReg.spi3w_en);
	write8(BME280_REGISTER_CONTROL, (_measReg.osrs_t << 5) | (_measReg.osrs_p << 2) | _measReg.mode);
}

static void write8(uint8_t reg, uint8_t value) {
	if(!spi) return;
	buffer[0] = WRITE(reg);
	buffer[1] = value;

	spi->ncs_gpio_port->BRR = (uint32_t)spi->ncs_gpio_pin;
	HAL_SPI_Transmit(spi->spi_port, buffer, 2, 1);
	spi->ncs_gpio_port->BSRR = (uint32_t)spi->ncs_gpio_pin;
}

static uint8_t read8(uint8_t reg) {
	read_buf(buffer,reg,1);
	return buffer[0];
}

static uint16_t read16(uint8_t reg) {
	read_buf(buffer,reg,2);
	return (uint16_t)(buffer[0]) << 8 | (uint16_t)(buffer[1]);
}

static inline uint16_t read16_LE(uint8_t reg) {
	uint16_t temp = read16(reg);
	return (temp >> 8) | (temp << 8);
}

static inline int16_t readS16(uint8_t reg) {
	return (int16_t)read16(reg);
}

static inline int16_t readS16_LE(uint8_t reg) {
	return (int16_t)read16_LE(reg);
}

static uint8_t *read_buf(uint8_t *buffer, uint8_t reg, uint8_t len) {
	if(buffer && spi) {
		buffer[0] = READ(reg);

		spi->ncs_gpio_port->BRR = (uint32_t)spi->ncs_gpio_pin;
		HAL_SPI_Transmit(spi->spi_port, buffer, 1, 1);
		HAL_SPI_Receive(spi->spi_port, buffer, len, 1);
		spi->ncs_gpio_port->BSRR = (uint32_t)spi->ncs_gpio_pin;
	}
	return buffer;
}

/*!
 *   @brief  Reads the factory-set coefficients
 */
static void bme280_readCoefficients(void) {
	_bme280_calib.dig_T1 = read16_LE(BME280_REGISTER_DIG_T1);
	_bme280_calib.dig_T2 = readS16_LE(BME280_REGISTER_DIG_T2);
	_bme280_calib.dig_T3 = readS16_LE(BME280_REGISTER_DIG_T3);

	_bme280_calib.dig_P1 = read16_LE(BME280_REGISTER_DIG_P1);
	_bme280_calib.dig_P2 = readS16_LE(BME280_REGISTER_DIG_P2);
	_bme280_calib.dig_P3 = readS16_LE(BME280_REGISTER_DIG_P3);
	_bme280_calib.dig_P4 = readS16_LE(BME280_REGISTER_DIG_P4);
	_bme280_calib.dig_P5 = readS16_LE(BME280_REGISTER_DIG_P5);
	_bme280_calib.dig_P6 = readS16_LE(BME280_REGISTER_DIG_P6);
	_bme280_calib.dig_P7 = readS16_LE(BME280_REGISTER_DIG_P7);
	_bme280_calib.dig_P8 = readS16_LE(BME280_REGISTER_DIG_P8);
	_bme280_calib.dig_P9 = readS16_LE(BME280_REGISTER_DIG_P9);

	_bme280_calib.dig_H1 = read8(BME280_REGISTER_DIG_H1);
	_bme280_calib.dig_H2 = readS16_LE(BME280_REGISTER_DIG_H2);
	_bme280_calib.dig_H3 = read8(BME280_REGISTER_DIG_H3);
	_bme280_calib.dig_H4 = ((int8_t)read8(BME280_REGISTER_DIG_H4) << 4) |
			(read8(BME280_REGISTER_DIG_H4 + 1) & 0xF);
	_bme280_calib.dig_H5 = ((int8_t)read8(BME280_REGISTER_DIG_H5 + 1) << 4) |
			(read8(BME280_REGISTER_DIG_H5) >> 4);
	_bme280_calib.dig_H6 = (int8_t)read8(BME280_REGISTER_DIG_H6);
}

static inline uint8_t bme280_isReadingCalibration(void) {
	uint8_t const rStatus = read8(BME280_REGISTER_STATUS);

	return (rStatus & (1 << 0)) != 0;
}

int32_t bme280_readTemperature(uint8_t *data) {
	int32_t var1, var2;

	int32_t adc_T = (uint32_t)data[3] << 16 | (uint32_t)data[4] << 8 | (uint32_t)(data[5] & 0xf0);
	if (adc_T == 0x800000) // value in case temp measurement was disabled
		return 0xffffffff;
	adc_T >>= 4;

	var1 = (int32_t)((adc_T >> 3) - ((int32_t)_bme280_calib.dig_T1 << 1));
	var1 = (var1 * ((int32_t)_bme280_calib.dig_T2)) >> 11;
	var2 = (int32_t)((adc_T >> 4) - ((int32_t)_bme280_calib.dig_T1));
	var2 = (((var2 * var2) >> 12) * ((int32_t)_bme280_calib.dig_T3)) >> 14;

	t_fine = var1 + var2 + t_fine_adjust;

	return (t_fine * 5 + 128) >> 8;
}

uint32_t bme280_readPressure(uint8_t *data) {
	int64_t var1, var2, var3, var4;

	//bme280_readTemperature(data); // must be done first to get t_fine

	uint32_t adc_P = (uint32_t)data[0] << 16 | (uint32_t)data[1] << 8 | (uint32_t)(data [2] & 0xf0);

	if (adc_P == 0x800000) // value in case pressure measurement was disabled
		return 0xffffffff;
	adc_P >>= 4;

	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)_bme280_calib.dig_P6;
	var2 = var2 + ((var1 * (int64_t)_bme280_calib.dig_P5) << 17);
	var2 = var2 + (((int64_t)_bme280_calib.dig_P4) << 35);
	var1 = ((var1 * var1 * (int64_t)_bme280_calib.dig_P3) >> 8) +
			((var1 * ((int64_t)_bme280_calib.dig_P2) << 12));
	var3 = ((int64_t)1) << 47;
	var1 = (var3 + var1) * ((int64_t)_bme280_calib.dig_P1) >> 33;

	if (var1 == 0) {
		return 0; // avoid exception caused by division by zero
	}

	var4 = 1048576 - adc_P;
	var4 = (((var4 << 31) - var2) * 3125) / var1;
	var1 = (((int64_t)_bme280_calib.dig_P9) * (var4 >> 13) * (var4 >> 13)) >> 25;
	var2 = (((int64_t)_bme280_calib.dig_P8) * var4) >> 19;
	var4 = ((var4 + var1 + var2) >> 8) + (((int64_t)_bme280_calib.dig_P7) << 4);

	return var4;
}

uint32_t bme280_readHumidity(uint8_t *data) {
	int32_t var1, var2, var3, var4, var5;

	//bme280_readTemperature(data); // must be done first to get t_fine

	uint32_t adc_H = (uint32_t)data[6] << 8 | (uint32_t)data[7];

	if (adc_H == 0x8000) // value in case humidity measurement was disabled
		return 0xffffffff;

	var1 = t_fine - ((int32_t)76800);
	var2 = (int32_t)(adc_H << 14);
	var3 = (int32_t)(((int32_t)_bme280_calib.dig_H4) << 20);
	var4 = ((int32_t)_bme280_calib.dig_H5) * var1;
	var5 = (((var2 - var3) - var4) + (int32_t)16384) >> 15;
	var2 = (var1 * ((int32_t)_bme280_calib.dig_H6)) >> 10;
	var3 = (var1 * ((int32_t)_bme280_calib.dig_H3)) >> 11;
	var4 = ((var2 * (var3 + (int32_t)32768)) >> 10) + (int32_t)2097152;
	var2 = ((var4 * ((int32_t)_bme280_calib.dig_H2)) + 8192) >> 14;
	var3 = var5 * var2;
	var4 = ((var3 >> 15) * (var3 >> 15)) >> 7;
	var5 = var3 - ((var4 * ((int32_t)_bme280_calib.dig_H1)) >> 4);
	var5 = (var5 < 0 ? 0 : var5);
	var5 = (var5 > 419430400 ? 419430400 : var5);
	return (uint32_t)(var5 >> 12);
}
