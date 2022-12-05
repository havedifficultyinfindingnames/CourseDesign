#ifndef M_INTERRUPTION_H
#define M_INTERRUPTION_H
#endif

#include "stm32f4xx_hal.h"

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart);