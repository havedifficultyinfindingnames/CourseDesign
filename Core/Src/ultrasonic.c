/**
  ******************************************************************************
  * @file           : ultrasonic.c
	* @author					: YuYang777
  * @brief          : Ultrasonic program body
  ******************************************************************************
  */
#include "ultrasonic.h"

HAL_StatusTypeDef US_Init(US_HandleTypeDef *hus, TIM_HandleTypeDef *htim, uint32_t matchChannel, uint32_t readChannel)
{
	if(NULL == hus)
	{
		return HAL_ERROR;
	}
	US_TypeDef us = {0};
	us.capIndex = 0;
	us.capValue1 = 0;
	us.capValue2 = 0;
	us.distance = 4000;
	hus->Instance = us;
	hus->htim = htim;
	hus->matchChannel = matchChannel;
	hus->readChannel = readChannel;
	return HAL_OK;
}

HAL_StatusTypeDef USGROUP_Init(USGROUP_HandleTypeDef *husg, TIM_HandleTypeDef *htim)
{
	if(NULL == husg)
	{
		return HAL_ERROR;
	}
	husg->htim = htim;
	for(int i = 0; i < 4; ++i)
	{
		US_HandleTypeDef hus = {0};
		husg->us[i] = hus;
	}
	if(
		US_Init(&(husg->us[0]), htim, HAL_TIM_ACTIVE_CHANNEL_1, TIM_CHANNEL_1) == HAL_OK &&
		US_Init(&(husg->us[1]), htim, HAL_TIM_ACTIVE_CHANNEL_2, TIM_CHANNEL_2) == HAL_OK &&
		US_Init(&(husg->us[2]), htim, HAL_TIM_ACTIVE_CHANNEL_3, TIM_CHANNEL_3) == HAL_OK &&
		US_Init(&(husg->us[3]), htim, HAL_TIM_ACTIVE_CHANNEL_4, TIM_CHANNEL_4) == HAL_OK)
	{
		return HAL_OK;
	}
	else
	{
		return HAL_ERROR;
	}
}

void US_ProcessCapture(US_HandleTypeDef *hus, TIM_HandleTypeDef *htim)
{
	if(NULL != hus && hus->htim->Instance == htim->Instance && (hus->matchChannel & htim->Channel) != 0)
	{
		HAL_TIM_IC_Stop_IT(htim,hus->readChannel);
		if(hus->Instance.capIndex == 0)
		{
			hus->Instance.capValue1 = HAL_TIM_ReadCapturedValue(htim,hus->readChannel);
			hus->Instance.capIndex = 1;
		}
		else
		{
			hus->Instance.capValue2 = HAL_TIM_ReadCapturedValue(htim,hus->readChannel);
			uint32_t diff;
			if(hus->Instance.capValue2 >= hus->Instance.capValue1)
			{
				diff = hus->Instance.capValue2 - hus->Instance.capValue1;
			}
			else
			{
				diff = hus->Instance.capValue2 + hus->htim->Instance->ARR - hus->Instance.capValue1;
			}
			hus->Instance.distance = 170 * diff / 1000;
			hus->Instance.capIndex = 0;
		}
		HAL_TIM_IC_Start_IT(htim,hus->readChannel);
	}
}

void USGROUP_ProcessCapture(USGROUP_HandleTypeDef *husg, TIM_HandleTypeDef *htim)
{
	if(NULL == husg || htim->Instance != husg->htim->Instance)
	{
		return;
	}
	for(int i = 0; i < 4; ++i)
	{
		US_ProcessCapture(&(husg->us[i]),htim);
	}
}

uint16_t US_GetDistance(USGROUP_HandleTypeDef *husg, US_DIRECTION direction)
{
	if(NULL == husg || direction > 3)
	{
		return 5000;
	}
	else
	{
		return husg->us[(uint16_t)direction].Instance.distance;
	}	
}
