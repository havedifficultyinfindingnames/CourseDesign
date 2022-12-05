/**
  ******************************************************************************
  * @file           : ultrasonic.h
	* @author					: YuYang777
  * @brief          : Header for ultrasonic.c file.
  *                   This file contains the common defines of stucture and 
	*										functions of the ultrasonic modules.
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H


#include "stm32f4xx_hal.h"

/**
  * @brief  Ultrasonic Structure definition
  */
typedef  struct
{
	uint8_t capIndex;
	uint16_t capValue1;
	uint16_t capValue2;
	uint16_t distance;
} US_TypeDef;

/**
  * @brief  Ultrasonic handle Structure definition
  */
typedef struct
{
	US_TypeDef Instance;
	TIM_HandleTypeDef *htim;
	uint32_t Channel;
} US_HandleTypeDef;

/* The directions ultrasonic modules faced */
typedef enum 
{
	US_DIR_FORWARD = 0,
	US_DIR_BACK = 1,
	US_DIR_LEFT = 2,
	US_DIR_RIGHT = 3
} US_DIRECTION;

/**
  * @brief  Ultrasonic Group handle Structure definition
  */
typedef struct
{
	US_HandleTypeDef us[4];
	TIM_HandleTypeDef *htim;
} USGROUP_HandleTypeDef;

HAL_StatusTypeDef US_Init(US_HandleTypeDef *hus, TIM_HandleTypeDef *htim, uint32_t channel);

HAL_StatusTypeDef USGROUP_Init(USGROUP_HandleTypeDef *husg, TIM_HandleTypeDef *htim);

void US_ProcessCapture(US_HandleTypeDef *hus, TIM_HandleTypeDef *htim);

void USGROUP_ProcessCapture(USGROUP_HandleTypeDef *hus, TIM_HandleTypeDef *htim);

uint16_t US_GetDistance(USGROUP_HandleTypeDef *husg, US_DIRECTION dirction);

#endif
