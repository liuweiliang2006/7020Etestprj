/**
  ******************************************************************************
  * File Name          : RTC.c
  * Description        : This file provides code for the configuration
  *                      of the RTC instances.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */
extern CONFIG_Meter_t CONFIG_Meter;				 //仪表的一些硬件信息
/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

    /**Initialize RTC Only 
    */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  /* USER CODE BEGIN RTC_Init 2 */
  RTC_TimeTypeDef time;  
  RTC_DateTypeDef date; 
	
  HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	RTC_AlarmTypeDef sAlarm;
	
	if(CONFIG_Meter.NotHaveDog == true)
	{
		sAlarm.AlarmTime.Hours = 0;
		//sAlarm.AlarmTime.Minutes = (time.Minutes + 1) % 60;//一分钟一启动
		sAlarm.AlarmTime.Minutes = (time.Minutes / 5 * 5 + 5) % 60;//五分钟一启动
		//sAlarm.AlarmTime.Minutes = 0;//一小时一启动
		sAlarm.AlarmTime.Seconds = 0;
		sAlarm.AlarmTime.SubSeconds = 0;
		sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
		sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS;
		sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
		sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
		sAlarm.AlarmDateWeekDay = 1;
		sAlarm.Alarm = RTC_ALARM_A;
		if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
	}
	else
	{
		sAlarm.AlarmTime.Hours = 0;
		sAlarm.AlarmTime.Minutes = 0;
		//printf("time.Seconds %d\r\n",time.Seconds);
		sAlarm.AlarmTime.Seconds = (time.Seconds / 20 + (time.Seconds % 20 < 15 ? 1 : 2)) % 3 * 20 ;//(time.Seconds / 10 + (time.Seconds % 10 < 5 ? 1 : 2)) % 6 * 10 ;
		//printf("AlarmTime.Seconds %d\r\n",sAlarm.AlarmTime.Seconds);
		sAlarm.AlarmTime.SubSeconds = 0;
		sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
		sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS
																|RTC_ALARMMASK_MINUTES;
		sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
		sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
		sAlarm.AlarmDateWeekDay = 1;
		sAlarm.Alarm = RTC_ALARM_A;
		if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
	}
  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
//获得RTC的时间
void GetRTC(long *hhmmss,long *yymmdd)  
{  
 RTC_TimeTypeDef time;  
 RTC_DateTypeDef date; 
	
 HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
 HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	
 *hhmmss = time.Hours*10000 + time.Minutes*100 + time.Seconds;  
 *yymmdd = (date.Year + 2000)*10000 + date.Month*100 + date.Date;  
	
}
//根据网络报文的时间更新设备的时间
void SetRTC(long hhmmss,long yymmdd)  
{  
 RTC_TimeTypeDef RTC_TimeStruct;  
 RTC_DateTypeDef RTC_DateStruct; 
	
 RTC_DateStruct.Year = yymmdd/10000;  
 RTC_DateStruct.Month = yymmdd/100%100; 
 RTC_DateStruct.Date = yymmdd%100; 
 RTC_DateStruct.WeekDay = 0;	
 HAL_RTC_SetDate(&hrtc,&RTC_DateStruct,RTC_FORMAT_BIN); 
	
 RTC_TimeStruct.Hours = hhmmss/10000;  //与服务器的时差
 RTC_TimeStruct.Minutes = hhmmss/100%100;  
 RTC_TimeStruct.Seconds = hhmmss%100;  
 RTC_TimeStruct.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
 RTC_TimeStruct.StoreOperation = RTC_STOREOPERATION_RESET;
 HAL_RTC_SetTime(&hrtc,&RTC_TimeStruct,RTC_FORMAT_BIN);
	
 HAL_RTC_GetTime(&hrtc, &RTC_TimeStruct, RTC_FORMAT_BIN);
 HAL_RTC_GetDate(&hrtc, &RTC_DateStruct, RTC_FORMAT_BIN);
 //printf(" the time is: %d :%d :%d   %d:%d:%d \r\n", RTC_DateStruct.Year, RTC_DateStruct.Month,RTC_DateStruct.Date, RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);
}

void Write_Time(tim_t t)
{
  long date,time;
	
	time=t.tm_hour*10000 + t.tm_minute*100 + t.tm_seconds;
	date=t.tm_day*10000 + t.tm_month*100 + (t.tm_year-2000);
	SetRTC(time,date);
}

tim_t Read_Time(void)
{
	tim_t t_tm;
	long date,time;//??UTC??
	GetRTC(&time,&date);
	//printf("date:%ld\r\n",date);
	t_tm.tm_year = date/10000;    //?	
	t_tm.tm_month = date/100%100;	  //?
	t_tm.tm_day = date%100;    //?
	t_tm.tm_hour = time/10000;      //?
	t_tm.tm_minute = time/100%100;     //?
	t_tm.tm_seconds = time%100;//?
	return t_tm;
}

//获得一天中的持续时间,不会大于24小时
int Get_SubSeconds(tim_t stim)
{
	tim_t t_tm;
	long date,time;//??UTC??
	GetRTC(&time,&date);
	t_tm.tm_year = date/10000;    //?	
	t_tm.tm_month = date/100%100;	  //?
	t_tm.tm_day = date%100;    //?
	t_tm.tm_hour = time/10000;      //?
	t_tm.tm_minute = time/100%100;     //?
	t_tm.tm_seconds = time%100;//?
	
	//如果开始小于结束,说明是在一天内
	if(stim.tm_hour <= t_tm.tm_hour)
	{
		return (t_tm.tm_hour * 3600 + t_tm.tm_minute * 60 + t_tm.tm_seconds)- (stim.tm_hour * 3600 + stim.tm_minute * 60 + stim.tm_seconds);
	}
  else
	{
	  return (t_tm.tm_hour * 3600 + t_tm.tm_minute * 60 + t_tm.tm_seconds + 86400)- (stim.tm_hour * 3600 + stim.tm_minute * 60 + stim.tm_seconds);
	}
}

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
