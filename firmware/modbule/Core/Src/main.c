/* USER CODE BEGIN Header */
/**
  Auto-generated glue code:

  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************

  Project specific code:

  *  Created on: 20. 4. 2020
  *      Author: Jakub Ladman ladmanj@volny.cz
  *      SPDX-License-Identifier: GPL-2.0-or-later
  */

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#ifdef DEBUG
#include <stdio.h>
#else
#define printf(...)	{}
#define puts(a)		{}
#endif

#include "modbus.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "dma_uart.h"
#include "flash.h"
#include "button.h"
#include "max31855.h"
#include "bme280.h"
#include "iostruct.h"
#include "stats.h"
#include "spi_helper.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define BME_PERIOD_MSEC	100
#define LED_ONTIME_MSEC	 10
#define LEDOFFTIME_MSEC	 90

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

WWDG_HandleTypeDef hwwdg;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

spi_handle max31855	= {.spi_port = &hspi1, .ncs_gpio_port = SPI1_CS0N_GPIO_Port, .ncs_gpio_pin = SPI1_CS0N_Pin};
spi_handle bme280	= {.spi_port = &hspi1, .ncs_gpio_port = SPI1_CS1N_GPIO_Port, .ncs_gpio_pin = SPI1_CS1N_Pin};

user_cfg_data config;

__attribute__((section(".wrflash"))) __attribute((aligned(8))) flash_cfg_data writable_flash_area = {
		.valid = FLASH_BOOL_TRUE, .free = FLASH_BOOL_FALSE,
		.data = {
				.slave_addr = 1,
				.parity = 0,
				.baudrate = 576,
				.hwc = hwc_auto,
				.logic_conf = {
						.pli = {
								{.invert = 0b0000000000000000,.enable = 0b0000000100000010},
								{.invert = 0b0000000000000000,.enable = 0b0000001000000001},
								{.invert = 0b0000000000000000,.enable = 0b0000000000000000},
								{.invert = 0b0000000000000000,.enable = 0b0000000000000000},
								{.invert = 0b0000000000000000,.enable = 0b0000000000000000},
								{.invert = 0b0000000000000000,.enable = 0b0000000000000000},
								{.invert = 0b0000000000000000,.enable = 0b0000000000000000},
								{.invert = 0b0000000000000000,.enable = 0b0000000000000000}
						},
						.plo = {.invert = 0b00000011,.output = 0b00000001},
				},
				.relay = {.index = sg_pressed_toggle, .mask = sim_input1},
				.logic = {
						{.index = sg_compare_result,   .mask = sim_comp1},
						{.index = sg_compare_result,   .mask = sim_comp2},
						{.index = sg_compare_result,   .mask = sim_comp3},
						{.index = sg_compare_result,   .mask = sim_comp4},
						{.index = sg_compare_result,   .mask = sim_comp5},
						{.index = sg_compare_result,   .mask = sim_comp6},
						{.index = sg_compare_result,   .mask = sim_comp7},
						{.index = sg_compare_result,   .mask = sim_comp8},
				},
				.comparators = {
						{.index = 0, .greater =       132, .equal = INT16_MAX, .less = INT16_MIN},
						{.index = 0, .greater = INT16_MAX, .equal = INT16_MAX, .less =       126},
						{.index = 0, .greater = INT16_MAX, .equal = INT16_MAX, .less = INT16_MIN},
						{.index = 0, .greater = INT16_MAX, .equal = INT16_MAX, .less = INT16_MIN},
						{.index = 1, .greater = INT16_MAX, .equal = INT16_MAX, .less = INT16_MIN},
						{.index = 1, .greater = INT16_MAX, .equal = INT16_MAX, .less = INT16_MIN},
						{.index = 1, .greater = INT16_MAX, .equal = INT16_MAX, .less = INT16_MIN},
						{.index = 1, .greater = INT16_MAX, .equal = INT16_MAX, .less = INT16_MIN},
				},
				.button_cfg = {
						{.debouncetck = 100, .clicktck = 500, .presstck = 1000},
						{.debouncetck = 100, .clicktck = 500, .presstck = 1000},
						{.debouncetck = 100, .clicktck = 500, .presstck = 1000},
						{.debouncetck = 100, .clicktck = 500, .presstck = 1000},
						{.debouncetck = 100, .clicktck = 500, .presstck = 1000},
						{.debouncetck = 100, .clicktck = 500, .presstck = 1000},
						{.debouncetck = 100, .clicktck = 500, .presstck = 1000},
				}
		}
};

