/* USER CODE BEGIN Header */
/**
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
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "flash.h"
#include "iostruct.h"
#include "modbule_a.h"
#include "modbule_b.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#define BSTAT_WDG	(1<<0)
#define BSTAT_PRI	(1<<1)
#define BSTAT_SEC	(1<<2)

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

__attribute__ ((section (".no_init"))) uint32_t lastboot;

__attribute__((section(".wrflash")))  flash_cfg_data config= {
		.valid = FLASH_BOOL_TRUE, .free = FLASH_BOOL_FALSE, .data = {
				.pri_app_crc = MODBULE_A,
				.sec_app_crc = MODBULE_B,
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
extern uint8_t app_a[];
extern uint8_t app_b[];
#define APP_SIZE	(30*1024)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_CRC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

flash_cfg_data *find_config(flash_cfg_data *ptr){
	uint16_t i;
	for(i = 0; i < (PGLEN/sizeof(flash_cfg_data)); i++){
		if(!ptr[i].valid){
			continue;
		}
		if(!ptr[i].free){
			return (flash_cfg_data*)&ptr[i];
		}
	}
	return (void*)0;
}

__attribute__((naked)) void startApplication(uint32_t stackPointer, uint32_t startupAddress){ // Starts an application (sets the main stack pointer to stackPointer and jumps to startupAddress)
    __ASM("msr msp, r0"); // Set stack pointer to application stack pointer
    __ASM("bx r1"); // Branch to application startup code
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  //uint32_t bark = LL_RCC_IsActiveFlag_WWDGRST();
	uint32_t bark = RCC->CSR & RCC_CSR_WWDGRSTF;
	RCC->CSR |= RCC_CSR_RMVF;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* USER CODE BEGIN Init */
#if 0	// SystemClock_Config doesn't fit, but not necessary
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
#endif
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_CRC_Init();
  /* USER CODE BEGIN 2 */
  union{
	  uint8_t *ptr;
	  flash_cfg_data *cfg;
  }u;

  for(u.ptr = app_a; u.ptr < (app_a + APP_SIZE); u.ptr++)
  {
	  LL_CRC_FeedData8(CRC, *u.ptr);
  }
  uint32_t CRC_result_A = LL_CRC_ReadData32(CRC);

  MX_CRC_Init();

  for(u.ptr = app_b; u.ptr < (app_b + APP_SIZE); u.ptr++)
  {
	  LL_CRC_FeedData8(CRC, *u.ptr);
  }
  uint32_t CRC_result_B = LL_CRC_ReadData32(CRC);

  u.cfg = find_config(&config);

  if(!u.cfg) Error_Handler();

  if(0 != (lastboot & ~(BSTAT_WDG|BSTAT_PRI|BSTAT_SEC))) lastboot = 0;
  if((BSTAT_WDG|BSTAT_PRI|BSTAT_SEC) ==
		  ((BSTAT_WDG|BSTAT_PRI|BSTAT_SEC) & lastboot)) lastboot = 0; // reinitialize if in invalid state

#define PRIVALID ((!(BSTAT_WDG & lastboot)) || (BSTAT_SEC & lastboot))
#define SECVALID ((!(BSTAT_WDG & lastboot)) || (BSTAT_PRI & lastboot))

  __disable_irq();

	if (PRIVALID && (u.cfg->data.pri_app_crc == CRC_result_A)) {

		lastboot = BSTAT_PRI;
		SCB->VTOR = (__IOM uint32_t)app_a;

	} else if (PRIVALID && (u.cfg->data.pri_app_crc == CRC_result_B)) {

		lastboot = BSTAT_PRI;
		SCB->VTOR = (__IOM uint32_t)app_b;

	} else if (SECVALID && (u.cfg->data.sec_app_crc == CRC_result_A)) {

		lastboot = BSTAT_SEC;
		SCB->VTOR = (__IOM uint32_t)app_a;

	} else if (SECVALID && (u.cfg->data.sec_app_crc == CRC_result_B)) {

		lastboot = BSTAT_SEC;
		SCB->VTOR = (__IOM uint32_t)app_b;

	} else Error_Handler();

  __enable_irq();

  if(bark) lastboot |= BSTAT_WDG;		// store whether last reset was due to watchdog
  startApplication(*(uint32_t*)(SCB->VTOR), *(uint32_t*)(SCB->VTOR + 4));

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1)
  {
  }

  /* HSI configuration and activation */
  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() != 1)
  {
  }

  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_1, 9, LL_RCC_PLLR_DIV_3);
  LL_RCC_PLL_Enable();
  LL_RCC_PLL_EnableDomain_SYS();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  }

  /* Set AHB prescaler*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  /* Sysclk activation on the main PLL */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  }

  /* Set APB1 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

  LL_Init1msTick(48000000);

  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(48000000);
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

  /* Peripheral clock enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_CRC);

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
//  LL_CRC_SetInputDataReverseMode(CRC, LL_CRC_INDATA_REVERSE_BYTE);
  LL_CRC_SetInputDataReverseMode(CRC, LL_CRC_INDATA_REVERSE_NONE);
//  LL_CRC_SetOutputDataReverseMode(CRC, LL_CRC_OUTDATA_REVERSE_BIT);
  LL_CRC_SetOutputDataReverseMode(CRC, LL_CRC_OUTDATA_REVERSE_NONE);
  LL_CRC_SetInitialData(CRC, LL_CRC_DEFAULT_CRC_INITVALUE);
//  LL_CRC_SetPolynomialCoef(CRC, 32773);
  LL_CRC_SetPolynomialCoef(CRC, 0x04C11DB7);
//  LL_CRC_SetPolynomialSize(CRC, LL_CRC_POLYLENGTH_16B);
  LL_CRC_SetPolynomialSize(CRC, LL_CRC_POLYLENGTH_32B);
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

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
