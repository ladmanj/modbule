/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ACT_LED_Pin GPIO_PIN_14
#define ACT_LED_GPIO_Port GPIOC
#define IN2_Pin GPIO_PIN_4
#define IN2_GPIO_Port GPIOA
#define IN3_Pin GPIO_PIN_5
#define IN3_GPIO_Port GPIOA
#define IN4_Pin GPIO_PIN_6
#define IN4_GPIO_Port GPIOA
#define IN5_Pin GPIO_PIN_7
#define IN5_GPIO_Port GPIOA
#define IN6_Pin GPIO_PIN_0
#define IN6_GPIO_Port GPIOB
#define USER_BTN_Pin GPIO_PIN_6
#define USER_BTN_GPIO_Port GPIOC
#define RELAY_Pin GPIO_PIN_12
#define RELAY_GPIO_Port GPIOA
#define SPI1_CS1N_Pin GPIO_PIN_15
#define SPI1_CS1N_GPIO_Port GPIOA
#define SPI1_CS0N_Pin GPIO_PIN_4
#define SPI1_CS0N_GPIO_Port GPIOB
#define IN1_Pin GPIO_PIN_8
#define IN1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
