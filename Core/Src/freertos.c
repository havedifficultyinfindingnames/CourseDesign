/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "bluetooth.h"
#include "ultrasonic.h"
#include "rs485.h"

#include "stm32f4xx_hal_uart.h"
#include <stdint.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

const uint16_t unsafeDistance = 500;	//distance car should stop gradually
const uint16_t urgentDistance = 50;		//diatance car should brake 

extern TIM_HandleTypeDef htim3;			//ultrasonic pwm

extern TIM_HandleTypeDef htim4;     //转向

extern TIM_HandleTypeDef htim8;			//ultrasonic capture

extern UART_HandleTypeDef huart2;		//485
extern UART_HandleTypeDef huart3;		//BT

USGROUP_HandleTypeDef husGroup;
MotorSignal_TypeDef signal;
uint8_t btBuffer[BT_COMMAND_LENGTH];
extern uint8_t btReceived;
uint8_t btConnected;

uint8_t emerg_stop[] = {0x01,0x06,0x00,0x20,0x00,0x01,0x49,0xC0};  //紧急停止
uint8_t nor_stop[] = {0x01,0x06,0x00,0x20,0x00,0x00,0x88,0x00};    //正常停止
uint8_t speed_forward[18][13] = {{0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0x1E,0xB0,0x73},   //前进，速度30~200
																 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0x28,0x30,0x65},
																 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0x32,0xB1,0xAE},
																 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0x3C,0x30,0x6A},
																 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0x46,0xB1,0x89},
																 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0x50,0x30,0x47},
																 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0x5A,0xB0,0x40},
																 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0x64,0x31,0x90},
																 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0x6E,0xB1,0x97},
																 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0x78,0x30,0x59},
																 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0x82,0xB0,0x1A},
																 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0x8C,0x31,0xDE},
																 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0x96,0xB0,0x15},
																 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0xA0,0x30,0x03},
																 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0xAA,0xB0,0x04},
																 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0xB4,0x30,0x0C},
																 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0xBE,0xB0,0x0B},
																 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0x00,0x00,0x00,0xC8,0x31,0xED}};
