#include "bluetooth.h"

void BT_ProcessMessage(uint8_t *message, MotorSignal_TypeDef *signal)
{
	switch(message[0])
	{
		case 'P':
			signal->movingControl = MOTOR_MOVE_STOP;
			break;
		case 'M':
			if(message[1] == 'F')
			{
				signal->movingControl = MOTOR_MOVE_FORWARD;
			}
			else if(message[1] == 'R')
			{
				signal->movingControl = MOTOR_MOVE_REVERSE;
			}
			break;
		case 'T':
			if(message[1] == 'L')
			{
				signal->turningControl = MOTOR_TURNING_LEFT;
			}
			else if(message[1] == 'R')
			{
				signal->turningControl = MOTOR_TURNING_RIGHT;
			}
			else if(message[1] == 'N')
			{
				signal->turningControl = MOTOR_TURNING_NONE;
			}
			break;
		case 'S':
			if(message[1] == 'U')
			{
				signal->speed += signal->speedInterval;
				if(signal->speed > signal->maxSpeed)
				{
					signal->speed = signal->maxSpeed;
				}
			}
			else if(message[1] == 'D')
			{
				signal->speed -= signal->speedInterval;
				if(signal->speed < signal->minSpeed)
				{
					signal->speed = signal->minSpeed;
				}
			}
			break;
		default:
			break;
	}
}
