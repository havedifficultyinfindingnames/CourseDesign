#include "rs485.h"
#include "usart.h"
#include "stdint.h"
//UART_HandleTypeDef USART2_RS485Handler;  //USART2句柄(用于RS485)

#if EN_USART2_RX   		//如果使能了接收   	  
//接收缓存区 	
uint8_t RS485_RX_BUF[64];  	//接收缓冲,最大64个字节.
//接收到的数据长度
uint8_t RS485_RX_CNT=0;  

void USART2_IRQHandler(void)
{
    uint8_t res;	  
    if((__HAL_UART_GET_FLAG(&huart2,UART_FLAG_RXNE)!=RESET))  //接收中断
	{	 	
       HAL_UART_Receive(&huart2,&res,1,1000);
		  if(RS485_RX_CNT<64)
		  {
		   	RS485_RX_BUF[RS485_RX_CNT]=res;		//记录接收到的值
			  RS485_RX_CNT++;						//接收数据增加1 
		  } 
	} 
}    
#endif

//初始化IO 串口2
//bound:波特率
void RS485_Init(void)
{
  #if EN_USART2_RX
	__HAL_UART_ENABLE_IT(&huart2,UART_IT_RXNE);//开启接收中断
	HAL_NVIC_EnableIRQ(USART2_IRQn);				        //使能USART2中断
	HAL_NVIC_SetPriority(USART2_IRQn,3,3);			        //抢占优先级3，子优先级3
  #endif	
	RS485_RX_ENABLE();											//默认为接收模式		
}

//RS485发送len个字节.
//buf:发送区首地址
//len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
void RS485_Send_Data(uint8_t *buf,uint8_t len)
{
  RS485_TX_ENABLE()			//设置为发送模式	
	HAL_UART_Transmit(&huart2,buf,len,1000);//串口2发送数据
	RS485_RX_CNT=0;	  
	RS485_RX_ENABLE();			//设置为接收模式	
}
//RS485查询接收到的数据
//buf:接收缓存首地址
//len:读到的数据长度
void RS485_Receive_Data(uint8_t *buf,uint8_t *len)
{
	uint8_t rxlen=RS485_RX_CNT;
	uint8_t i=0;
	*len=0;				//默认为0
	HAL_Delay(10);		//等待10ms,连续超过10ms没有接收到一个数据,则认为接收结束
	if(rxlen==RS485_RX_CNT&&rxlen)//接收到了数据,且接收完成了
	{
		for(i=0;i<rxlen;i++)
		{
			buf[i]=RS485_RX_BUF[i];	
		}		
		*len=RS485_RX_CNT;	//记录本次数据长度
		RS485_RX_CNT=0;		//清零
	}
} 
