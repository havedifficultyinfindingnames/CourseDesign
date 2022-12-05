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
#include "bluetooth.h"
#include "ultrasonic.h"
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

extern TIM_HandleTypeDef htim3;			//ultrasonic pwm
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim8;			//ultrasonic capture

extern UART_HandleTypeDef huart2;		//485
extern UART_HandleTypeDef huart3;		//BT

USGROUP_HandleTypeDef husGroup;
MotorSignal_TypeDef signal;
uint8_t btBuffer[BT_COMMAND_LENGTH];
uint8_t btReveived;

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
  /* Infinite loop */
  for(;;)
  {
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
    osDelay(1);
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
  for(;;)
  {
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
	
}
/* USER CODE END Application */