volatile uint8_t btnbuffer = 0;
buttonstruct buttons[MAXBTNS];

holding_regs 	*holding_registers = NULL;
input_regs 		*input_registers = NULL;
coil_regs	 	*coils = NULL;
disc_inps	 	*discrete_inputs = NULL;

typedef int injected_func(uint8_t *data);
typedef struct
{
	uint32_t crc;
	uint32_t len;
	uint32_t adr;
	uint8_t	 dat;
} bl_struct;

uint8_t stop = FALSE;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CRC_Init(void);
static void MX_WWDG_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#ifdef DEBUG
int __io_putchar(int ch)
{
	uint8_t c = ch;
	return (HAL_UART_Transmit(&huart1, &c, 1, 1));
}
#endif

static inline uint8_t read_buttons()
{
 return (!HAL_GPIO_ReadPin(IN1_GPIO_Port,      IN1_Pin     ) ? 1<<0 : 0)
		|(!HAL_GPIO_ReadPin(IN2_GPIO_Port,      IN2_Pin     ) ? 1<<1 : 0)
		|(!HAL_GPIO_ReadPin(IN3_GPIO_Port,      IN3_Pin     ) ? 1<<2 : 0)
		|(!HAL_GPIO_ReadPin(IN4_GPIO_Port,      IN4_Pin     ) ? 1<<3 : 0)
		|(!HAL_GPIO_ReadPin(IN5_GPIO_Port,      IN5_Pin     ) ? 1<<4 : 0)
		|(!HAL_GPIO_ReadPin(IN6_GPIO_Port,      IN6_Pin     ) ? 1<<5 : 0)
		|(!HAL_GPIO_ReadPin(USER_BTN_GPIO_Port, USER_BTN_Pin) ? 1<<6 : 0);
}

static inline void read_temp()
{
	tc_data_max31855 tc = read_tc(&max31855);
	input_registers->temperature.s16_tc_temp = (25.0 * tc.tc_temp);
	input_registers->temperature.s16_cj_temp = (6.25 * tc.cj_temp);
}

static inline void read_humid()
{
	static uint32_t next_bme_event = 0;
	if(next_bme_event < HAL_GetTick()) {
		next_bme_event = HAL_GetTick() + BME_PERIOD_MSEC;

		bme_data_bme280 bme = read_bme();

		input_registers->humidity.u16_humidity = ((uint32_t)bme.humidity * 100) >> 10;
		input_registers->humidity.s16_temperature = bme.temperature;
		input_registers->humidity.u16_pressure_hi = (bme.pressure >> 8) / 100;
		input_registers->humidity.u16_pressure_lo = (bme.pressure >> 8) % 100;
	}
}
static inline void perform_comparisons()
{
	for(uint8_t i=0;i<MAXCOMP;i++)
	{
		uint8_t idx = holding_registers->config.comparators[i].index;
		int16_t grt = holding_registers->config.comparators[i].greater;
		int16_t equ = holding_registers->config.comparators[i].equal;
		int16_t lss = holding_registers->config.comparators[i].less;
		int16_t temp = 0;
		switch(idx)
		{
		case 0: temp = input_registers->temperature.s16_cj_temp; break;
		case 1: temp = input_registers->temperature.s16_tc_temp; break;
		case 2: temp = input_registers->humidity.u16_humidity; break;
		case 3: temp = input_registers->humidity.s16_temperature; break;
		case 4: temp = input_registers->humidity.u16_pressure_hi; break;
		case 5: temp = input_registers->ts.timestamp_hi; break;
		case 6: temp = input_registers->ts.timestamp_lo; break;
		case 7: temp = holding_registers->user_comp_val; break;
		}
		discrete_inputs->by_type.cmp[0][i]
										= ((temp > grt)||(temp == equ)||(temp < lss));
	}
}

static inline void perform_logic()
{
	uint8_t pl_inputs = 0, pl_outputs, i;
	for(i=0;i<MAXLOGIC;i++)
	{
		uint8_t x = ffs(config.logic[i].mask)-1;
		uint8_t y = config.logic[i].index;
		uint8_t val;

		if(x > sg_group_max)
			return;

		if(x >= sg_user_data0)
		{
			if(y >= co_numbit)
				return;

			x -= sg_user_data0;
			val = (*coils)[x][y];
		}
		else
		{
			uint8_t idx = x*8+y;
			if (idx > sizeof(discrete_inputs->whole))
				return;

			val = discrete_inputs->whole[idx];
		}
		pl_inputs |= val ? (1<<i) : 0;
	}

	pl_outputs = proglogic(pl_inputs, &holding_registers->config.logic_conf);

	for(i=0;i<MAXLOGIC;i++)
	{
		discrete_inputs->by_type.log[0][i] = (pl_outputs & 1<<i) ? 1 : 0;
	}
}

