#ifndef __RS485_H
#define __RS485_H
#include "stdint.h"

extern uint8_t RS485_RX_BUF[64]; 		//接收缓冲,最大64个字节
extern uint8_t RS485_RX_CNT;   			//接收到的数据长度

/* User Config */
//#define RS485_USART         huart2
#define RS485_RE_GPIO_PORT  GPIOG
#define RS485_RE_GPIO_PIN   GPIO_PIN_8

//如果想串口中断接收，设置EN_USART2_RX为1，否则设置为0
#define EN_USART2_RX 	1			//0,不接收;1,接收.

/* RS485 TX/RX Control */
#define RS485_TX_ENABLE()   HAL_Delay(1);\
                            HAL_GPIO_WritePin(RS485_RE_GPIO_PORT, RS485_RE_GPIO_PIN, GPIO_PIN_SET);\
                            HAL_Delay(1);

#define RS485_RX_ENABLE()   HAL_Delay(1);\
                            HAL_GPIO_WritePin(RS485_RE_GPIO_PORT, RS485_RE_GPIO_PIN, GPIO_PIN_RESET);\
                            HAL_Delay(1);


void RS485_Init(void);
void RS485_Send_Data(uint8_t *buf,uint8_t len);
void RS485_Receive_Data(uint8_t *buf,uint8_t *len);	
#endif
