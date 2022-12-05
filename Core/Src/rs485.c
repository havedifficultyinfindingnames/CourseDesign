#include "rs485.h"
#include "usart.h"
#include "stdint.h"
//UART_HandleTypeDef USART2_RS485Handler;  //USART2���(����RS485)

#if EN_USART2_RX   		//���ʹ���˽���   	  
//���ջ����� 	
uint8_t RS485_RX_BUF[64];  	//���ջ���,���64���ֽ�.
//���յ������ݳ���
uint8_t RS485_RX_CNT=0;  

void USART2_IRQHandler(void)
{
    uint8_t res;	  
    if((__HAL_UART_GET_FLAG(&huart2,UART_FLAG_RXNE)!=RESET))  //�����ж�
	{	 	
       HAL_UART_Receive(&huart2,&res,1,1000);
		  if(RS485_RX_CNT<64)
		  {
		   	RS485_RX_BUF[RS485_RX_CNT]=res;		//��¼���յ���ֵ
			  RS485_RX_CNT++;						//������������1 
		  } 
	} 
}    
#endif

//��ʼ��IO ����2
//bound:������
void RS485_Init(void)
{
  #if EN_USART2_RX
	__HAL_UART_ENABLE_IT(&huart2,UART_IT_RXNE);//���������ж�
	HAL_NVIC_EnableIRQ(USART2_IRQn);				        //ʹ��USART2�ж�
	HAL_NVIC_SetPriority(USART2_IRQn,3,3);			        //��ռ���ȼ�3�������ȼ�3
  #endif	
	RS485_RX_ENABLE();											//Ĭ��Ϊ����ģʽ		
}

//RS485����len���ֽ�.
//buf:�������׵�ַ
//len:���͵��ֽ���(Ϊ�˺ͱ�����Ľ���ƥ��,���ｨ�鲻Ҫ����64���ֽ�)
void RS485_Send_Data(uint8_t *buf,uint8_t len)
{
  RS485_TX_ENABLE()			//����Ϊ����ģʽ	
	HAL_UART_Transmit(&huart2,buf,len,1000);//����2��������
	RS485_RX_CNT=0;	  
	RS485_RX_ENABLE();			//����Ϊ����ģʽ	
}
//RS485��ѯ���յ�������
//buf:���ջ����׵�ַ
//len:���������ݳ���
void RS485_Receive_Data(uint8_t *buf,uint8_t *len)
{
	uint8_t rxlen=RS485_RX_CNT;
	uint8_t i=0;
	*len=0;				//Ĭ��Ϊ0
	HAL_Delay(10);		//�ȴ�10ms,��������10msû�н��յ�һ������,����Ϊ���ս���
	if(rxlen==RS485_RX_CNT&&rxlen)//���յ�������,�ҽ��������
	{
		for(i=0;i<rxlen;i++)
		{
			buf[i]=RS485_RX_BUF[i];	
		}		
		*len=RS485_RX_CNT;	//��¼�������ݳ���
		RS485_RX_CNT=0;		//����
	}
} 
