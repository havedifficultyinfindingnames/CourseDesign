/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

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
#define BT_STA_Pin GPIO_PIN_6
#define BT_STA_GPIO_Port GPIOF
#define LED_RED_Pin GPIO_PIN_9
#define LED_RED_GPIO_Port GPIOF
#define LED_GREEN_Pin GPIO_PIN_10
#define LED_GREEN_GPIO_Port GPIOF
#define BT_WKUP_Pin GPIO_PIN_0
#define BT_WKUP_GPIO_Port GPIOC
#define MOTOR_TX_Pin GPIO_PIN_2
#define MOTOR_TX_GPIO_Port GPIOA
#define MOTOR_RX_Pin GPIO_PIN_3
#define MOTOR_RX_GPIO_Port GPIOA
#define US_PWM_Pin GPIO_PIN_6
#define US_PWM_GPIO_Port GPIOA
#define BT_TX_Pin GPIO_PIN_10
#define BT_TX_GPIO_Port GPIOB
#define BT_RX_Pin GPIO_PIN_11
#define BT_RX_GPIO_Port GPIOB
#define MOTOR_PWM_Pin GPIO_PIN_12
#define MOTOR_PWM_GPIO_Port GPIOD
#define MOTOR_PWMD13_Pin GPIO_PIN_13
#define MOTOR_PWMD13_GPIO_Port GPIOD
#define US_CAP1_Pin GPIO_PIN_6
#define US_CAP1_GPIO_Port GPIOC
#define US_CAP2_Pin GPIO_PIN_7
#define US_CAP2_GPIO_Port GPIOC
#define US_CAP3_Pin GPIO_PIN_8
#define US_CAP3_GPIO_Port GPIOC
#define US_CAP4_Pin GPIO_PIN_9
#define US_CAP4_GPIO_Port GPIOC
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