uint8_t speed_reveres[18][13] = { {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0xE2,0xF1,0xE6},     //后退，速度30~200
															 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0xD8,0x71,0xF5},
															 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0xCE,0xF0,0x3B},
															 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0xC4,0x70,0x3C},
															 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0xBA,0xF0,0x1C},
															 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0xB0,0x70,0x1B},
															 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0xA6,0xF1,0xD5},
															 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0x9C,0x71,0xC6},
															 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0x92,0xF0,0x02},
															 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0x88,0x71,0xC9},
															 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0x7E,0xF1,0x8F},
															 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0x74,0x71,0x88},
															 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0x6A,0xF1,0x80},
															 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0x60,0x71,0x87},
															 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0x56,0xF1,0x91},
															 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0x4C,0x70,0x5A},
															 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0x42,0xF1,0x9E},
															 {0x01,0x10,0x00,0x21,0x00,0x02,0x04,0xFF,0xFF,0xFF,0x38,0x70,0x7D}};

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for TASK_BT */
osThreadId_t TASK_BTHandle;
const osThreadAttr_t TASK_BT_attributes = {
  .name = "TASK_BT",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TASK_CONTROL */
osThreadId_t TASK_CONTROLHandle;
const osThreadAttr_t TASK_CONTROL_attributes = {
  .name = "TASK_CONTROL",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TASK_US */
osThreadId_t TASK_USHandle;
const osThreadAttr_t TASK_US_attributes = {
  .name = "TASK_US",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for TASK_LCD */
osThreadId_t TASK_LCDHandle;
const osThreadAttr_t TASK_LCD_attributes = {
  .name = "TASK_LCD",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void BTTaskRoutine(void *argument);
void ControlTaskRoutine(void *argument);
void USTaskRoutine(void *argument);
void LCDTaskRountine(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of TASK_BT */
  TASK_BTHandle = osThreadNew(BTTaskRoutine, NULL, &TASK_BT_attributes);

  /* creation of TASK_CONTROL */
  TASK_CONTROLHandle = osThreadNew(ControlTaskRoutine, NULL, &TASK_CONTROL_attributes);

  /* creation of TASK_US */
  TASK_USHandle = osThreadNew(USTaskRoutine, NULL, &TASK_US_attributes);

  /* creation of TASK_LCD */
  TASK_LCDHandle = osThreadNew(LCDTaskRountine, NULL, &TASK_LCD_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
		//HAL_GPIO_TogglePin(BT_WKUP_GPIO_Port,BT_WKUP_Pin);
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_BTTaskRoutine */
/**
* @brief Function implementing the TASK_BT thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_BTTaskRoutine */
void BTTaskRoutine(void *argument)
{
  /* USER CODE BEGIN BTTaskRoutine */
	MotorSignal_Init(&signal,50,0,10);
	/* BT recevie start */
	HAL_UART_Receive_IT(&huart3, (uint8_t *)btBuffer, BT_COMMAND_LENGTH);
  /* Infinite loop */
  for(;;)
  {
		btConnected = HAL_GPIO_ReadPin(BT_STA_GPIO_Port, BT_STA_Pin);
		if(btReceived == BT_RECEIVED)
		{
			BT_ProcessMessage(btBuffer, &signal);
			btReceived = BT_WAIT;
			HAL_UART_Receive_IT(&huart3, (uint8_t *)btBuffer, BT_COMMAND_LENGTH);
		}
    osDelay(1);
  }
  /* USER CODE END BTTaskRoutine */
}

/* USER CODE BEGIN Header_ControlTaskRoutine */
/**
* @brief Function implementing the TASK_CONTROL thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ControlTaskRoutine */
void ControlTaskRoutine(void *argument)
{
  /* USER CODE BEGIN ControlTaskRoutine */
  /* Infinite loop */
  for(;;)
  {
		if(signal.modified == 1)
		{
				if(BT_CONNECTED == btConnected)
				{
					HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
				}
				else
				{
					HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET );
				}
				if(SIGNAL_MODIFIED == signal.modified)
				{
					if(signal.speed > 25)
					{
						HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
					}
					else
					{
						HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
					}
					signal.modified = SIGNAL_UNMODIFIED;
				}
				osDelay(1);
				
				switch(signal.turningControl)
				{
					case 0:
					{
						HAL_TIM_PWM_Stop(&htim4,TIM_CHANNEL_1);
						HAL_TIM_PWM_Stop(&htim4,TIM_CHANNEL_2);
						break;
					}
					case 1:
					{
						HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1);    //左转
						break;
					}
					case 2:
					{
						HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_2);    //右转
						break;
					}
					default:
					{
						HAL_TIM_PWM_Stop(&htim4,TIM_CHANNEL_1);
						HAL_TIM_PWM_Stop(&htim4,TIM_CHANNEL_2);
					}
				}
				
				switch(signal.movingControl)
				{
				case 0:
					{ 
						RS485_Send_Data(nor_stop,8);       //正常停止
						break;
					}
					case 1:
					{
						RS485_Send_Data(speed_forward[signal.speed],13);  //前进
						break;
					}
					case 2:
					{
						RS485_Send_Data(speed_reveres[signal.speed],13);  //后退
						break;
					}
					default:
						RS485_Send_Data(emerg_stop,8);        //紧急停止
				}
			}
		else
			RS485_Send_Data(emerg_stop,8);     //蓝牙断开，小车停止
  }
  /* USER CODE END ControlTaskRoutine */
}

/* USER CODE BEGIN Header_USTaskRoutine */
/**
* @brief Function implementing the TASK_US thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_USTaskRoutine */
void USTaskRoutine(void *argument)
{
  /* USER CODE BEGIN USTaskRoutine */
	USGROUP_Init(&husGroup, &htim8);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END USTaskRoutine */
}

/* USER CODE BEGIN Header_LCDTaskRountine */
/**
* @brief Function implementing the TASK_LCD thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_LCDTaskRountine */
void LCDTaskRountine(void *argument)
{
  /* USER CODE BEGIN LCDTaskRountine */
  /* Infinite loop */
  static uint16_t cur_bg_color = 0;
  for(;;)
  {
    LCD_display_char(20, 20, '0', FONT_VERY_LARGE, false);
    LCD_clear(++cur_bg_color);
    osDelay(1);
  }
  /* USER CODE END LCDTaskRountine */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	USGROUP_ProcessCapture(&husGroup,htim);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
	if(huart->Instance == USART3)	//bluetooth
	{
		btReceived = BT_RECEIVED;
	}
}
/* USER CODE END Application */