static inline void remote_toggles_update()
{
	for(uint8_t i = 0; i < MAXBTNS; i++)
	{
		if ((*coils)[i][co_clt_clr])
		{
			discrete_inputs->by_type.btn[i][di_clt] = 0;
		}
		else if ((*coils)[i][co_clt_set])
		{
			discrete_inputs->by_type.btn[i][di_clt] = 1;
		}

		if ((*coils)[i][co_dbl_clr])
		{
			discrete_inputs->by_type.btn[i][di_dbl] = 0;
		}
		else if ((*coils)[i][co_dbl_set])
		{
			discrete_inputs->by_type.btn[i][di_dbl] = 1;
		}

		if ((*coils)[i][co_lpr_clr])
		{
			discrete_inputs->by_type.btn[i][di_lpr] = 0;
		}
		else if ((*coils)[i][co_lpr_set])
		{
			discrete_inputs->by_type.btn[i][di_lpr] = 1;
		}

		if ((*coils)[i][co_rls_clr])
		{
			discrete_inputs->by_type.btn[i][di_rls] = 0;
		}
		else if ((*coils)[i][co_lpr_set])
		{
			discrete_inputs->by_type.btn[i][di_rls] = 1;
		}
	}
}

static inline void relay_update()
{
	uint8_t x = ffs(holding_registers->config.relay.mask)-1;
	uint8_t y = holding_registers->config.relay.index;
	uint8_t val;

	if(x <= sg_group_max)
	{
		if(x >= sg_user_data0)
		{
			if(y >= co_numbit)
				return;

			x -= sg_user_data0;
			val = (*coils)[x][y];
		}
		else
		{
			uint8_t idx = x*8+y;
			if (idx > sizeof(discrete_inputs->whole))
				return;

			val = discrete_inputs->whole[idx];
		}

		HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin,
				val ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
}

static inline void activity_led_update()
{
	static uint32_t next_led_event = 0;
	if(next_led_event < HAL_GetTick())
	{
		static uint8_t act_led_state = GPIO_PIN_RESET;
		if(act_led_state == GPIO_PIN_RESET)
		{
			act_led_state = GPIO_PIN_SET;
			next_led_event = HAL_GetTick() + LED_ONTIME_MSEC;
		}
		else
		{
			act_led_state = GPIO_PIN_RESET;
			next_led_event = HAL_GetTick() + LEDOFFTIME_MSEC;
		}
		HAL_GPIO_WritePin(ACT_LED_GPIO_Port, ACT_LED_Pin, act_led_state);
	}
}

static inline void timestamp_update()
{
	uint32_t tick = HAL_GetTick();
	input_registers->ts.timestamp_hi = (uint16_t)(tick >> 16);
	input_registers->ts.timestamp_lo = (uint16_t)(tick & 0xffff);
}

__attribute__((optimize("unroll-loops")))
void HAL_SYSTICK_Callback(void)
{
	if(stop) return;
	if(input_registers && discrete_inputs && holding_registers && coils)
	{
		btnbuffer = read_buttons();

		for(uint8_t i=0;i<MAXBTNS;i++)
		{
			fsm_tick(&buttons[i].fsm);
			discrete_inputs->by_type.btn[i][di_raw] = (btnbuffer & (1<<i)) ? 1 : 0;
		}
		timestamp_update();
		static uint8_t phase = 0;
		switch(phase++)
		{
		case 0:
			if(config.hwc != hwc_switch)
			{
				if(config.hwc & hwc_therm) read_temp();
			}
			break;
		case 1:
			if(config.hwc != hwc_switch)
			{
				if(config.hwc & hwc_humid) read_humid();
			}
			break;
		case 2:
			if(config.hwc != hwc_switch)
			{
				perform_comparisons();
			}
			break;
		case 3:
			perform_logic();
			phase = 0;
			break;
		}
		remote_toggles_update();
		relay_update();
		activity_led_update();
	}
}

static void click(uint8_t id){
	puts("click");
	if(discrete_inputs && id)
	{
		discrete_inputs->by_type.btn[id-1][di_clt] ^= 1;
	}
}

static void dblclick(uint8_t id){
	puts("double");
	if(discrete_inputs && id)
	{
		discrete_inputs->by_type.btn[id-1][di_dbl] ^= 1;
	}
}

static void lngpress(uint8_t id){
	puts("press");
	if(discrete_inputs && id)
	{
		discrete_inputs->by_type.btn[id-1][di_lpr] ^= 1;
	}
}

static void lngrls(uint8_t id){
	puts("release");
	if(discrete_inputs && id)
	{
		discrete_inputs->by_type.btn[id-1][di_rls] ^= 1;
	}
}

static inline uint8_t readbtn(uint8_t id) {
	if(!id) return 0;
	return btnbuffer & (1<<(id-1));
}

static void initbuttons()
{
	int i;
	for(i=0;i<MAXBTNS;i++)
	{
		buttons[i].id = i+1;
		buttons[i].read_btn = readbtn;
		buttons[i].now = HAL_GetTick;
		buttons[i].start_time = 0;
		buttons[i].stop_time = 0;
		buttons[i].debouncetck 	= config.button_cfg[i].debouncetck;
		buttons[i].clicktck 	= config.button_cfg[i].clicktck;
		buttons[i].presstck 	= config.button_cfg[i].presstck;
		buttons[i].is_pressed = FALSE;
		buttons[i].click = click;
		buttons[i].doubleclick = dblclick;
		buttons[i].press_start = lngpress;
		buttons[i].press_hold = NULL;
		buttons[i].press_release = lngrls;
		fsm_init(&buttons[i].fsm, btn_state0_idle, &buttons[i]);
	}
}

static void bl_CRC_mode(CRC_HandleTypeDef *hcrc) {
	// configure CRC to 32bit
	hcrc->Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
	hcrc->Init.CRCLength = CRC_POLYLENGTH_32B;
	hcrc->Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
	hcrc->Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
	if (HAL_CRC_Init(&*hcrc) != HAL_OK) {
		Error_Handler();
	}
}

static void stop_operations(void)
{
	stop = TRUE;
	__HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
	__HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_TC);
	__HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_TE);
}

