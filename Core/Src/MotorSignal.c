#include "MotorSignal.h"

HAL_StatusTypeDef MotorSignal_Init(MotorSignal_TypeDef *motorSignal, uint16_t maxSpeed, uint16_t minSpeed, uint16_t interval)
{
	if(NULL == motorSignal)
	{
		return HAL_ERROR;
	}
	else
	{
		motorSignal->turningControl = MOTOR_TURNING_NONE;
		motorSignal->movingControl = MOTOR_MOVE_STOP;
		motorSignal->speed = 0;
		motorSignal->minSpeed = minSpeed;
		motorSignal->maxSpeed = maxSpeed;
		motorSignal->speedInterval = interval;
		return HAL_OK;
	}
}
