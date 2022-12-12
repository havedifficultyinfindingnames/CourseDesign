#include "bluetooth.h"

void BT_ProcessMessage(uint8_t *message, MotorSignal_TypeDef *signal)
{
	switch(message[0])
	{
		case 'P':
			signal->movingControl = MOTOR_MOVE_STOP;
			break;
		case 'F':
			signal->movingControl = MOTOR_MOVE_FORWARD;
			break;
		case 'B':
			signal->movingControl = MOTOR_MOVE_REVERSE;
			break;
		case 'L':
			signal->turningControl = MOTOR_TURNING_LEFT;
			break;
		case 'R':
			signal->turningControl = MOTOR_TURNING_RIGHT;
			break;
		case 'N':
			signal->turningControl = MOTOR_TURNING_NONE;
			break;
		case 'U':
			signal->speed += signal->speedInterval;
			if(signal->speed > signal->maxSpeed)
			{
				signal->speed = signal->maxSpeed;
			}
			break;
		case 'D':
			signal->speed -= signal->speedInterval;
			if(signal->speed < signal->minSpeed)
			{
				signal->speed = signal->minSpeed;
			}
			break;
		default:
			break;
	}
	signal->modified = SIGNAL_MODIFIED;
}