static void resume_operations(void)
{
	stop = FALSE;
	__HAL_DMA_ENABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
	__HAL_DMA_ENABLE_IT(&hdma_usart2_rx, DMA_IT_TC);
	__HAL_DMA_ENABLE_IT(&hdma_usart2_rx, DMA_IT_TE);
}

static void check_and_load_injected_code(bl_struct *bl)
{
	const char *result = "ERR-";

	if ((input_registers->boot_buf_size + sizeof(bl->adr)) >= bl->len)
	{
		bl_CRC_mode(&hcrc);
		uint32_t crc = HAL_CRC_Calculate(&hcrc, &bl->adr, bl->len);
		MX_CRC_Init();

		if (bl->crc == crc)
		{
			// Am I intended to run the code in place?
			if (bl->adr == (uint32_t) &bl->dat)
			{
				injected_func *inj_fn = (injected_func*) (1 | (uint32_t) &(bl->dat));
				// calling it!
				inj_fn((uint8_t*) holding_registers);
				return;
			}
			// Am I intended to store the code to FLASH?
			if(((SCB->VTOR == 0x8000800)&&(bl->adr >= 0x8008000)&&(bl->adr < 0x800f800))
			||( (SCB->VTOR == 0x8008000)&&(bl->adr >= 0x8000800)&&(bl->adr < 0x8008000)))
				do {
					FLASH_EraseInitTypeDef flash_er_init = {
							.TypeErase = FLASH_TYPEERASE_PAGES,
							.Page = (bl->adr-0x8000000)/PGLEN,
							.NbPages = 1 };

					if((0x8000000 + flash_er_init.Page * PGLEN) != bl->adr)
						// not ok, address isn't page aligned
						break;

					uint32_t flash_op_status;
					HAL_FLASH_Unlock();

					__HAL_FLASH_CLEAR_FLAG(
							FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR);

					HAL_FLASHEx_Erase(&flash_er_init, &flash_op_status);

					while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP));

					if (__HAL_FLASH_GET_FLAG( FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR ))
					{
						//not ok
						HAL_FLASH_Lock();
						break;
					}

					__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR );

					uint16_t i;
					for(i=0;i<bl->len;i+=32*sizeof(uint64_t)){
						HAL_FLASH_Program(FLASH_TYPEPROGRAM_FAST, (uint32_t)&((uint8_t*)bl->adr)[i], (uint32_t)(&bl->dat)+i);
						while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP));
					}
					HAL_FLASH_Lock();
					if(__HAL_FLASH_GET_FLAG( FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR ))
						//not ok
						break;

					result = "-OK-";
				}while(0);
		}
	}
	memcpy(&holding_registers->rmt_command,result,4);
}

