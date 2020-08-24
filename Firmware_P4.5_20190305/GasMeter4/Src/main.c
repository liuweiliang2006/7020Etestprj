
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "stm32f0xx_hal.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "iwdg.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "bsp.h"
#include "Sim80X.h"
#include "RechargePacket.h"
#include "StringUtils.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
/* ����ͳ������ʱ�� */
volatile uint32_t CPU_RunTime = 0UL;
extern volatile bool IsIntoLowPower;

//float flow_zero = 0;//����������ֵ,�ڷ��Źرյ�״̬�ɼ�
uint8_t collectionSum = 0;//�ɼ�����

//extern uint32_t gGasCnt;
extern uint8_t		Usart3RxTemp;
extern REAL_DATA_PARAM_t REAL_DATA_PARAM;   //�ź�ǿ�ȵ�
extern CONFIG_Meter_t CONFIG_Meter;				  //�Ǳ��һЩӲ����Ϣ
extern uint32_t zeroFlowTime;
extern uint32_t vibrationsNumber;
extern uint32_t OpenLockTime;
extern REAL_DATA_CALIBRATION_t REAL_DATA_CALIBRATION;
extern uint8_t Current_card_type;  //��ǰ��������

extern volatile uint32_t StartRunTime;//ϵͳ��ʼ���е�ʱ��,�ɶ�ʱ���ۼ� �������͹�������
extern volatile bool IsStartCooking5; //�������������Ҵ���5����

extern osThreadId defaultTaskHandle;
extern uint32_t beginMotoTime;

extern REAL_DATA_Credit_t REAL_DATA_Credit; 

extern uint8_t sStep;       //��ǰ��Ļ��ֵ

uint32_t ADC_Data[50];
uint32_t ADC_DataAvg;

uint8_t AvgNumber = 4;

bool IsTestPower = false;//�Ƿ����˵�Դ����
bool IsTestFlow = true;//�Ƿ�������������
bool IsTestEnd = false;//���������Ƿ����

float ADTestVFlow = 0; //У׼��ʱ������ѹ,ʵ���õ�ʱ����ʵʱ����
float TmpRealTimeFlow = 0; //У׼��ʱ������ѹ,ʵ���õ�ʱ����ʵʱ����

float volt1 = 0;   //��1����  �ɼ�ֵ	�͵�λ
float	y1 = 0;      //��1����  ����ֵ

float vert2 = 0;    //��2����  �ɼ�ֵ
float	y2 =0;       //��2����  ����ֵ

//bool IsReadVoltage = false;

