/**
  ******************************************************************************
  * @file           : bluetooth.h
	* @author					: YuYang777
  * @brief          : Header for bluetooth.c file.
  *                   This file contains the common defines of functions used
	*										by the bluetooth module.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "MotorSignal.h"

#define BT_RECEIVED 1
#define BT_WAIT 0
#define BT_CONNECTED GPIO_PIN_RESET
#define BT_DISCONNECTED GPIO_PIN_SET

/* Length of the buffer used by the bluetooth module -------------------------*/
#define BT_COMMAND_LENGTH 1

void BT_ProcessMessage(uint8_t *message, MotorSignal_TypeDef *signal);

#endif