#define CMDWRITE	(uint32_t)0x392d6377	// "wc-9"
#define CMDRESET	(uint32_t)0x29287372	// "rs()"
#define CMDLDCDE	(uint32_t)0x36386c62	// "bl86"
#define CMDWFAIL	(uint32_t)0x6c696166 	// "fail"

extern FLASH_ProcessTypeDef pFlash;

static void check_command(uint8_t *query)
{
	if (!query)
		return;

	uint8_t req = query[1];
	size_t addr = query[2] << 8 | query[3];
	size_t nb = query[4] << 8 | query[5];

	if ((req == MODBUS_FC_WRITE_MULTIPLE_REGISTERS)
			&& (nb == sizeof(holding_registers->rmt_command) / sizeof(uint16_t)))
	{
		if (addr == 0)
		{
			switch (holding_registers->rmt_command)
			{
			case CMDWRITE:
				user_cfg_data *p_cfg = &(holding_registers->config);
				stop_operations();
				write_user_data(p_cfg);
				resume_operations();
				break;
			case CMDRESET:
				HAL_NVIC_SystemReset();
				break;
			case CMDLDCDE:
				check_and_load_injected_code((bl_struct*) (&((holding_with_boot_buf*) holding_registers)->crc)); //boot_buff
				break;
			case CMDWFAIL:
				while(1); // wait for watch dog
				break;
			default:
				break;
			}

		}
	}
}

static void apply_flash_config()
{
	// read config data from user flash memory segment
	uint8_t status;
	flash_cfg_data *flash_config = find_last_config(&writable_flash_area, &status);
	if (status == OCCUP) {
		config = flash_config->data;
	}
}

static void autodetect_spi_devices()
{
	// do the SPI device auto detection

	if (bme280_init(&bme280))
		if (config.hwc & hwc_auto){
			config.hwc |= hwc_humid;									// humidity meter found
		}


	uint8_t temp[8];
	if (detect_max31855((tc_data_max31855*) temp, &max31855))
		if (config.hwc & hwc_auto){
			config.hwc |= hwc_therm;									// thermo-couple chip found
		}

	if (config.hwc == hwc_auto)
		config.hwc = hwc_switch;										// no SPI device found
}