extern bool gkeyValue;   //���ݵ���Ļ����ֵ
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
static void MX_NVIC_Init(void);
void my_NVIC_DeInit(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
__IO uint32_t VectorTable [48] __attribute__((at(0x20000000)));

void AppVectorTblRemap(void)
{
 memcpy((uint32_t*)0x20000000, (uint32_t*)0x8008000, 48*4); 
 
  __HAL_RCC_SYSCFG_CLK_ENABLE(); 
        
  __HAL_SYSCFG_REMAPMEMORY_SRAM();
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	HAL_RCC_DeInit();
	my_NVIC_DeInit();
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
	AppVectorTblRemap();
	__enable_irq();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_SPI2_Init();
  //MX_RTC_Init();
  MX_TIM6_Init();
//  MX_TIM7_Init();
  //MX_IWDG_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
  /* RTC_IRQn interrupt configuration */
	
	//�ƶ�����ȡ���ڴ�ֵ�ú���
	//MX_RTC_Init();//ÿ���Զ�����,��Ҫע�͵�,��Ϊ�����ȡ���ú���ܵ���,�й���û�е����ò�һ��
	//MX_IWDG_Init();//ÿ���Զ����ɺ��Ժ���Ҫע�͵���һ��,�����������Ź��Ĵ���,�������������27��,��Ҫ�رտ��Ź�,�����Լ�����
	//HAL_NVIC_SetPriority(RTC_IRQn, 3, 0); �����ɹ����������Ҫ������ʱ����,��Ҫ����������,�����Զ����ɵĲ����,
	//HAL_NVIC_EnableIRQ(RTC_IRQn);�����ں�����ϵͳֵ�����������RTC��ʱ���ж�
	
//	HAL_TIM_Base_Start_IT(&htim6);
//	HAL_TIM_Base_Start_IT(&htim7);
	
	//TestMB85RS16A();
	//TestW25Q64();
	//�����￪���洢���ĵ�Դ,��ȡ����,Ӧ����ϵͳ�������������ִ�ж�ȡ�Ȳ���,�����п���ͨ��GPRS���ͱ�����Ϣ
	
	printf("System2 Init ok |%s| |%s|\r\n", __DATE__, __TIME__);
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();
	AppObjCreate(); //������ֵ�ź������ڱ�ʶUART����������ݡ�
	AppObjCreateQueue();//ȫ����Ϣ��������ָʾ��ǰ���еĴ���ָ�� 
  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
  //todo ������е�����,����д��־
  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Configure LSE Drive Capability 
    */
  HAL_PWR_EnableBkUpAccess();

  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI14|RCC_OSCILLATORTYPE_LSI
                              |RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/8000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 3, 0);
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* EXTI0_1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
  /* EXTI2_3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI2_3_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
  /* EXTI4_15_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
  /* ADC1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(ADC1_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(ADC1_IRQn);
  /* USART2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART2_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USART3_6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART3_6_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(USART3_6_IRQn);
  /* TIM6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM6_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(TIM6_IRQn);
  /* TIM7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM7_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(TIM7_IRQn);
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

/* USER CODE BEGIN 4 */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{

  RTC_AlarmTypeDef sAlarm;
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hrtc);
  
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_RTC_AlarmAEventCallback could be implemented in the user file
   */

		if(CONFIG_Meter.NotHaveDog == false || Current_card_type == 1)//�й�������������
		{
				//printf("sTime %d:%d:%d\r\n",sTime.Hours,sTime.Minutes,sTime.Seconds);
				//SystemClock_Config();
				HAL_RTC_GetAlarm(hrtc, &sAlarm,RTC_ALARM_A,RTC_FORMAT_BIN);
				//printf("sAlarm  %02d:%02d:%02d\r\n",sAlarm.AlarmTime.Hours,sAlarm.AlarmTime.Minutes,sAlarm.AlarmTime.Seconds);
				//HAL_RTC_DeactivateAlarm(&hrtc,RTC_ALARM_A);
				
				sAlarm.AlarmTime.Hours = 0;
				sAlarm.AlarmTime.Minutes = 0;
				sAlarm.AlarmTime.Seconds = (sAlarm.AlarmTime.Seconds + 20) % 60;//20��
				//printf("sAlarm  %02d:%02d:%02d\r\n",sAlarm.AlarmTime.Hours,sAlarm.AlarmTime.Minutes,sAlarm.AlarmTime.Seconds);
				sAlarm.AlarmTime.SubSeconds = 0;
				sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
				sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
				sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS
																		|RTC_ALARMMASK_MINUTES;
				sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
				sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
				sAlarm.AlarmDateWeekDay = 1;
				sAlarm.Alarm = RTC_ALARM_A;
				if (HAL_RTC_SetAlarm_IT(hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
				{
					_Error_Handler(__FILE__, __LINE__);
				}
		}
		else
		{
		    //printf("sTime %d:%d:%d\r\n",sTime.Hours,sTime.Minutes,sTime.Seconds);
				//SystemClock_Config();
				HAL_RTC_GetAlarm(hrtc, &sAlarm,RTC_ALARM_A,RTC_FORMAT_BIN);
				//printf("sAlarm  %02d:%02d:%02d\r\n",sAlarm.AlarmTime.Hours,sAlarm.AlarmTime.Minutes,sAlarm.AlarmTime.Seconds);
				//HAL_RTC_DeactivateAlarm(&hrtc,RTC_ALARM_A);
				
				sAlarm.AlarmTime.Hours = 0;
				//sAlarm.AlarmTime.Minutes = (sAlarm.AlarmTime.Minutes + 1) % 60;//һ����
			  sAlarm.AlarmTime.Minutes = (sAlarm.AlarmTime.Minutes / 5 * 5  + 5) % 60;//�����
			  //sAlarm.AlarmTime.Minutes = 0;//һСʱ
				sAlarm.AlarmTime.Seconds = 0;
				//printf("sAlarm  %02d:%02d:%02d\r\n",sAlarm.AlarmTime.Hours,sAlarm.AlarmTime.Minutes,sAlarm.AlarmTime.Seconds);
				sAlarm.AlarmTime.SubSeconds = 0;
				sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
				sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
				sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS;
				sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
				sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
				sAlarm.AlarmDateWeekDay = 1;
				sAlarm.Alarm = RTC_ALARM_A;
				if (HAL_RTC_SetAlarm_IT(hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
				{
					_Error_Handler(__FILE__, __LINE__);
				}
		}

}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
void my_NVIC_DeInit(void)
{
  /* EXTI0_1_IRQn interrupt configuration */
  //HAL_NVIC_SetPriority(EXTI0_1_IRQn, 3, 0);
  HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);
  /* EXTI2_3_IRQn interrupt configuration */
  //HAL_NVIC_SetPriority(EXTI2_3_IRQn, 3, 0);
  HAL_NVIC_DisableIRQ(EXTI2_3_IRQn);
  /* EXTI4_15_IRQn interrupt configuration */
  //HAL_NVIC_SetPriority(EXTI4_15_IRQn, 3, 0);
  HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
  /* DMA1_Channel2_3_IRQn interrupt configuration */
  //HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 3, 0);
  HAL_NVIC_DisableIRQ(DMA1_Channel2_3_IRQn);
  /* ADC1_IRQn interrupt configuration */
  //HAL_NVIC_SetPriority(ADC1_IRQn, 3, 0);
  HAL_NVIC_DisableIRQ(ADC1_IRQn);
  /* USART2_IRQn interrupt configuration */
  //HAL_NVIC_SetPriority(USART2_IRQn, 3, 0);
  HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USART3_6_IRQn interrupt configuration */
  //HAL_NVIC_SetPriority(USART3_6_IRQn, 3, 0);
  HAL_NVIC_DisableIRQ(USART3_6_IRQn);
}
/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(GPIO_Pin);

  /* NOTE: This function should not be modified, when the callback is needed,
            the HAL_GPIO_EXTI_Callback could be implemented in the user file
   */ 
	//if(StartRunTime > 1)
	{
		//printf("GPIO_Pin %d\r\n", GPIO_Pin);
		//printf("--------------\r\n");
		if((GPIO_Pin == GPIO_PIN_0   //��Ļ����
			|| GPIO_Pin == GPIO_PIN_1  //���  PA1
		  || GPIO_Pin == GPIO_PIN_2  //����  ΢��
		  || GPIO_Pin ==GPIO_PIN_12  //ҡ��
		   ) && sStep == 0)
		{
			//printf("GPIO_Pin1 %d\r\n", GPIO_Pin);
			gkeyValue = true;
		}

		}
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  huart pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	 uint32_t i = 0;
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_UART_RxCpltCallback could be implemented in the user file
   */
  if(huart == &huart2)
	{
		if((Sim80x.Status.DataTransferMode==0)&&(Sim80x.UsartRxTemp!=0))
    {
			/*add by lwl*/
			__HAL_TIM_SET_COUNTER(&htim6,0);//clear timer6 counter value
			if( Sim80x.UsartRxIndex == 0) //recive fist byte
			{
				__HAL_TIM_CLEAR_FLAG(&htim6,TIM_FLAG_UPDATE); //clear timer inturrupt flag
				HAL_TIM_Base_Start_IT(&htim6);                //open timer6 inturrupt function
			}
			
			Sim80x.UsartRxLastTime = HAL_GetTick();
			Sim80x.UsartRxBuffer[Sim80x.UsartRxIndex] = Sim80x.UsartRxTemp;
			if(Sim80x.UsartRxIndex < (_SIM80X_BUFFER_SIZE-1))
					Sim80x.UsartRxIndex++;
    }
    else if(Sim80x.Status.DataTransferMode==1)
    {
			
			Sim80x.UsartRxLastTime = HAL_GetTick();
			Sim80x.UsartRxBuffer[Sim80x.UsartRxIndex] = Sim80x.UsartRxTemp;
			if(Sim80x.UsartRxIndex < (_SIM80X_BUFFER_SIZE-1))
					Sim80x.UsartRxIndex++;
    }

    HAL_UART_Receive_IT(&_SIM80X_USART,&Sim80x.UsartRxTemp,1);
//		Sim80x_RxCallBack();
	}
	if(huart == &huart3)
	{
		fill_rec_buf();

	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    HAL_ADC_Stop_DMA(hadc);
	  float  tmp = 0;

	
	  //����Щ����û�е�ʱ��Ųɼ���ѹ,���ⷧ�Ŷ���Ӱ���ѹ�ɼ�
	  if(glockStatus == motor_null && gmotorStatus == motor_null && gassembleStatus == motor_null && IsIntoLowPower == false)//&& GSM_ON_FLAG == 0 
	  {
		  if(IsTestFlow == true)
		  {
			
				if(Current_card_type == 3)
				{
				    //if(NEEDLE_input == 0)//down ��
						{
							//if((REAL_DATA_CALIBRATION.slope - 1) < 0.0000009 && REAL_DATA_CALIBRATION.zero < 0.0000009)
							{
								ADTestVFlow = (float)ADC_Data[0] / 4096 * 3.3;
		//						printf("ADTestVFlow f1 :%f\r\n",ADTestVFlow);
							}
//							else
//							{
//								ADTestVFlow = (REAL_DATA_CALIBRATION.slope * ADC_Data[0] + REAL_DATA_CALIBRATION.zero);
		//						printf("ADTestVFlow f2 :%f\r\n",REAL_DATA_CALIBRATION.slope);
		//						printf("ADTestVFlow f2 :%f\r\n",REAL_DATA_CALIBRATION.zero);
		//						printf("ADTestVFlow f2 :%f\r\n",ADTestVFlow);
//							}
							if(strcmp(REAL_DATA_PARAM.ElectricValveStatus, "1")==0)
							{
								 vert2 = ADTestVFlow;
								 y2 = 3.144;
								 //printf("ADC_Data %d\r\n", ADC_Data[0]);
								 //printf("vert2:%.2f\r\n",vert2);
								 //REAL_DATA_Credit.CumulationGasL += (getYvalue(ADTestVFlow) / 60.0 * 3);
								 zeroFlowTime = 0;
							}
							else
							{
								 volt1 = ADTestVFlow;
								 //printf("ADC_Data %d\r\n", ADC_Data[0]);
								 //printf("volt1:%.2f\r\n",volt1);
								 y1 = 0;
								 //printf("x1:%.2f\r\n",x1);
							}
						}
				}
				
				if(Current_card_type == 1)
				{
						
					  ADTestVFlow = (float)ADC_Data[0] / 4096 * 3.3;
					//����뷧�ǹ�,�����ǹ�
					if( strcmp(REAL_DATA_PARAM.ElectricValveStatus, "0")==0)//NEEDLE_input == GPIO_PIN_SET ||  �����뷧��λ����,�����ж�
					{
						
						TmpRealTimeFlow = 0;
					}
					else
					{
						//�������Եȵ���1V��ʱ���Լ��10L/min
						if(IsStartCooking5 == false || (IsStartCooking5 == true && collectionSum == 3))
						{
							if(strcmp(CONFIG_Meter.USE_SENSOR, "1") == 0)//100L������
							{
								if(ADTestVFlow > (0.529) && ADTestVFlow <= 1)
								{
									//��������֤���ڵ�ѹ����0.6341 0.6024 ��ʱ�������㷨һ��,ԼΪ2L/min
									//if(ADTestVFlow > 0.6024)
									if(ADTestVFlow > 0.54)
									{
										TmpRealTimeFlow = (54.0255 * (ADTestVFlow) - 27.6083);// + flow_zero
									}
									else
									{
										//TmpRealTimeFlow = getYvalue(ADTestVFlow + flow_zero) * REAL_DATA_Credit.LPGDensity;
										TmpRealTimeFlow = (35.6297 * (ADTestVFlow) - 18.2077);// + flow_zero
										//printf("TmpRealTimeFlow %f\r\n",TmpRealTimeFlow);
									}
									if(TmpRealTimeFlow > 0 && TmpRealTimeFlow < 20)
									{
										 if(IsStartCooking5 == true)
										 {
											 //printf("IsStartCooking5 true\r\n");
											 SubToMeter(TmpRealTimeFlow * REAL_DATA_Credit.Magnification / 60.0 * 60);//1.227  1.25/2.484=0.5032206ϵ��Ӧ����0.503����// (50.0 * ADTestVFlow - 25.0)
										 }
										 else
										 {
											 //printf("IsStartCooking5 false\r\n");
											 SubToMeter(TmpRealTimeFlow * REAL_DATA_Credit.Magnification / 60.0 * 3);//1.227  1.25/2.484=0.5032206ϵ��Ӧ����0.503����// (50.0 * ADTestVFlow - 25.0)
										 }
										 zeroFlowTime = 0;
									   //printf("�ʢ7����:%.2f\r\n",(REAL_DATA_CALIBRATION.slope * ADTestV + REAL_DATA_CALIBRATION.zero));
									}
									 //������Ҫע�͵�
									 //zeroFlowTime = 0;
								}
								else if(ADTestVFlow > 1)//����1�ǲ����������������������ر�
								{
									zeroFlowTime = 20;
									TmpRealTimeFlow = 0;
								}
								else//ʣ�µ����Ϊ��ⲻ��,���߾��Ȳ��������,��Ϊȼ��¯�ر���
								{
									zeroFlowTime++;
									TmpRealTimeFlow = 0;
								}
							}
							else
							{
							  if(ADTestVFlow > (0.55) && ADTestVFlow <= 2)
								{
									if((REAL_DATA_CALIBRATION.slope - 1) < 0.0000009 && REAL_DATA_CALIBRATION.zero < 0.0000009)
									{
											TmpRealTimeFlow = getY10value(ADTestVFlow) * REAL_DATA_Credit.Magnification;//Equal to the following formula
									}
									else
									{
											TmpRealTimeFlow = (REAL_DATA_CALIBRATION.slope * (ADTestVFlow) + REAL_DATA_CALIBRATION.zero);
									}
									 {
										 if(IsStartCooking5 == true)
										 {
											 //printf("IsStartCooking5 true\r\n");
											 SubToMeter(TmpRealTimeFlow * REAL_DATA_Credit.Magnification / 60.0 * 60);//1.227  1.25/2.484=0.5032206ϵ��Ӧ����0.503����// (50.0 * ADTestVFlow - 25.0)
										 }
										 else
										 {
											 //printf("IsStartCooking5 false\r\n");
											 SubToMeter(TmpRealTimeFlow * REAL_DATA_Credit.Magnification / 60.0 * 3);//1.227  1.25/2.484=0.5032206ϵ��Ӧ����0.503����// (50.0 * ADTestVFlow - 25.0)
										 }
									 }
									 
									 
									 zeroFlowTime = 0;
									 //������Ҫע�͵�
									 //zeroFlowTime = 0;
								}
								else if(ADTestVFlow > 2)//����1�ǲ����������������������ر�
								{
									zeroFlowTime = 20;
									TmpRealTimeFlow = 0;
								}
								else//ʣ�µ����Ϊ��ⲻ��,���߾��Ȳ��������,��Ϊȼ��¯�ر���
								{
									zeroFlowTime++;
									TmpRealTimeFlow = 0;
								}
							}
						}
					}
			  }
			}
			
			if(IsTestPower == true)
			{
				if((REAL_DATA_CALIBRATION.slope - 1) < 0.0000009 && REAL_DATA_CALIBRATION.zero < 0.0000009)
				//if(REAL_DATA_CALIBRATION.slope == 1 && REAL_DATA_CALIBRATION.zero == 0)
				{
				  tmp = (float)ADC_Data[1] / 4096 * 3.3 * 2;
				  //printf("BatteryVoltage1:%.4f\r\n",tmp);
				}
				else
				{
					if((REAL_DATA_CALIBRATION.slope * ADC_Data[1] + REAL_DATA_CALIBRATION.zero) < 2.7)
					{
						tmp = (REAL_DATA_CALIBRATION.slope * ADC_Data[1] + REAL_DATA_CALIBRATION.zero) * 2;
				    //printf("BatteryVoltage2:%.4f\r\n",tmp);
					}
					else
					{
						tmp = (REAL_DATA_CALIBRATION.slope * ADC_Data[1] + REAL_DATA_CALIBRATION.zero);
				    //printf("BatteryVoltage3:%.4f\r\n",tmp);
					}
				}

				if(tmp > 3.3)
				{
		      REAL_DATA_PARAM.BatVoltage = tmp;
          //REAL_DATA_PARAM.BatVoltage = 3.8;
				}
			  tmp = 0;
			}
		}
		if(IsStartCooking5 == true)
		{
			if(collectionSum < 3)
			{
				 collectionSum++;
			   //HAL_ADCEx_Calibration_Start(hadc);
				 //HAL_ADC_Start_DMA(hadc, (uint32_t*)&ADC_Data, 4);
			}
			else
			{
				collectionSum = 0;
				IsTestEnd = true;
			}
		}
		else
		{
			IsTestEnd = true;
		}
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
    if (htim->Instance == TIM6) 
		{
      CPU_RunTime++;
		  //printf("CPU_RunTime:%d \r\n", CPU_RunTime);
			StartRunTime++;
			//printf("StartRunTime:%d \r\n", StartRunTime);
			//����ʱ���һ
//			if(OpenLockTime > 0)
//			{
//				OpenLockTime--;
//			}
//			//������ʱ���һ,ͬʱ����������,�������ֵֻ����һ��ʱ��֮��Ż��Ϊһ����ֵ
//			zeroFlowTime++;
    }
	  if (htim->Instance == TIM7) //ÿ����ѭ��һ��
		{
			//��ʱ��7 ÿ������һ�β��� PA5���Դ�������ѹ,PA7���Ե�ص�ѹ
			//BATAD_POWER(1);��Ӧ��ÿ�β��Ե�ʱ���ڿ���,�ȶ���Ҫʱ��
			if(IsTestFlow == true || IsTestPower == true)
			{
				 HAL_ADCEx_Calibration_Start(&hadc);
				 HAL_ADC_Start_DMA(&hadc, (uint32_t*)&ADC_Data, AvgNumber);
				 //printf("HAL_ADCEx_Calibration_Start\r\n");
			}
			
			//����ʱ���һ
			if(OpenLockTime > 0)
			{
				OpenLockTime--;
			}
			//������ʱ���һ,ͬʱ����������,�������ֵֻ����һ��ʱ��֮��Ż��Ϊһ����ֵ
			zeroFlowTime++;
			
			if(vibrationsNumber > 0)
			{
					vibrationsNumber--;
			}
			
			//printf("CPU_RunTime: %s %s \r\n", __DATE__, __TIME__);
			//run_rtc = Read_Time();
			//printf("min:%d sec:%d\r\n",run_rtc.tm_min,run_rtc.tm_sec);
		}
  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
