
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
/* 用于统计运行时间 */
volatile uint32_t CPU_RunTime = 0UL;
extern volatile bool IsIntoLowPower;

//float flow_zero = 0;//传感器的零值,在阀门关闭的状态采集
uint8_t collectionSum = 0;//采集次数

//extern uint32_t gGasCnt;
extern uint8_t		Usart3RxTemp;
extern REAL_DATA_PARAM_t REAL_DATA_PARAM;   //信号强度等
extern CONFIG_Meter_t CONFIG_Meter;				  //仪表的一些硬件信息
extern uint32_t zeroFlowTime;
extern uint32_t vibrationsNumber;
extern uint32_t OpenLockTime;
extern REAL_DATA_CALIBRATION_t REAL_DATA_CALIBRATION;
extern uint8_t Current_card_type;  //当前卡的类型

extern volatile uint32_t StartRunTime;//系统开始运行的时间,由定时器累加 如果进入低功耗清零
extern volatile bool IsStartCooking5; //是正在做饭并且大于5分钟

extern osThreadId defaultTaskHandle;
extern uint32_t beginMotoTime;

extern REAL_DATA_Credit_t REAL_DATA_Credit; 

extern uint8_t sStep;       //当前屏幕的值

uint32_t ADC_Data[50];
uint32_t ADC_DataAvg;

uint8_t AvgNumber = 4;

bool IsTestPower = false;//是否开启了电源测量
bool IsTestFlow = true;//是否开启了流量测试
bool IsTestEnd = false;//流量测试是否完成

float ADTestVFlow = 0; //校准的时候代表电压,实际用的时候是实时流量
float TmpRealTimeFlow = 0; //校准的时候代表电压,实际用的时候是实时流量

float volt1 = 0;   //第1个点  采集值	低点位
float	y1 = 0;      //第1个点  输入值

float vert2 = 0;    //第2个点  采集值
float	y2 =0;       //第2个点  输入值

//bool IsReadVoltage = false;

extern bool gkeyValue;   //传递的屏幕按键值
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
	
	//移动到获取完内存值得后面
	//MX_RTC_Init();//每次自动生成,需要注释掉,因为这个读取配置后才能调用,有狗与没有的配置不一样
	//MX_IWDG_Init();//每次自动生成后以后需要注释掉这一句,这是启动看门狗的代码,如果自启动大于27秒,需要关闭看门狗,後面自己啟動
	//HAL_NVIC_SetPriority(RTC_IRQn, 3, 0); 在生成工具中如果想要启动定时报警,需要启动这两句,但是自动生成的不灵活,
	//HAL_NVIC_EnableIRQ(RTC_IRQn);所以在后面获得系统值后再启动这个RTC定时器中断
	