static void modbus_uart_init()
{
	huart2.Init.BaudRate = config.baudrate * 100;
	switch(config.parity)
	{
	case 0: huart2.Init.Parity = UART_PARITY_NONE; break;
	case 1: huart2.Init.Parity = UART_PARITY_ODD; break;
	case 2: huart2.Init.Parity = UART_PARITY_EVEN; break;
	}

	if (HAL_RS485Ex_Init(&huart2, UART_DE_POLARITY_HIGH, 8, 8) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_CRC_Init();
  MX_WWDG_Init();
  /* USER CODE BEGIN 2 */

  config.baudrate = 96;					// fail safe configuration
  config.parity = UART_PARITY_NONE;
  config.slave_addr = 247;
  config.hwc = hwc_auto;

  if(HAL_GPIO_ReadPin(USER_BTN_GPIO_Port, USER_BTN_Pin) == GPIO_PIN_SET)	// check if user button is pressed during startup
	  apply_flash_config(); //if button not pressed, read the configuration from flash

  autodetect_spi_devices();
  modbus_uart_init();
  initbuttons();

  // make CRC unit ready
  __HAL_RCC_CRC_CLK_ENABLE();

  modbus_t *ctx;
  ctx = modbus_new_rtu(&huart2);
  modbus_set_slave(ctx, config.slave_addr);
  modbus_set_error_recovery(ctx, MODBUS_ERROR_RECOVERY_PROTOCOL);

  if (modbus_connect(ctx) == -1) {
      printf("Connection failed: %s\n",
              modbus_strerror(errno));
      modbus_free(ctx);

      HAL_NVIC_SystemReset();
  }
	modbus_mapping_t *mb_mapping = NULL;

	mb_mapping = modbus_mapping_new(sizeof(coil_regs)/sizeof(uint8_t), sizeof(disc_inps)/sizeof(uint8_t),
			sizeof(holding_with_boot_buf)/sizeof(uint16_t), sizeof(input_regs)/sizeof(uint16_t));

	if (mb_mapping == NULL) {
		printf("Failed to allocate the mapping: %s\n",
				modbus_strerror(errno));
		modbus_free(ctx);

		HAL_NVIC_SystemReset();
	}

	holding_registers 	= (holding_regs *)	mb_mapping->tab_registers;
	input_registers   	= (input_regs *)	mb_mapping->tab_input_registers;
	coils 				= (coil_regs *)		mb_mapping->tab_bits;
	discrete_inputs 	= (disc_inps *)		mb_mapping->tab_input_bits;

	{
		uint32_t adr = (uint32_t)(((holding_with_boot_buf *)holding_registers)->boot_buff);

		input_registers->boot_buf_addr_hi  = adr >> 16;
		input_registers->boot_buf_addr_lo  = adr & 0xffff;

		input_registers->boot_buf_size  = sizeof(((holding_with_boot_buf *)holding_registers)->boot_buff);
		input_registers->boot_vector 	= 0xffff & SCB->VTOR;
	}


	holding_registers->config = config;

	uint8_t *query = malloc(MODBUS_MAX_ADU_LENGTH);
	if (query == NULL) {
		printf("Failed to allocate the query buffer: %s\n",
				modbus_strerror(errno));
		modbus_free(ctx);

		HAL_NVIC_SystemReset();
	}
	input_registers->sysinf.version = DATA_VERSION;

	init_dma_circ_buf();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	static uint16_t errcount = 0;
	int rc;
	rc = modbus_receive(ctx, query);
	if ((rc == -1) && (errno != ETIMEDOUT)) {
		/* Connection closed by the client or error */
		errcount++;
	}
	if (rc > 0) {

		modbus_reply(ctx, query, rc, mb_mapping);
		/* refresh local registers */
//#ifdef DEBUG
//		memcpy(input_registers->sysinf.statistics, stats, sizeof(stats));
//#endif
//		input_registers->sysinf.statistics[4] = errcount;

		check_command(query);
	}
}

  printf("Quit the loop: %s\n", modbus_strerror(errno));

//  if(query) free(query);
//  modbus_mapping_free(mb_mapping);
//
//  /* For RTU, skipped by TCP (no TCP connect) */
//  modbus_close(ctx);
//  modbus_free(ctx);

  HAL_NVIC_SystemReset();

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 9;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.GeneratingPolynomial = 32773;
  hcrc.Init.CRCLength = CRC_POLYLENGTH_16B;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_BYTE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 57600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_RS485Ex_Init(&huart2, UART_DE_POLARITY_HIGH, 8, 8) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief WWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_WWDG_Init(void)
{

  /* USER CODE BEGIN WWDG_Init 0 */

  /* USER CODE END WWDG_Init 0 */

  /* USER CODE BEGIN WWDG_Init 1 */

  /* USER CODE END WWDG_Init 1 */
  hwwdg.Instance = WWDG;
  hwwdg.Init.Prescaler = WWDG_PRESCALER_64;
  hwwdg.Init.Window = 97;
  hwwdg.Init.Counter = 97;
  hwwdg.Init.EWIMode = WWDG_EWI_DISABLE;
  if (HAL_WWDG_Init(&hwwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN WWDG_Init 2 */

  /* USER CODE END WWDG_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ACT_LED_GPIO_Port, ACT_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI1_CS1N_GPIO_Port, SPI1_CS1N_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI1_CS0N_GPIO_Port, SPI1_CS0N_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : ACT_LED_Pin */
  GPIO_InitStruct.Pin = ACT_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ACT_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : IN2_Pin IN3_Pin IN4_Pin IN5_Pin */
  GPIO_InitStruct.Pin = IN2_Pin|IN3_Pin|IN4_Pin|IN5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : IN6_Pin IN1_Pin */
  GPIO_InitStruct.Pin = IN6_Pin|IN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USER_BTN_Pin */
  GPIO_InitStruct.Pin = USER_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(USER_BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RELAY_Pin */
  GPIO_InitStruct.Pin = RELAY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RELAY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI1_CS1N_Pin */
  GPIO_InitStruct.Pin = SPI1_CS1N_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI1_CS1N_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI1_CS0N_Pin */
  GPIO_InitStruct.Pin = SPI1_CS0N_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI1_CS0N_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
