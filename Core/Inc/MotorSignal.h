/**
  ******************************************************************************
  * @file           : MotorSignal.h
	* @author					: YuYang777
  * @brief          : Header for MotorSignal.c file.
  *                   This file contains the common defines of enums, structures and 
	*										functions for communication between the bluetooth module and the
	*										motor controller.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MOTORSIGNAL_H
#define __MOTORSIGNAL_H

#define SIGNAL_MODIFIED 1
#define SIGNAL_UNMODIFIED 0

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

typedef enum
{
	MOTOR_TURNING_NONE = 0,
	MOTOR_TURNING_LEFT = 1,
	MOTOR_TURNING_RIGHT = 2,
} MOTOR_TURNING_CONTROL;

typedef enum 
{
	MOTOR_MOVE_STOP = 0,
	MOTOR_MOVE_FORWARD = 1,
	MOTOR_MOVE_REVERSE = 2,
} MOTOR_MOVING_CONTROL;

typedef struct
{
	MOTOR_TURNING_CONTROL turningControl;
	MOTOR_MOVING_CONTROL movingControl;
	uint16_t speed;
	uint16_t minSpeed;
	uint16_t maxSpeed;
	uint16_t speedInterval;
	uint8_t modified;
} MotorSignal_TypeDef;

HAL_StatusTypeDef MotorSignal_Init(MotorSignal_TypeDef *motorSignal, uint16_t maxSpeed, uint16_t minSpeed, uint16_t interval);

#endif