//	HAL_TIM_Base_Start_IT(&htim6);
//	HAL_TIM_Base_Start_IT(&htim7);
	
	//TestMB85RS16A();
	//TestW25Q64();
	//在这里开启存储器的电源,读取参数,应该在系统启动的情况下再执行读取等操作,这样有可能通过GPRS发送报警信息
	
	printf("System2 Init ok |%s| |%s|\r\n", __DATE__, __TIME__);
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();
	AppObjCreate(); //创建二值信号量用于标识UART接收完成数据、
	AppObjCreateQueue();//全局消息对列用于指示当前进行的传输指令 
  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
  //todo 如果运行到这里,重启写日志
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

		if(CONFIG_Meter.NotHaveDog == false || Current_card_type == 1)//有狗或者正在做饭
		{
				//printf("sTime %d:%d:%d\r\n",sTime.Hours,sTime.Minutes,sTime.Seconds);
				//SystemClock_Config();
				HAL_RTC_GetAlarm(hrtc, &sAlarm,RTC_ALARM_A,RTC_FORMAT_BIN);
				//printf("sAlarm  %02d:%02d:%02d\r\n",sAlarm.AlarmTime.Hours,sAlarm.AlarmTime.Minutes,sAlarm.AlarmTime.Seconds);
				//HAL_RTC_DeactivateAlarm(&hrtc,RTC_ALARM_A);
				
				sAlarm.AlarmTime.Hours = 0;
				sAlarm.AlarmTime.Minutes = 0;
				sAlarm.AlarmTime.Seconds = (sAlarm.AlarmTime.Seconds + 20) % 60;//20秒
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
				//sAlarm.AlarmTime.Minutes = (sAlarm.AlarmTime.Minutes + 1) % 60;//一分钟
			  sAlarm.AlarmTime.Minutes = (sAlarm.AlarmTime.Minutes / 5 * 5  + 5) % 60;//五分钟
			  //sAlarm.AlarmTime.Minutes = 0;//一小时
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
		if((GPIO_Pin == GPIO_PIN_0   //屏幕按键
			|| GPIO_Pin == GPIO_PIN_1  //光电  PA1
		  || GPIO_Pin == GPIO_PIN_2  //开盖  微动
		  || GPIO_Pin ==GPIO_PIN_12  //摇把
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

	
	  //当这些东西没有的时候才采集电压,避免阀门动作影响电压采集
	  if(glockStatus == motor_null && gmotorStatus == motor_null && gassembleStatus == motor_null && IsIntoLowPower == false)//&& GSM_ON_FLAG == 0 
	  {
		  if(IsTestFlow == true)
		  {
			
				if(Current_card_type == 3)
				{
				    //if(NEEDLE_input == 0)//down 开
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
					//如果针阀是关,阀门是关
					if( strcmp(REAL_DATA_PARAM.ElectricValveStatus, "0")==0)//NEEDLE_input == GPIO_PIN_SET ||  因检测针阀限位有误,不做判断
					{
						
						TmpRealTimeFlow = 0;
					}
					else
					{
						//经过测试等等于1V的时候大约是10L/min
						if(IsStartCooking5 == false || (IsStartCooking5 == true && collectionSum == 3))
						{
							if(strcmp(CONFIG_Meter.USE_SENSOR, "1") == 0)//100L传感器
							{
								if(ADTestVFlow > (0.529) && ADTestVFlow <= 1)
								{
									//经过测试证明在电压等于0.6341 0.6024 的时候两种算法一致,约为2L/min
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
											 SubToMeter(TmpRealTimeFlow * REAL_DATA_Credit.Magnification / 60.0 * 60);//1.227  1.25/2.484=0.5032206系数应该在0.503左右// (50.0 * ADTestVFlow - 25.0)
										 }
										 else
										 {
											 //printf("IsStartCooking5 false\r\n");
											 SubToMeter(TmpRealTimeFlow * REAL_DATA_Credit.Magnification / 60.0 * 3);//1.227  1.25/2.484=0.5032206系数应该在0.503左右// (50.0 * ADTestVFlow - 25.0)
										 }
										 zeroFlowTime = 0;
									   //printf("炜盛7流量:%.2f\r\n",(REAL_DATA_CALIBRATION.slope * ADTestV + REAL_DATA_CALIBRATION.zero));
									}
									 //正常需要注释掉
									 //zeroFlowTime = 0;
								}
								else if(ADTestVFlow > 1)//超过1是不正常的立即触发零流量关闭
								{
									zeroFlowTime = 20;
									TmpRealTimeFlow = 0;
								}
								else//剩下的情况为检测不到,或者精度不够的情况,认为燃气炉关闭了
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
											 SubToMeter(TmpRealTimeFlow * REAL_DATA_Credit.Magnification / 60.0 * 60);//1.227  1.25/2.484=0.5032206系数应该在0.503左右// (50.0 * ADTestVFlow - 25.0)
										 }
										 else
										 {
											 //printf("IsStartCooking5 false\r\n");
											 SubToMeter(TmpRealTimeFlow * REAL_DATA_Credit.Magnification / 60.0 * 3);//1.227  1.25/2.484=0.5032206系数应该在0.503左右// (50.0 * ADTestVFlow - 25.0)
										 }
									 }
									 
									 
									 zeroFlowTime = 0;
									 //正常需要注释掉
									 //zeroFlowTime = 0;
								}
								else if(ADTestVFlow > 2)//超过1是不正常的立即触发零流量关闭
								{
									zeroFlowTime = 20;
									TmpRealTimeFlow = 0;
								}
								else//剩下的情况为检测不到,或者精度不够的情况,认为燃气炉关闭了
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
			//开锁时间减一
//			if(OpenLockTime > 0)
//			{
//				OpenLockTime--;
//			}
//			//零流量时间加一,同时又流量置零,所以这个值只有在一定时间之后才会成为一个大值
//			zeroFlowTime++;
    }
	  if (htim->Instance == TIM7) //每三秒循环一次
		{
			//定时器7 每秒启动一次测试 PA5测试传感器电压,PA7测试电池电压
			//BATAD_POWER(1);不应该每次测试的时候在开启,稳定需要时间
			if(IsTestFlow == true || IsTestPower == true)
			{
				 HAL_ADCEx_Calibration_Start(&hadc);
				 HAL_ADC_Start_DMA(&hadc, (uint32_t*)&ADC_Data, AvgNumber);
				 //printf("HAL_ADCEx_Calibration_Start\r\n");
			}
			
			//开锁时间减一
			if(OpenLockTime > 0)
			{
				OpenLockTime--;
			}
			//零流量时间加一,同时又流量置零,所以这个值只有在一定时间之后才会成为一个大值
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
