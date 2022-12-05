#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include "stm32f4xx_hal.h"
#include "MotorSignal.h"

#define BT_COMMAND_LENGTH 2

void BT_ProcessMessage(uint8_t *message, MotorSignal_TypeDef *signal);

#endif
