/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */     
#include "stm32f0xx_hal.h"
#include "tim.h"
#include "bsp.h"
#include "Sim80X.h"
#include "HeartbeatPacket.h"
#include "RechargePacket.h"
#include "ReportDataPacket.h"
#include "ReportStatePacket.h"
#include "WaringPacket.h"
#include "package.h"
#include "CookingSessionReport.h"
#include "ControlPacket.h"
#include "SetupPacket.h"
#include "CustomerCredit.h"
#include "InformationPacket.h"
#include "UsedResetPacket.h"
#include "iwdg.h"
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId myKeyTaskHandle;
osThreadId myLedTaskHandle;
osThreadId myTaskCmdAnalyzHandle;
osThreadId myTaskSim80xBufHandle;

/* USER CODE BEGIN Variables */
#define Version    0x41

#define MYSYSCLK 48 

char *LockPassword ="32321";
uint8_t gkeySum = 0;     //当前输入密码的位数

//错误信息
bool No_valid_Card = false;//无效卡,默认假 就是有效
volatile bool IsNeedRestart = false;
bool IsNeedTimeRestart = false;  //是否定时重启

//log
uint32_t logStart = 0;
uint32_t logEnd = 0;
bool IsNeedReportLog = false;  //是否导出日志

uint32_t VoltageStart = 0;
uint32_t VoltageEnd = 0;
bool IsNeedReportVoltage  = false;

//屏幕有关
volatile bool gkeyValue = false;   //传递的屏幕按键值
uint8_t sStep = 0;       //当前屏幕的值
uint32_t LCD_Delay_Time  = 0;    //显示的时间
bool Lcd_Status = false; //屏幕状态默认关

LogTypeDef_t LogTypeDef;

//盖子锁相关
//bool lockValue = false;   //锁微动的值,长按下是true,用于屏幕按键触发关锁
bool LidStatus = false;   //盖子的真实状态
bool NEEDLEStatus = false;   //针阀的真实状态
uint32_t OpenLockTime = 0;    //开锁计时,当每次发送开锁指令后,如果指定时间内没有操作,自动关锁

//联网步骤相关
volatile int16_t connectStep = -1;      //连接的步骤,停止无连接等于零,每成功一步加一 -1为联网之前

//发送报文相关
bool IsHaveSimError = false;
bool IsNeedTimeing = false;
bool IsSendHalfTime = false;//是否已经发送了一半的定时报
//uint8_t TimeRetryNumber = 0;

uint8_t HearRetryNumber = 0; //当没有其它需要发送是beat的发送次数
bool IsNeedWarning = false;
uint8_t WarnRetryNumber = 0;
bool IsNeedCustomerCreditResponse = false;
bool IsNeedInformationResponse = false;
bool IsNeedSendCook = false; //做完饭上报

bool IsSendedCSRP = false; //是否发送过在等待回复
bool IsReceivedCSRPReply = false; //是否接收到回复

bool IsNeedIllegal_lid_opening = false;//是否需要触发开盖报警,默认是触发的,只有当刷卡,开盖之后就不触发了
uint32_t WarnTime = 0;//第一次报警的时间 //由于低功耗的时候,所有定时器运行不正常,所以这个定时器的功能改由RTC代替

bool IsLowPowerWarned = false;

uint32_t TimeForPowerStart=0;  //开始上电的时间
uint32_t TimeForCurrStart=0;   //当前步骤开始运行的时间
uint32_t loopSumCurr=0;        //循环重试的次数
uint32_t loopSumPDP=0;         //循环重试的次数 当移动场景建立后,连接的时候可能不成功,这个时候要往前跳步骤,但有可能一直不成功,所以需要增加变量
//////////////////////////////////////////////////////////////////
//bool IstefffmpSended = false;//是否已经发送了
//extern uint32_t sumCnt;
//extern uint16_t identCnt;

//刷卡相关
volatile uint8_t Current_card_type = 0;  //当前卡的类型

//需要回复的命令相关
volatile uint8_t commandType = 0;

//char* updata = "123456";
//char updata2[7];
//volatile uint32_t gGasCnt = 0;

//存储的结构体
CONFIG_GPRS_t CONFIG_GPRS;           //联网配置信息
bool IsSaveCONFIG_GPRS = false;

CONFIG_Meter_t CONFIG_Meter;				 //仪表的一些硬件信息
bool IsSaveCONFIG_Meter = false;

REAL_DATA_PARAM_t REAL_DATA_PARAM;   //信号强度等
bool IsSaveREAL_DATA_PARAM = false;

REAL_DATA_Credit_t REAL_DATA_Credit; //仪表充值减值信息
bool IsSaveREAL_DATA_Credit = false;

Current_Credit_t Current_Credit;
Current_Credit_t Tmp_Credit;

Current_Log_t Current_Log;
bool IsSaveREAL_Current_Log = false;

Current_Voltage_t Current_Voltage;
bool IsSaveREAL_Current_Voltage = false;

float StartCredit = 0; //每次做饭开始的信用
tim_t StartCookingSeconds; //每次做饭开始的时间
volatile bool IsStartCooking5 = false; //是正在做饭并且大于5分钟


REAL_DATA_CALIBRATION_t REAL_DATA_CALIBRATION;
bool IsSaveCALIBRATION = false;
bool IsStartCALIBRATION = false;

REAL_Flow_CALIBRATION_t REAL_Flow_CALIBRATION;
bool IsSaveFlowCALIBRATION = false;

update_t update;
bool IsSaveUpdate = false;

//按键相关
key_Status_t key1_Status;						//表盖按键的状态  上升沿触发  正常为高
key_Status_t Key2_Status;				    //开盖锁的状态    上升沿触发  正常状态为高  从倾斜到正常为继电器断开,由低到高,上升沿触发
key_Status_t Key3_Status;				    //针阀的状态      下降沿触发  正常状态为高  按下继电器,就是打开针阀为低,触发

bool haveClickButton = false;            //是否出现单击事件
bool haveLongButton = false;             //是否出现长按事件

key_Status_t OpenLid_Status;			  //光电传感器的状态,暗着的时候为0,开盖为1,上升沿触发

//运行时参数
volatile uint32_t zeroFlowTime = 0;//当流量等于零的时候每一秒增加一,当大于一分钟时,关闭电磁阀
volatile uint32_t StartRunTime = 0;//每次从低功耗回来后置零,只有大于2秒才能再次进入低功耗
volatile bool IsIntoLowPower = false;//是进入低功耗了,这个时候正在走进入低功耗的流程,不采集数据

volatile uint32_t vibrationsNumber = 0;//用于检测仪表的震动数目

//串口设置缓存
extern cmd_analyze_struct cmd_analyze;
extern uint32_t rec_count;
//屏幕结构体
extern LCDPanel_TypeDef LCDPanelStruct;

//AD的数据
extern bool IsTestPower;//是否开启了电源测量
extern bool IsTestFlow;//是否开启了流量测试

extern bool IsTestEnd;//流量测试是否完成

extern uint32_t ADC_Data[4];


extern float volt1;
extern float	y1;

extern float vert2;
extern float	y2;

extern float ADTestVFlow;
//extern float flow_zero;//传感器的零值,在阀门关闭的状态采集

extern float TmpRealTimeFlow; //校准的时候代表电压,实际用的时候是实时流量
extern float rechargeCredit;

//extern bool IsReadVoltage;

//uint8_t t_W25Q64[] = "132486";
//uint8_t r_W25Q64[7];
//extern TestStatus TransferStatus1;

/*命令表*/
const cmd_list_struct cmd_list[]= {
    /* 命令    					参数数目    	处理函数        帮助信息                         */
    {"hello",  							 0,      printf_hello,        "HelloWorld!"},
    {"arg",     						 8,      handle_arg,          "arg <arg1> <arg2>"},
    {"ReadSystemPara",       0,      ReadSystemPara,      "return"},
    {"SetAddr",       			 1,      SetAddr,      "SetAddr"},
    {"DTISETTIME",       		 1,      DTISETTIME,      "DTISETTIME"},
    {"UpDuty",       		 		 1,      UpDuty,      "UpDuty"},
    {"StartDuty",       		 1,      StartDuty,      "StartDuty"},
    {"HMI_type",       		 	 1,      HMI_type,      "HMI_type"},
//{"Lock_version",       1,      Lock_version,      "Lock_version"},
    {"WarnBattery",       	 1,      WarnBattery,      "WarnBattery"},
    {"WarnPerpaid",       	 1,      WarnPerpaid,      "WarnPerpaid"},
    {"WarnVolume",      		 1,      WarnVolume,      "WarnVolume"},
    {"NominalCapacity",      1,      NominalCapacity,      "NominalCapacity"},
    {"GasRemain",            1,      GasRemain,      "GasRemain"},
    {"SetIP",       		 		 5,      SetIP,      "SetIP"},
    {"RESETALL",       		 	 0,      RESETALL,      "RESETALL"},
    {"ClearZero",       		 0,      ClearZero,      "ClearZero"},
    {"USE_GPRS_APN",       	 1,      USE_GPRS_APN,      "USE_GPRS_APN"},
    {"USE_SENSOR",       	   1,      USE_SENSOR,      "USE_SENSOR"},
    {"SetAPN",       		 		 1,      SetAPN,      "SetAPN"},
    {"SetName_APN",       	 1,      SetName_APN,      "SetName_APN"},
    {"SetAP_N_secret",       1,      SetAP_N_secret,      "SetAP_N_secret"},
		{"SetPin",               1,      SetPin,             "SetPin"},
    {"RealTimeFlow",         1,      RealTimeFlow,      "RealTimeFlow"},
    {"ResetSystem",          1,      ResetSystem,      "ResetSystem"},
    {"SetParaming",          1,      SetParaming,      "SetParaming"},
    {"ExportLog",            2,      ExportLog,       "ExportLog"},
    {"FindLogIndex",         1,      FindLogIndex,      "FindLogIndex"},
    {"VeryLowPower",         1,      VeryLowPower,      "VeryLowPower"},
    {"readVoltage",          0,      readVoltage,      "readVoltage"},
    {"writeVoltage",         10,     writeVoltage,      "writeVoltage"},
    {"ReadHisVoltage",       3,      ReadHisVoltage,    "ReadHisVoltage"}
};
/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void StartkeyTask(void const * argument);
void StartLedTask(void const * argument);
void StartCmdAnalyzTask(void const * argument);
void Sim80xBuffTask(void const * argument);
void M26test_Task(void const * argument);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */
void IntoLowPower();
/* USER CODE END FunctionPrototypes */

/* Hook prototypes */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
    //操作内存的初始值,以及复位电机

    //按键赋初值,以后就不用管了
    key1_Status.index_input = 1;
    key1_Status.key_m = key_state_0;
    key1_Status.key_time_1 = 0;
    key1_Status.key_state = key_state_0;
    key1_Status.key_time = 0;

    //开盖的微动
    Key2_Status.index_input = 2;
    Key2_Status.key_m = key_state_0;
    Key2_Status.key_time_1 = 0;
    Key2_Status.key_state = key_state_0;
    Key2_Status.key_time = 0;

    //针阀的微动OpenLid_Status
    Key3_Status.index_input = 3;
    Key3_Status.key_m = key_state_0;
    Key3_Status.key_time_1 = 0;
    Key3_Status.key_state = key_state_0;
    Key3_Status.key_time = 0;

    //光敏电阻的状态OpenLid_Status
		OpenLid_Status.index_input = 4;
		OpenLid_Status.key_m = key_state_0;
		OpenLid_Status.key_time_1 = 0;
		OpenLid_Status.key_state = key_state_0;
		OpenLid_Status.key_time = 0;

    //初始化的时候先用程序给结构体赋初值
    memset(&CONFIG_GPRS,0,sizeof(CONFIG_GPRS_t));

    //strcpy(CONFIG_GPRS.Server_IP,"47.95.200.195");
    strcpy(CONFIG_GPRS.Server_IP,"35.177.78.198");
    strcpy(CONFIG_GPRS.Socket_Port,"5057");
    strcpy(CONFIG_GPRS.USE_GPRS_APN,"1");
    //strcpy(CONFIG_GPRS.APN,"iot-eu.aer.net");
    //strcpy(CONFIG_GPRS.APN,"kopagas");
		strcpy(CONFIG_GPRS.APN,"vpn.mgas.com");
    memset(CONFIG_GPRS.APN_UserName,0,sizeof(CONFIG_GPRS.APN_UserName));
    memset(CONFIG_GPRS.APN_Password,0,sizeof(CONFIG_GPRS.APN_Password));
    memset(CONFIG_GPRS.CenterSimNumber,0,sizeof(CONFIG_GPRS.CenterSimNumber));
    memset(CONFIG_GPRS.ServerSimNumber,0,sizeof(CONFIG_GPRS.ServerSimNumber));
    strcpy(CONFIG_GPRS.Communications_Type,"1");

    memset(&CONFIG_Meter,0,sizeof(CONFIG_Meter_t));

    strcpy(CONFIG_Meter.MeterNo,"XXXX000666");
    CONFIG_Meter.MeterNoLength = 10;

    strcpy(CONFIG_Meter.CURRENCY,"KSh");
    CONFIG_Meter.CYLINDER_CAPACITY = 13;
    strncpy(CONFIG_Meter.CYLINDER_No,"XXXXXXXXXXX",11);
    strcpy(CONFIG_Meter.BATTERY_MODEL,"TX123");
    CONFIG_Meter.UpDuty = 360;// 分钟 24小时
    CONFIG_Meter.StartDuty = 180;// 分钟 24小时
    CONFIG_Meter.LowBattery = 3.7;
    CONFIG_Meter.LowCredit = 200;
    CONFIG_Meter.LowGasVolume = 1;
    strcpy(CONFIG_Meter.Longitude,"0000000000");
    strcpy(CONFIG_Meter.Latitude,"0000000000");
    strcpy(CONFIG_Meter.USE_SENSOR,"2");
    CONFIG_Meter.IsHaveRFID = true;
    CONFIG_Meter.NotHaveDog = false;
    CONFIG_Meter.ResetSum = 0;
    strcpy(CONFIG_Meter.CompileDate,__DATE__);
    strcpy(CONFIG_Meter.CompileTime,__TIME__);
    CONFIG_Meter.IsNormalReset = false;
    CONFIG_Meter.NormalResetSum = 0;

    memset(&REAL_DATA_PARAM,0,sizeof(REAL_DATA_PARAM_t));

    LidStatus = (Opening_input == 1 ? true : false);         //盖子的真实状态
    NEEDLEStatus = (NEEDLE_input == GPIO_PIN_RESET ? true : false);;   //针阀的真实状态

    strcpy(REAL_DATA_PARAM.LidSensorStatus,Opening_input == 1 ? "1" : "0");
    strcpy(REAL_DATA_PARAM.LidElectricLock,"0");//未锁,既是开
    strcpy(REAL_DATA_PARAM.NEEDLESensorStatus,NEEDLE_input == GPIO_PIN_SET ? "0" : "1");
    strcpy(REAL_DATA_PARAM.ElectricValveStatus,"0");//阀门是关着的
    strcpy(REAL_DATA_PARAM.TankSensorStatus,"1");//无用
    strcpy(REAL_DATA_PARAM.TankLockStatus,"0");//锁是关闭的

    REAL_DATA_PARAM.TankQuality = 13;//单位千克

    memset(&REAL_DATA_Credit,0,sizeof(REAL_DATA_Credit_t));

    REAL_DATA_Credit.LPGDensity = 2.525;
    REAL_DATA_Credit.CumulationCredit = 0;
    REAL_DATA_Credit.CumulationGasL = 0;
    REAL_DATA_Credit.UnitPrice = 233;
    REAL_DATA_Credit.UsedKg = 0;
    REAL_DATA_Credit.CookingSessionSendNumber = 0;
    REAL_DATA_Credit.CookingSessionEnd = 0;

    REAL_DATA_Credit.CardTotal = 0;

    Current_Log.indexSendNumber = 0;
    Current_Log.indexCurrent = 0;
    Current_Log.addrBigData = 0x500000;

    memset(&Current_Credit,0,sizeof(Current_Credit_t));
    Current_Credit.CurrCredit = 0;
		
		update.BOOTFLAG = 0x00;

    LL_VCC(1);   //存储器上电并弄成可以写的状态

    ENBOOST_PWR(1); //6.5伏电压,给传感器,电磁阀供电 1.1mA
    GPRS_PWR(1);

    //开机测试电压,整点测试电压,以及屏幕亮的时候测试电压
    BATAD_POWER(1);	//0.05mA
    IsTestPower = true;

    HAL_Delay(100);
//
//	__HAL_RCC_SPI2_CLK_DISABLE();

    //printf("SPI_read_MB85RS16A %d\r\n",SPI_read_MB85RS16A(0));
    //如果开始两个字节为22,说明不是第一次启动了
    if(SPI_read_MB85RS16A(0) == Version && SPI_read_MB85RS16A(1) == Version)
    {
        //printf("read:ok\r\n");
        CONFIG_GPRS_READ();
        CONFIG_Meter_READ();
        REAL_DATA_PARAM_READ();
        REAL_DATA_Credit_READ();

        //if(CONFIG_Meter.IsHaveRFID == false)
        {
            REAL_Current_Credit_READ();
        }

        REAL_Current_Log_READ();
				
				BOOTFLAG_ADDR_READ();

//		printf("REAL_DATA_Credit %f\r\n",REAL_DATA_Credit.CumulationCredit);
//		printf("REAL_DATA_Credit %f\r\n",REAL_DATA_Credit.CumulationGasL);
//		printf("REAL_DATA_Credit %f\r\n",REAL_DATA_Credit.UsedKg);
		  printf("REAL_DATA_Credit %f\r\n",REAL_DATA_Credit.Magnification);
//		printf("REAL_DATA_Credit %f\r\n",REAL_DATA_Credit.LPGDensity);
//		printf("REAL_DATA_Credit %f\r\n",REAL_DATA_Credit.UnitPrice);
//		printf("REAL_DATA_Credit %d\r\n",REAL_DATA_Credit.CardTotal);
//		printf("REAL_DATA_Credit %d\r\n",REAL_DATA_Credit.CookingSessionSendNumber);
//		printf("REAL_DATA_Credit %d\r\n",REAL_DATA_Credit.CookingSessionEnd);
    }
    else
    {
        printf("write:ok\r\n");
        SPI_write_MB85RS16A(Version,0);
        SPI_write_MB85RS16A(Version,1);

        CONFIG_GPRS_Write();
        CONFIG_Meter_Write();
        REAL_DATA_PARAM_Write();
        REAL_DATA_Credit_Write();
        REAL_Current_Log_Write();
    }

    //REAL_DATA_Credit.CardTotal = 3;
    //REAL_DATA_Credit_Write();

    MX_RTC_Init();//每次自动生成的部分需要手工注释掉

    HAL_NVIC_SetPriority(RTC_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(RTC_IRQn);

    //没有狗等于假,说明有狗等于真,启动看门狗
    if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
    {
        //MX_IWDG_Init();//每次自动生成后以后需要注释掉这一句,这是启动看门狗的代码,如果自启动大于27秒,需要关闭看门狗
    }
		
    strcpy(CONFIG_Meter.FIRMWARE_V,GetSoftWareTime()); 
		
		strcpy(CONFIG_Meter.PCB_V,PCB_Version);

    strcpy(CONFIG_Meter.CompileDate,__DATE__);
    strcpy(CONFIG_Meter.CompileTime,__TIME__);

//	char dddd[] = "##AD";
//	char *erp = dddd;
//	for(uint16_t i=0 ; i < sizeof(dddd) - 1; i++)
//	{
//			DecodeTask(*(erp++));
//	}
//	char aaa[] = "000D/201903041229/192**##ss**";
//	char *esrp = aaa;
//	for(uint16_t i=0 ; i < sizeof(aaa) - 1; i++)
//	{
//			DecodeTask(*(esrp++));
//	}

//	if(SPI_read_MB85RS16A(0) == 0x22 && SPI_read_MB85RS16A(1) == 0x22)
//	{
//		printf("Flash1:OK\r\n");
//	}
//	SPI_FLASH_BufferWrite(t_W25Q64,0x00000,6);
//	SPI_FLASH_BufferRead(r_W25Q64, 0x00000,6);
//	if(strcmp(r_W25Q64, "132486")==0)
//	{
//		printf("Flash2:OK\r\n");
//	}
//	TransferStatus1 = Buffercmp(t_W25Q64,r_W25Q64,6);
//	if( PASSED == TransferStatus1 )
//	{
//		printf("Flash2:OK\r\n");
//	}
    CALIBRATION_Voltage_READ();//读校准电压

    printf("slope %f\r\n",REAL_DATA_CALIBRATION.slope);
    printf("zero %f\r\n",REAL_DATA_CALIBRATION.zero);

    if(REAL_DATA_CALIBRATION.slope < 0.0001)
    {
        REAL_DATA_CALIBRATION.slope = 1;
        IsSaveCALIBRATION = true;
    }

    if(REAL_DATA_CALIBRATION.zero < -100)
    {
        REAL_DATA_CALIBRATION.zero = 0;
        IsSaveCALIBRATION = true;
    }
    if(REAL_DATA_Credit.Magnification < 0.5)
    {
        REAL_DATA_Credit.Magnification = 1;
        IsSaveREAL_DATA_Credit = true;
    }

    CALIBRATION_Flow_READ();//读流量校准

    if(REAL_Flow_CALIBRATION.ParamNumber <= 1)
    {
        REAL_Flow_CALIBRATION.ParamNumber = 6;
        REAL_Flow_CALIBRATION.ABCDEF[0] = -70552.6641533807;
        REAL_Flow_CALIBRATION.ABCDEF[1] = 640186.66667379939;
        REAL_Flow_CALIBRATION.ABCDEF[2] = -2321522.5705936439;
        REAL_Flow_CALIBRATION.ABCDEF[3] = 4205713.801771218;
        REAL_Flow_CALIBRATION.ABCDEF[4] = -3806560.5313984714;
        REAL_Flow_CALIBRATION.ABCDEF[5] = 1377143.6443224344;

        IsSaveFlowCALIBRATION = true;
    }

    //重置阀门状态
    if(strcmp(REAL_DATA_PARAM.LidElectricLock, "1")==0)
    {
        strcpy(REAL_DATA_PARAM.LidElectricLock,"0");
        glockStatus = motor_open;
    }

    if(strcmp(REAL_DATA_PARAM.ElectricValveStatus, "1")==0)
    {
        strcpy(REAL_DATA_PARAM.ElectricValveStatus,"0");
        gmotorStatus = motor_close;
    }
		
    if(CONFIG_Meter.IsNormalReset == false)
    {
        if(strcmp(REAL_DATA_PARAM.TankLockStatus, "1")==0)
        {
            strcpy(REAL_DATA_PARAM.TankLockStatus,"0");
            gassembleStatus = motor_open;
        }

        OpenLockTime = 2;
        glockSet = motor_open;

        gkeyValue = true;//非正常重启,亮屏幕
    }

    printf("CONFIG_GPRS %d bytes\r\n",sizeof(CONFIG_GPRS));
    printf("SetIP 1 %s PORT %s\r\n",CONFIG_GPRS.Server_IP,CONFIG_GPRS.Socket_Port);

    printf("USE_GPRS_APN %s\r\n",CONFIG_GPRS.USE_GPRS_APN);
    printf("SetAPN %s\r\n",CONFIG_GPRS.APN);
    printf("SetName_APN %s\r\n",CONFIG_GPRS.APN_UserName);
    printf("SetAP_N_secret %s\r\n",CONFIG_GPRS.APN_Password);
		printf("SetPin %s\r\n",CONFIG_GPRS.Pin);
    printf("----------------------------------\r\n");
    printf("CONFIG_Meter %d bytes\r\n",sizeof(CONFIG_Meter));
    printf("SetAddr %s\r\n",CONFIG_Meter.MeterNo);
//	long date,time;//UTC
//	GetRTC(&time,&date);
//	printf("SETTIME %08ld%06ld\r\n",date,time);
    //printf("MeterNoLength %d\r\n",CONFIG_Meter.MeterNoLength);
    printf("FIRMWARE_V %s%s\r\n",FIRMWARE_Version,CONFIG_Meter.FIRMWARE_V);
    printf("PCB_VERSION %s\r\n",PCB_Version);
    printf("CURRENCY %s\r\n",CONFIG_Meter.CURRENCY);
    printf("CYLINDER_No %s\r\n",CONFIG_Meter.CYLINDER_No);
    printf("CYLINDER_CAPACITY %f\r\n",CONFIG_Meter.CYLINDER_CAPACITY);
    printf("BATTERY_MODEL %s\r\n",CONFIG_Meter.BATTERY_MODEL);
    printf("UpDuty %d\r\n",CONFIG_Meter.UpDuty);
    printf("StartDuty %d\r\n",CONFIG_Meter.StartDuty);
    printf("VeryLowPower %d\r\n",CONFIG_Meter.NotHaveDog);
    printf("WarnBattery %f\r\n",CONFIG_Meter.LowBattery);
    printf("WarnPerpaid %f\r\n",CONFIG_Meter.LowCredit);
    printf("WarnVolume %f\r\n",CONFIG_Meter.LowGasVolume);
    printf("NominalCapacity %.3f\r\n",CONFIG_Meter.CYLINDER_CAPACITY);
    printf("GasRemain %.3f\r\n",REAL_DATA_PARAM.TankQuality);
    printf("USE_SENSOR %s\r\n",CONFIG_Meter.USE_SENSOR);
    //printf("Longitude %s\r\n",CONFIG_Meter.Longitude);
    //printf("Latitude %s\r\n",CONFIG_Meter.Latitude);
    printf("CompileDate %s\r\n",__DATE__);
    printf("CompileTime %s\r\n",__TIME__);
    printf("ResetSum %d\r\n",CONFIG_Meter.ResetSum);
    printf("ResetTime %s\r\n",CONFIG_Meter.ResetTime);
    printf("NormalResetSum %d\r\n",CONFIG_Meter.NormalResetSum);
    printf("----------------------------------\r\n");
    printf("REAL_DATA_PARAM %d bytes\r\n",sizeof(REAL_DATA_PARAM));
    printf("CumulationCredit %f\r\n",REAL_DATA_Credit.CumulationCredit);
    printf("CumulationGasL %f\r\n",REAL_DATA_Credit.CumulationGasL);
    printf("UsedKg %f\r\n",REAL_DATA_Credit.UsedKg);
    printf("CardTotal %d\r\n",REAL_DATA_Credit.CardTotal);
    printf("CookingSessionSendNumber %d\r\n",REAL_DATA_Credit.CookingSessionSendNumber);
    printf("CookingSessionEnd %d\r\n",REAL_DATA_Credit.CookingSessionEnd);
    printf("----------------------------------\r\n");
    printf("Current_Credit %d bytes\r\n",sizeof(Current_Credit_t));
    int16_t TmpIndex = 0;
    if(REAL_DATA_Credit.CardTotal > 0)//如果内存中有卡,先从内存中寻找
    {
        while(TmpIndex < REAL_DATA_Credit.CardTotal)
        {
            memset(&Tmp_Credit,0,sizeof(Current_Credit_t));
            Current_Credit_READ(TmpIndex);//刷卡的时候挨个读取内存中的卡信息,并比对
            //把搜索到的卡片信息打印出来
            printf("CUSTOMER_ID %s\r\n",Tmp_Credit.CUSTOMER_ID);
            printf("CUSTOMER_ID.length %d\r\n",Tmp_Credit.CUSTOMERNoLength);
            printf("CARD_ID %s\r\n",Tmp_Credit.CARD_ID);
            printf("CARD_ID.length %d\r\n",Tmp_Credit.CARDNoLength);
            printf("CurrCredit %f\r\n",Tmp_Credit.CurrCredit);

            //每次都判断是否与当前卡的信息一致,如果不一致按照当前卡片走,并且写入大内存
            //这个的作用是防止大内存写失败,主动重启不丢数据,但现在造成了调试过程中丢失重启丢失数据
            //如果充值了但是又没有使用,就是没有刷新当前卡,这个值在重启后就丢失了
//            if(stringCmp(Tmp_Credit.CUSTOMER_ID,Current_Credit.CUSTOMER_ID,7) == PASSED
//                    && stringCmp(Tmp_Credit.CARD_ID,Current_Credit.CARD_ID,18) == PASSED)
//            {
//                //printf("CurrCredit:%f\r\n\r\n",Current_Credit.CurrCredit);
//                Tmp_Credit.CurrCredit = Current_Credit.CurrCredit;
//                Current_Credit_Write(Tmp_Credit);
//            }

            TmpIndex++;
        }
    }
		printf("----------------------------------\r\n");
		printf("Current_Log %d bytes\r\n",sizeof(Current_Log));
		printf("----------------------------------\r\n");
		printf("Current_Voltage %d bytes\r\n",sizeof(Current_Voltage));
		printf("----------------------------------\r\n");
		printf("update %d bytes\r\n",sizeof(update));
    printf("----------------------------------\r\n");
    printf("total %d bytes\r\n",sizeof(CONFIG_GPRS) + sizeof(CONFIG_Meter)+ sizeof(REAL_DATA_PARAM)+ sizeof(REAL_DATA_Credit) + sizeof(Current_Credit) + sizeof(Current_Log) + sizeof(Current_Voltage) +sizeof(update));
    printf("----------------------------------\r\n");

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

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
//  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 512);
//  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

//  /* definition and creation of myKeyTask */
//  osThreadDef(myKeyTask, StartkeyTask, osPriorityNormal, 0, 256);
//  myKeyTaskHandle = osThreadCreate(osThread(myKeyTask), NULL);

//  /* definition and creation of myLedTask */
//  osThreadDef(myLedTask, StartLedTask, osPriorityNormal, 0, 512);
//  myLedTaskHandle = osThreadCreate(osThread(myLedTask), NULL);

//  /* definition and creation of myTaskCmdAnalyz */
//  osThreadDef(myTaskCmdAnalyz, StartCmdAnalyzTask, osPriorityNormal, 0, 256);
//  myTaskCmdAnalyzHandle = osThreadCreate(osThread(myTaskCmdAnalyz), NULL);

	osThreadDef(M26test, M26test_Task, osPriorityNormal, 0, 512);
  myTaskCmdAnalyzHandle = osThreadCreate(osThread(M26test), NULL);
  /* definition and creation of myTaskSim80xBuf */
  osThreadDef(myTaskSim80xBuf, Sim80xBuffTask, osPriorityNormal, 0, 256);
  myTaskSim80xBufHandle = osThreadCreate(osThread(myTaskSim80xBuf), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
    //承担执行开关阀,充钱,减值的操作 刷卡等的操作

    tim_t run_rtc;
	  tim_t StartWarn; //开始报警的时间

    long date,time;//UTC

    //rfid需要的变量
    uchar getdata[16];
    uint8_t ascii_i;
    uint8_t keyA[6] = {0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC};
    //uint8_t keyB[6] = {0x20, 0x19, 0x02, 0x20, 0x03, 0x44};
    uint8_t process;
    uint8_t status_RFID;//执行命令后状态
    uint8_t Card_cap;//卡片容量
//	uint8_t M_Card_type = 0;  //卡的类型,应用的类型
    uint8_t Type_serial_number[2];   //卡的类型比如M1什么的
    uint8_t Card_serial_number[5];   //卡片的固有号码,不可更改
    char Use_serial_number[32];      //使用的卡片号码
    uint8_t TmpCARDNoLength = 0;
    char Use_CUSTOMER_number[32];    //客户号
    uint8_t TmpCUSTOMERNoLength = 0;
    char Use_Convert[32];           //从卡里读取的转换率
    uint8_t TmpConvertLength = 0;
    float Tmp_convert = 0;//转换成浮点数的转换率
		char Use_Date[32];           //从卡里读取的可以开锁日期
		uint8_t TmpCardDateLength = 0;
		long Card_lock_date; //卡片里面允许的开锁日期

    //处理上电需要干什么,比如发送心跳包,或者时间到了,定时上报,或者要报警了
    uint32_t TimeForSlowRunFree = 0;//查询CSQ的时间
    //uint32_t TimeForBEATRun = 0;//心跳包的时间

    uint32_t TimeForSlowRunGPRSFree = 0;
		//uint32_t TimeForSlowCSRP = 0;
		
    //char str[64];
    char pAPN[17];
    char pAPN_UserName[17];
    char pAPN_Password[17];

    osDelay(500);

    
    //如果是非正常重启
    if(CONFIG_Meter.IsNormalReset == false)
    {
        GetRTC(&time,&date);
        sprintf(CONFIG_Meter.ResetTime,"%06ld%06ld",date,time);
        CONFIG_Meter.ResetSum++;
        CONFIG_Meter_Write();

        LogWrite(RestartUnNorm,"HardFault",NULL);
    }
    else
    {
        GetRTC(&time,&date);
        CONFIG_Meter.IsNormalReset = false;
        CONFIG_Meter.NormalResetSum++;
        CONFIG_Meter_Write();
    }
		
//		Cooking_Session_READ(REAL_DATA_Credit.CookingSessionSendNumber);//发送的时候从开始位置开始读取,发送成功,索引加一
//    char sendBuffer[200]={0};
//	  refreshCookingSessionReport(&CookingSessionReport);
//	  encodeCookingSessionReport(sendBuffer,&CookingSessionReport);
//    printf("sendBuffer:%s\r\n",sendBuffer);
    //printf("power down0 %ld\r\n",time);
    /* Infinite loop */
    for(;;)
    {
        //系统运行侦测
        //printf("\r\npower\r\n");
        //HAL_GPIO_TogglePin(RUN_LED_GPIO_Port,RUN_LED_Pin);

        //存储数据
        if(IsSaveCONFIG_GPRS == true)
        {
            LL_VCC(1);
            CONFIG_GPRS_Write();
            IsSaveCONFIG_GPRS = false;
        }

        if(IsSaveCONFIG_Meter == true)
        {
            LL_VCC(1);
            CONFIG_Meter_Write();
            IsSaveCONFIG_Meter = false;

            if(IsNeedRestart == true)
            {
							  IntoLowPower();
//                HAL_NVIC_SystemReset();
            }
        }

        if(IsSaveREAL_DATA_PARAM == true)
        {
            LL_VCC(1);
            REAL_DATA_PARAM_Write();
            IsSaveREAL_DATA_PARAM = false;
        }

        if(IsSaveREAL_DATA_Credit == true)
        {
            LL_VCC(1);
            REAL_DATA_Credit_Write();
            IsSaveREAL_DATA_Credit = false;
        }

        if(IsSaveREAL_Current_Log == true)
        {
            LL_VCC(1);
            REAL_Current_Log_Write();
            IsSaveREAL_Current_Log = false;
        }
				
				if(IsSaveREAL_Current_Voltage == true)
				{
					 LL_VCC(1);
					 REAL_Current_Voltage_Write();
					 IsSaveREAL_Current_Voltage = false;
				}

        if(IsSaveCALIBRATION == true)
        {
            LL_VCC(1);
            CALIBRATION_Voltage_Write();
					
            HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
            osDelay(100);
            HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);

            IsSaveCALIBRATION = false;
        }

        if(IsSaveFlowCALIBRATION == true)
        {
            LL_VCC(1);
            CALIBRATION_Flow_Write();
            IsSaveFlowCALIBRATION = false;
        }

        if(IsNeedReportLog == true)
        {
            IsNeedReportLog = false;
            LogRead(logStart,logEnd);
        }
				
				if(update.BOOTFLAG == 0x22)
				{			
						IsNeedInformationResponse = true;
						update.BOOTFLAG = 0x00;
					  //IsSaveUpdate = true;
					  BOOTFLAG_ADDR_Write();
				}
				
				if(IsSaveUpdate == true)
        {
            IsSaveUpdate = false;
					
            BOOTFLAG_ADDR_Write();
					
					  osDelay(100);
					  
					  IntoLowPower();
//					  HAL_NVIC_SystemReset();
        }

//        int i = 0;
//        W25Q64_Write((uint8_t*)&updata,0x500000,6);
//        printf("MD5:");
//        for(i=0; i< 6; i++) 
//				{
//            printf("%02x", updata[i]);
//        }
//        printf("\r\n");
//        memset(&updata2,0,6);
//        //memset(&updata.URL_ADDR,0,128);
//        SPI_FLASH_BufferRead((uint8_t*)&updata2,0x500000,6);
//        printf("MD5:");
//				i = 0;
//        for(i=0; i<6; i++) 
//				{
//            printf("%02x", updata2[i]);
//        }
//        printf(" StrSetbuf %s\r\n",updata.URL_ADDR);
//        printf("bb:%02x\r\n", updata.BOOTFLAG);

        osDelay(100);

        //RFID测试代码,当系统为有RFID刷卡的时候
        if(CONFIG_Meter.IsHaveRFID == true)
        {
            if(IsTestRFID == true && StartRunTime > 1)
            {
                RFID_Reset(1);
                RFID_POWER(1);
                //			printf("\r\n~~~IsTestRFID~~~\r\n");
                //if(IsInitRFID == false)
                {
                    IsInitRFID = true;
                    MFRC522_Init();
                }
                //if(IsLockCard == false)
                {
                    //Find cards 寻卡,返回卡类型
                    status_RFID = MFRC522_Request(PICC_REQIDL, Type_serial_number);
                    if (status_RFID == MI_OK) //检测到卡
                    {
                        //printf("\r\n~~~Card detected~~~\r\n");
                        //printf("Card Type : %3d\r\n",Type_serial_number[0]);

                        // Anti-collision, return card serial number == 4 bytes
                        //防冲撞,返回卡的序列号
                        status_RFID = MFRC522_Anticoll(Card_serial_number);
                        if (status_RFID == MI_OK)
                        {
                            //checksum1 = Card_serial_number[0] ^ Card_serial_number[1] ^ Card_serial_number[2] ^ Card_serial_number[3];
                            //printf("The card's number is:");
                            //printf("%3d:%3d:%3d:%3d\r\n", (uchar)Card_serial_number[0],(uchar)Card_serial_number[1],(uchar)Card_serial_number[2],(uchar)Card_serial_number[3]);

                            Card_cap=MFRC522_SelectTag(Card_serial_number);
                            //printf("The card's cap is:%d\r\n",Card_cap);
                            IsLockCard = true;
                            //MFRC522_Halt();
                            //MFRC522_Init();
                            //osDelay(100);
                        }
                    }
                    else
                    {
                        memset(&Current_Credit,0,sizeof(Current_Credit_t));//没有检测到卡,清空这个东西
                        Current_card_type = 0;
                        IsLockCard = false;
                        No_valid_Card = false; //默认无卡为有效
                    }
                }
                //当选定卡的时候,锁定然后不停的读取,直到读取不成功的时候,说明卡片移除了,然后如果不进入刷卡程序段就锁定了当前卡片的信息
                if(IsLockCard == true && Current_card_type == 0)
                {
                    // Be Carefull !! ::
                    // write data on 0,1,2,4,5,6,8,... and Don't write data on 3,7,11,.... blocks .
                    // write Key on 3,7,11,... blocks.
                    // Read from and Wirte to block 1 and 2
                    //读取内部卡片类型
                    //Try to authenticate each block first with the A key.
                    process=MFRC522_Auth(PICC_AUTHENT1A,0,keyA ,Card_serial_number);
                    //process= MFRC522_Write(ii,senddata);
                    if (process == MI_OK)
                    {
                        process= MFRC522_Read(1, getdata);//获得卡片的类型
                        if (process == MI_OK)
                        {
                            //									printf("1 length : %d\r\n",getdata[1]);
                            //									for (ascii_i=1;ascii_i<=getdata[0];ascii_i++)
                            //									{
                            //										printf("%c", getdata[ascii_i]);
                            //									}
                            //									printf("%s\r\n", getdata);

                            if(Current_card_type == 0)
                            {
                                HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
                                osDelay(50);
                                HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
                                LCD_Delay_Time = 10 * 100;
                                //printf("LCD_Delay_Time2 %d\r\n",LCD_Delay_Time);
                            }

                            if(getdata[0] == '1'&&getdata[1] == '1'&&getdata[2] == '1'&&getdata[3] == '1'&&getdata[4] == '1'&&getdata[5] == '1')
                            {
                                //printf("ok1\r\n");
                                //M_Card_type = 1;
                                Current_card_type = 1;//正常卡
                            }
                            if(getdata[0] == '2'&&getdata[1] == '2'&&getdata[2] == '2'&&getdata[3] == '2'&&getdata[4] == '2'&&getdata[5] == '2')
                            {
                                //printf("ok2\r\n");
                                //M_Card_type = 2;
                                Current_card_type = 2;//开锁卡
                            }
                            if(getdata[0] == '3'&&getdata[1] == '3'&&getdata[2] == '3'&&getdata[3] == '3'&&getdata[4] == '3'&&getdata[5] == '3')
                            {
                                //printf("ok3\r\n");
                                //M_Card_type = 3;
                                Current_card_type = 3;//流量校准
                            }
                            if(getdata[0] == '4'&&getdata[1] == '4'&&getdata[2] == '4'&&getdata[3] == '4'&&getdata[4] == '4'&&getdata[5] == '4')
                            {
                                //printf("ok4\r\n");
                                //M_Card_type = 4;
                                Current_card_type = 4;//重置
                            }

                            //如果是正经卡,就读取卡片的信息
                            switch(Current_card_type)
                            {
																case 1:// && strcmp(REAL_DATA_PARAM.Electric_Meter.ElectricValveStatus, "0")==0
																		//读取卡片的编号
																		//Try to authenticate each block first with the A key.
																		process=MFRC522_Auth(PICC_AUTHENT1A,4,keyA ,Card_serial_number);//cardNO
																		if (process == MI_OK)
																		{
																				process= MFRC522_Read(4, getdata);
																				if (process == MI_OK)
																				{
																						//printf("block : %d  %d\r\n",getdata[0],getdata[1]);
																						memset(Use_serial_number,0,sizeof(Use_serial_number));
																						TmpCARDNoLength = getdata[1];
																						for (ascii_i=0; ascii_i<(getdata[1] > 14 ? 14 : getdata[1]); ascii_i++)
																						{
																								//printf("%d %c\r\n",ascii_i, getdata[ascii_i + 2]);
																								Use_serial_number[ascii_i] = getdata[ascii_i + 2];
																						}
																						if(TmpCARDNoLength > 14)
																						{
																								process= MFRC522_Read(5, getdata);
																								if (process == MI_OK)
																								{
																										for (ascii_i=0; ascii_i<((TmpCARDNoLength -14) > 16 ? 16 : (TmpCARDNoLength - 14)); ascii_i++)
																										{
																												Use_serial_number[14 + ascii_i] = getdata[ascii_i];
																										}
																								}
																						}
																						//printf("%s\r\n", Use_serial_number);
																						//strncpy(Current_Credit.CARD_ID,Use_serial_number,TmpCARDNoLength);
																				}
																				else
																				{
																						//IsLockCard = false;//读取卡片号码区失败
																						No_valid_Card = true;
																				}
																		}
																		else
																		{
																				//IsLockCard = false;//验证卡片号码区失败
																				No_valid_Card = true;
																		}
																		//读取客户号码
																		//Try to authenticate each block first with the A key.
																		process=MFRC522_Auth(PICC_AUTHENT1A,8,keyA ,Card_serial_number);//customer
																		if (process == MI_OK)
																		{
																				process= MFRC522_Read(8, getdata);
																				if (process == MI_OK)
																				{
																						//printf("block : %d  %d\r\n",getdata[0],getdata[1]);
																						memset(Use_CUSTOMER_number,0,sizeof(Use_CUSTOMER_number));
																						TmpCUSTOMERNoLength = getdata[1];
																						for (ascii_i=0; ascii_i<(getdata[1] > 14 ? 14 : getdata[1]); ascii_i++)
																						{
																								//printf("%c", getdata[ascii_i]);
																								Use_CUSTOMER_number[ascii_i] = getdata[ascii_i + 2];
																						}
																						if(TmpCUSTOMERNoLength > 14)
																						{
																								process= MFRC522_Read(9, getdata);
																								if (process == MI_OK)
																								{
																										for (ascii_i=0; ascii_i<((TmpCUSTOMERNoLength - 14) > 16 ? 16 : (TmpCUSTOMERNoLength - 14)); ascii_i++)
																										{
																												Use_CUSTOMER_number[14 + ascii_i] = getdata[ascii_i];
																										}
																								}
																						}
																						//printf("%s\r\n", Use_CUSTOMER_number);
																						int16_t TmpIndex = 0;
																						if(REAL_DATA_Credit.CardTotal > 0)//如果内存中有卡,先从内存中寻找
																						{
																								while(TmpIndex < REAL_DATA_Credit.CardTotal)
																								{
																										memset(&Tmp_Credit,0,sizeof(Current_Credit_t));
																										Current_Credit_READ(TmpIndex);//刷卡的时候挨个读取内存中的卡信息,并比对
																										if(stringCmp(Tmp_Credit.CUSTOMER_ID,Use_CUSTOMER_number,7) == PASSED
																														&& stringCmp(Tmp_Credit.CARD_ID,Use_serial_number,18) == PASSED)
																										{
																												//把搜索到的卡片信息设置为当前卡片的信息
																												//如果当前的卡片就是这个卡片,比较数值,按照小的走,按照小的走,也不好,万一有充值的怎么办呢
		//																					if(stringCmp(Current_Credit.CUSTOMER_ID,Use_CUSTOMER_number,7) == PASSED
		//																					&& stringCmp(Current_Credit.CARD_ID,Use_serial_number,18) == PASSED)
		//																				  {
		//																						if(Tmp_Credit.CurrCredit != Current_Credit.CurrCredit)
		//																						{
																												Current_Credit.CurrCredit = Tmp_Credit.CurrCredit;//卡内余额
		//																						}
		//																						printf("CurrCredit:%f\r\n", Current_Credit.CurrCredit);
		//																						printf("Tmp_Credit:%f\r\n", Tmp_Credit.CurrCredit);
		//																				  }
																												Current_Credit.CUSTOMERNoLength = 7;//客户长度
																												strncpy(Current_Credit.CUSTOMER_ID,Use_CUSTOMER_number,32);//客户号
																												Current_Credit.CARDNoLength = 18;//卡片长度
																												strncpy(Current_Credit.CARD_ID,Use_serial_number,32);//卡片号

																												Current_Credit.CurrIndex = Tmp_Credit.CurrIndex;//卡片序号

																												REAL_Current_Credit_Write();
																												//后面还需要赋值
																												//																	if(Current_Credit.CurrCredit <=0)
																												//																	{
																												//																		//TODO 获取余额
																												//																	}
																												break;
																										}
																										TmpIndex++;
																								}
																						}
																						if(TmpIndex == REAL_DATA_Credit.CardTotal)//刷卡中找不到就新建
																						{
																								memset(&Current_Credit,0,sizeof(Current_Credit_t));
																								Current_Credit.CurrIndex = TmpIndex;//赋值最大号
																								Current_Credit.CUSTOMERNoLength = 7;//TmpCUSTOMERNoLength
																								strncpy(Current_Credit.CUSTOMER_ID,Use_CUSTOMER_number,32);
																								Current_Credit.CARDNoLength = 18;//TmpCARDNoLength
																								strncpy(Current_Credit.CARD_ID,Use_serial_number,32);

																								if(strcmp(Use_serial_number, "111111111111111111")==0)
																								{
																									char *str1 = strstr(CONFIG_Meter.MeterNo,"KE");
																									if(str1!=NULL)
																									{
																										Current_Credit.CurrCredit = 23.3;//1000g
																									}
																									else
																									{
																										Current_Credit.CurrCredit = 1000;//1000g
																									}
																								}
																								else if(strcmp(Use_serial_number, "222222222222222222")==0)
																								{
																									char *str1 = strstr(CONFIG_Meter.MeterNo,"KE");
																									if(str1!=NULL)
																									{
																										Current_Credit.CurrCredit = 23.3;//1000g
																									}
																									else
																									{
																										Current_Credit.CurrCredit = 4000;//100g
																									}
																								}
																								else if(strcmp(Use_serial_number, "333333333333333333")==0)
																								{
																									char *str1 = strstr(CONFIG_Meter.MeterNo,"KE");
																									if(str1!=NULL)
																									{
																										Current_Credit.CurrCredit = 23.3;//1000g
																									}
																									else
																									{
																										Current_Credit.CurrCredit = 2000;//100g
																									}
																								}
																								else
																								{
																										Current_Credit.CurrCredit = 0;
																								}

																								REAL_DATA_Credit.CardTotal++;
																								LL_VCC(1);
																								REAL_DATA_Credit_Write();//保存索引 刷卡中新建卡
																								Current_Credit_Write(Current_Credit);//刷卡不存在新建卡的时候
																								//TODO 获取余额 不用获取余额,联网直接接收
																						}
																				}
																				else
																				{
																						//IsLockCard = false;//读取失败
																						No_valid_Card = true;
																				}
																		}
																		else
																		{
																				//IsLockCard = false;//验证包租婆号码区失败
																				No_valid_Card = true;
																		}
																		break;
																		//开锁并停止刷卡
																case 2://开锁
//																		if(strcmp(REAL_DATA_PARAM.Electric_Meter.LidElectricLock, "1")==0)
//																		{
//																		 strcpy(REAL_DATA_PARAM.Electric_Meter.LidElectricLock,"0");
//																		 glockStatus = motor_open;
//																		}
																    //读取卡片的日期
																		//Try to authenticate each block first with the A key.
																		process=MFRC522_Auth(PICC_AUTHENT1A,20,keyA ,Card_serial_number);//cardNO
																		if (process == MI_OK)
																		{
																				process= MFRC522_Read(20, getdata);
																				if (process == MI_OK)
																				{
																						//printf("block : %d  %d\r\n",getdata[0],getdata[1]);
																					  //printf("getdata:%s\r\n",getdata);
																						memset(Use_Date,0,sizeof(Use_Date));
																						TmpCardDateLength = getdata[1];
																					  //printf("TmpCardDateLength:%d\r\n",TmpCardDateLength);
																						for (ascii_i=0; ascii_i<(getdata[1] > 14 ? 14 : getdata[1]); ascii_i++)
																						{
																								//printf("%d %c\r\n",ascii_i, getdata[ascii_i + 2]);
																								Use_Date[ascii_i] = getdata[ascii_i + 2];
																						}
																						//printf("Use_Date:%s\r\n",Use_Date);
																						if(TmpCardDateLength > 14)
																						{
																								process= MFRC522_Read(21, getdata);
																								if (process == MI_OK)
																								{
																										for (ascii_i=0; ascii_i<((TmpCardDateLength -14) > 16 ? 16 : (TmpCardDateLength - 14)); ascii_i++)
																										{
																												Use_Date[14 + ascii_i] = getdata[ascii_i];
																										}
																								}
																						}
																						
																						sscanf(Use_Date,"%ld",&Card_lock_date);
																						//printf("%s\r\n", Use_serial_number);
																						//strncpy(Current_Credit.CARD_ID,Use_serial_number,TmpCARDNoLength);
																				}
																				else
																				{
																						//IsLockCard = false;//读取卡片号码区失败
																						No_valid_Card = true;
																				}
																		}
																		else
																		{
																				//IsLockCard = false;//验证卡片号码区失败
																				No_valid_Card = true;
																		}
																		GetRTC(&time,&date);
																		
																		//printf("date:%ld\r\n",date);
																		//printf("Card_lock_date:%ld\r\n",Card_lock_date);
																		
																		if(date <= Card_lock_date)
																		{
																			if(strcmp(REAL_DATA_PARAM.TankLockStatus, "1")==0)
																			{
																					strcpy(REAL_DATA_PARAM.TankLockStatus,"0");
																					gassembleStatus = motor_open;
																					OpenLockTime = 10;
																					glockSet = motor_open;
																					//printf("open\r\n");
																					IsTestRFID = false;
																					osDelay(100);
																					//RFID_POWER(0);
																					IsInitRFID = false;
																					IsNeedIllegal_lid_opening = false;
																					//TODO 保存开锁日志 ,并发送开锁报文

																					LogWrite(OpenAssemble,"OpenAssemble",NULL);
																			}
																		}
																		break;
																		//校准逻辑
																case 3://校准流量
																		//读取转换系数
																		//Try to authenticate each block first with the A key.
																		process=MFRC522_Auth(PICC_AUTHENT1A,12,keyA ,Card_serial_number);//convert
																		if (process == MI_OK)
																		{
																				process= MFRC522_Read(12, getdata);
																				if (process == MI_OK)
																				{
																						if(getdata[0] == 0x88)
																						{
																								//printf("Use_Convert : %d  %d\r\n",getdata[0],getdata[1]);
																								memset(Use_Convert,0,sizeof(Use_Convert));
																								TmpConvertLength = getdata[1];
																								for (ascii_i=0; ascii_i<(getdata[1] > 14 ? 14 : getdata[1]); ascii_i++)
																								{
																										//printf("%d %c\r\n",ascii_i, getdata[ascii_i + 2]);
																										Use_Convert[ascii_i] = getdata[ascii_i + 2];
																								}
																								if(TmpConvertLength > 14)
																								{
																										process= MFRC522_Read(13, getdata);
																										if (process == MI_OK)
																										{
																												for (ascii_i=0; ascii_i<((TmpConvertLength -14) > 16 ? 16 : (TmpConvertLength - 14)); ascii_i++)
																												{
																														Use_Convert[14 + ascii_i] = getdata[ascii_i];
																												}
																										}
																								}
																								sscanf(Use_Convert,"%f",&Tmp_convert);
																						}
																						else
																						{
																								Tmp_convert = 0;
																						}

																						//printf("Use_Convert %s\r\n" ,Use_Convert);
																						//printf("Tmp_convert %f\r\n" ,Tmp_convert);

																						if(Tmp_convert == 0)//如果转换率等于零触发校准逻辑
																						{
																								IsTestFlow = true;
																								IsTestRFID = false;
																								osDelay(100);
																								//RFID_POWER(0);
																								IsInitRFID = false;

																						}
																						else//否则给转换系数赋值
																						{
																								REAL_DATA_Credit.Magnification = Tmp_convert;
																								LL_VCC(1);
																								REAL_DATA_Credit_Write();	//保存转换系数
																								//IsLockCard = false;
																								Current_card_type = 0;
																								osDelay(500);
																						}
																				}
																				else
																				{
																						//IsLockCard = false;//读取失败
																						No_valid_Card = true;
																				}
																		}
																		else
																		{
																				//IsLockCard = false;//验证失败
																				No_valid_Card = true;
																		}
																		break;
																case 4://回复出厂
																		RESETALL(5,NULL);
																		//SPI_FLASH_BulkErase();
																		//printf("SPI_FLASH_BulkErase\r\n");
																		//IsLockCard = false;
																		Current_card_type = 0;
																		osDelay(1000);
																		break;
                            }
                        }
                        else
                        {
                            IsLockCard = false;//获取卡片类型失败
                            No_valid_Card = true;
                        }
                    }
                    else
                    {
                        IsLockCard = false;//验证卡片类型失败
                        No_valid_Card = true;
                    }
                }
            }
            else
            {
                RFID_POWER(0);
                RFID_Reset(0);
            }
        }
//		else
//		{
//			  LL_VCC(1);   //存储器上电并弄成可以写的状态
//				REAL_Current_Credit_Write();//如果没有刷卡一直存储当前信息
//		}

        //定时唤醒采集
        //BATAD_POWER(1);
        //REAL_DATA_PARAM.BatVoltage = ADC_BAT();

        //没有开阀的情况下熄灭屏幕后,如果这个时候仪表上有卡,清除卡片信息,保护性措施,
        if(Current_card_type == 1 && strcmp(REAL_DATA_PARAM.ElectricValveStatus, "0") == 0 && sStep == 0 && Lcd_Status == false)
        {
            Current_card_type = 0;
            //printf("ElectricValveStatus %s\r\n",REAL_DATA_PARAM.ElectricValveStatus);
        }

//        if(Current_card_type == 0 && IsReadVoltage == true)
//        {
//            if(haveClickButton == true)
//            {
//                haveClickButton = false;
//                IsSaveREAL_DATA_PARAM = true;
//                gmotorStatus = motor_open;
//            }
//        }
        //开阀
        //当关闭刷卡测试的时候,卡的类型就锁定了,只需要执行对应的卡片操作就好
        //当威盛没有电压的时间超过3分钟,自动关阀,或者电压过低,没有钱,没有气 关闭阀门
        if(Current_card_type == 1)
        {
            if(haveClickButton == true)
            {
                haveClickButton = false;
                //在各种条件满足的情况下,开阀
                if(Current_Credit.CurrCredit > 0
                        && REAL_DATA_PARAM.TankQuality > 0.5
                        && REAL_DATA_PARAM.BatVoltage >= 3.6
                        && strcmp(REAL_DATA_PARAM.ElectricValveStatus, "0")==0
                        //&& Key2_input == 1 //为了兼容100L与10L的PCB,删除这个条件
								        //&& Opening_input == 0
                        && NEEDLE_input == 0
                        && gmotorStatus == motor_null
                        && sStep != 0 //关屏的时候不开阀
                        &&Lcd_Status == true
                  )
                {
                    zeroFlowTime = 0;
                    strcpy(REAL_DATA_PARAM.ElectricValveStatus,"1");
                    IsSaveREAL_DATA_PARAM = true;
                    gmotorStatus = motor_open;

                    LogWrite(OpenValve,"ClickButton",NULL);

                    IsTestFlow = true;
                    GPRS_PWR(1);

                    IsTestRFID = false;

                    IsInitRFID = false;

                    memset(&CookingSessionReport,0,sizeof(CookingSessionReport_t));
                    strncpy(CookingSessionReport.meterNumer,CONFIG_Meter.MeterNo,10);
                    strncpy(CookingSessionReport.CUSTOMER_ID,Current_Credit.CUSTOMER_ID,7);
                    strncpy(CookingSessionReport.CARD_ID,Current_Credit.CARD_ID,18);
                    strncpy(CookingSessionReport.SESSION_END_TYPE,"XX",2);
                    sprintf(CookingSessionReport.START_CUMULATIVE_VOLUME,"%.2f",REAL_DATA_Credit.CumulationGasL);
                    StartCookingSeconds = Read_Time();
                    sprintf(CookingSessionReport.SESSION_START_TIME,"%04d%02d%02d%02d%02d%02d",StartCookingSeconds.tm_year,StartCookingSeconds.tm_month,StartCookingSeconds.tm_day,StartCookingSeconds.tm_hour,StartCookingSeconds.tm_minute,StartCookingSeconds.tm_seconds);
                    sprintf(CookingSessionReport.CREDIT_SESSION_START,"%.2f",Current_Credit.CurrCredit);
                    sprintf(CookingSessionReport.UnitPrice, "%.2f", REAL_DATA_Credit.UnitPrice);
                    sprintf(CookingSessionReport.GasDensity, "%.3f", REAL_DATA_Credit.LPGDensity);

                    StartCredit = Current_Credit.CurrCredit;

                    strncpy(REAL_DATA_PARAM.CUSTOMER_ID,Current_Credit.CUSTOMER_ID,7);
                }
                else //开阀的时候不操作屏幕
                {
                    gkeyValue = true;
                }
            }

            //关阀
            if((
								(zeroFlowTime > 10 && Get_SubSeconds(StartCookingSeconds) < 120)//当开始做饭小于5分钟的时候判断零流量大于10自动关阀
								|| (TmpRealTimeFlow < 0.3 && Get_SubSeconds(StartCookingSeconds) >= 120)//当开始做饭大于等于5分钟的时候判断零流量大于1自动关阀
								//|| REAL_DATA_PARAM.BatVoltage < 3.6 做饭过程中不判断电压,因为电压变化没有这么快,万一有可能是其他动作把电压瞬间拉低,也可能是程序错误
								|| Current_Credit.CurrCredit <= 0
								|| REAL_DATA_PARAM.TankQuality <= 0.5
								|| ((Opening_input == 1 
						         //|| Key2_input == 0
						         )&& IsNeedIllegal_lid_opening == true)
                     || NEEDLE_input == 1)
                    && strcmp(REAL_DATA_PARAM.ElectricValveStatus, "1")==0
                    //&& IsReadVoltage == false
							)//
            {
//					printf("Cook-----------------------------------End\r\n");
//				  printf("zeroFlowTime %d\r\n",zeroFlowTime);
//				  printf("SESSION_START_TIME %s\r\n",CookingSessionReport.SESSION_START_TIME);
//				  printf("Get_SubSeconds %d\r\n",Get_SubSeconds(StartCookingSeconds));
//          printf("IsStartCooking5 : %d\r\n",IsStartCooking5);

                TmpRealTimeFlow = 0;
                IsStartCooking5 = false;
                Current_card_type = 0;//立即置零,否则这个时候点亮显示屏,打不开刷卡
                //if(gmotorStatus == motor_null)
                {
                    gmotorStatus = motor_close;
                    strcpy(REAL_DATA_PARAM.ElectricValveStatus,"0");//0:Valve closed, 1: Valve open

                    osDelay(100);
                    //if(sStep != 0)
                    {
                        RFID_Reset(1);
                        RFID_POWER(1);
                        IsTestRFID = true;
                    }
                    osDelay(100);

                    if(Lcd_Status == false)
                    {
                        gkeyValue = true;
                    }
                    IsInitRFID = false;
                    IsLockCard = false;

                    if(StartCredit > Current_Credit.CurrCredit)
                    {
                        if(NEEDLE_input == 1)
                        {
                          strncpy(CookingSessionReport.SESSION_END_TYPE,"01",2);
													LogWrite(CloseValve,"UserCloseNeedle",NULL);
                        }
                        else if(zeroFlowTime > 10 || (TmpRealTimeFlow < 0.3 && Get_SubSeconds(StartCookingSeconds) >= 120))
                        {
                          strncpy(CookingSessionReport.SESSION_END_TYPE,"02",2);
													LogWrite(CloseValve,"Noflow>3mins",NULL);
                        }
                        else if(REAL_DATA_PARAM.TankQuality <= 0.5)
                        {
                          strncpy(CookingSessionReport.SESSION_END_TYPE,"03",2);
													LogWrite(CloseValve,"LowGas",NULL);
                        }
                        else if(REAL_DATA_PARAM.BatVoltage < 3.6)
                        {
                          strncpy(CookingSessionReport.SESSION_END_TYPE,"04",2);
													LogWrite(CloseValve,"Low battery",NULL);
                        }
                        else if(Current_Credit.CurrCredit <= 0)
                        {
                          strncpy(CookingSessionReport.SESSION_END_TYPE,"05",2);
													LogWrite(CloseValve,"No credit",NULL);
                        }
												else if(Opening_input == 1 && IsNeedIllegal_lid_opening == true)
                        {
                          strncpy(CookingSessionReport.SESSION_END_TYPE,"06",2);
													LogWrite(CloseValve,"Open lid",NULL);
                        }
                        else
                        {
                          strncpy(CookingSessionReport.SESSION_END_TYPE,"XX",2);
													LogWrite(CloseValve,"Unknown",NULL);
                        }

                        //LogWrite(CloseValve,CookingSessionReport.SESSION_END_TYPE,NULL);

                        sprintf(CookingSessionReport.END_CUMULATIVE_VOLUME,"%.2f",REAL_DATA_Credit.CumulationGasL);
                        GetRTC(&time,&date);
                        sprintf(CookingSessionReport.SESSION_END_TIME,"%08ld%06ld",date,time);
                        sprintf(CookingSessionReport.CREDIT_SESSION_END,"%.2f",Current_Credit.CurrCredit);
                        sprintf(CookingSessionReport.UnitPrice, "%.2f", REAL_DATA_Credit.UnitPrice);
                        sprintf(CookingSessionReport.GasDensity, "%.3f", REAL_DATA_Credit.LPGDensity);
												sprintf(CookingSessionReport.GAS_REMAINING, "%.3f", REAL_DATA_PARAM.TankQuality);

                        LL_VCC(1);
                        Cooking_Session_Write();//每次做完饭如果发现有用气量存储cooking

                        LogWrite(Cooking,"CookingEnd",NULL);

                        memset(&CookingSessionReport,0,sizeof(CookingSessionReport_t));

                        //							Cooking_Session_READ(REAL_DATA_Credit.CookingSessionEnd);
                        //							printf("CookingSessionReport1 %s\r\n",CookingSessionReport.CARD_ID);
                        //							printf("CookingSessionReport1 %s\r\n",CookingSessionReport.CUSTOMER_ID);
                        //							printf("CookingSessionReport1 %s\r\n",CookingSessionReport.CREDIT_SESSION_START);
                        //							printf("CookingSessionReport1 %s\r\n",CookingSessionReport.CREDIT_SESSION_END);

                        //							printf("CookingSessionReport1 %s\r\n",CookingSessionReport.START_CUMULATIVE_VOLUME);
                        //							printf("CookingSessionReport1 %s\r\n",CookingSessionReport.END_CUMULATIVE_VOLUME);
                        //
                        //							printf("CookingSessionReport1 %s\r\n",CookingSessionReport.SESSION_START_TIME);
                        //							printf("CookingSessionReport1 %s\r\n",CookingSessionReport.SESSION_END_TIME);
                        //							printf("CookingSessionReport1 %s\r\n",CookingSessionReport.SESSION_END_TYPE);
                        //
                        //							printf("CookingSessionSendNumber %d\r\n",REAL_DATA_Credit.CookingSessionSendNumber);
                        //							printf("CookingSessionEnd %d\r\n",REAL_DATA_Credit.CookingSessionEnd);

                        REAL_DATA_Credit.CookingSessionEnd++;
                    }

                    LL_VCC(1);
                    REAL_DATA_PARAM_Write();//保存阀门的实时信息
                    REAL_DATA_Credit_Write();//保存总体仪表的充值信息 关阀时

                    //else
                    {
                        //保存当前仪表的充值信息,有RFID的时候其实不用保存,事实证明在大的存储器中存数据存在莫名丢失显现
                        //为了避免这种情况发生,这个值改成每次存储,并且每次做完饭后重启读取此数据,并且在下次读取的时候做比对,以小值为准
                        REAL_Current_Credit_Write();
                    }

                    //if(CONFIG_Meter.IsHaveRFID)
                    {
                        Current_Credit_Write(Current_Credit);//使用中关闭阀门的时候,每次写完重新读取,应该一致,如果不一致,人为重启,重启的时候再写入
                        //memset(&Tmp_Credit,0,sizeof(Current_Credit_t));
                        //Current_Credit_READ(Current_Credit.CurrIndex);
                    }
										
										//printf("Current_Credit:%f\r\n",Current_Credit.CurrCredit);
										//printf("Tmp_Credit:%f\r\n",Tmp_Credit.CurrCredit);
										
                    //如果刚刚写进去的参数不等于要写进的参数,说明写入失败,这个时候重启,比较后重新写入
//                    if(Current_Credit.CurrCredit != Tmp_Credit.CurrCredit)
//                    {
//                        CONFIG_Meter.IsNormalReset = true;
//											  
//                        CONFIG_Meter_Write();
//                        //重新启动
//                        LogWrite(RestartUnNorm,"bigFlashError",NULL);
//											
////                        HAL_NVIC_SystemReset();
//                    }
//                    else
                    {
                        HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
                        osDelay(500);
                        HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);

                        Current_card_type = 0;//这个位置实际上是多余的

                        if(
													strcmp(Use_serial_number, "111111111111111111")!=0 &&
                          //strcmp(Use_serial_number, "222222222222222222")!=0 &&
                          strcmp(Use_serial_number, "333333333333333333")!=0
												)
                        {
                            if(//REAL_DATA_PARAM.BatVoltage <= CONFIG_Meter.LowBattery || 
                                    Current_Credit.CurrCredit <= CONFIG_Meter.LowCredit || 
                                     REAL_DATA_PARAM.TankQuality <= CONFIG_Meter.LowGasVolume)
                            {
                                IsNeedWarning = true;//关闭时有报警
																connectStep = 0;
																GSM_ON_FLAG = 1;
																printf("Sim80x_SetPower(true)18\r\n");
																Sim80x_SetPower(true);
                            }
                        }
                    }
                }
                zeroFlowTime = 60;
            }
        }

        //处理关锁逻辑,首先锁定的卡片是开锁卡或者无卡,
        //保护性关锁,转动摇把关锁
        if(Current_card_type == 2 || Current_card_type == 0)//开锁
        {
            if(haveClickButton == true)
            {
                //LogWrite(ClickButton,"ClickButton",NULL);
                haveClickButton = false;
                gkeyValue = true;
            }

            //如果针阀是开,拆卸锁是开,电机没有动作,防止电机还没有动作完成,立即动作了摇把,电机动作异常
            if(NEEDLE_input == 0 && strcmp(REAL_DATA_PARAM.TankLockStatus, "0")==0 && gassembleStatus == motor_null)
            {
                strcpy(REAL_DATA_PARAM.TankLockStatus,"1");
                gassembleStatus = motor_close;
                //printf("motor_close\r\n");
                RFID_Reset(1);
                RFID_POWER(1);
                osDelay(100);
                IsTestRFID = true;
                IsInitRFID = false;
                IsLockCard = false;
                LL_VCC(1);
                REAL_DATA_PARAM_Write();
                Current_card_type = 0;

                LogWrite(CloseAssemble,"UseNeedle",NULL);
            }
        }

        //如果当前是校准的卡的处理逻辑
        if(Current_card_type == 3)
        {
            //关针阀,而且校准是正在的,关闭校准逻辑
            if(NEEDLE_input == 1 && IsStartCALIBRATION == true)
            {
                IsStartCALIBRATION = false;
                //printf("zeroFlowTime:%d\r\n",zeroFlowTime);
                zeroFlowTime = 60;
                RFID_Reset(1);
                RFID_POWER(1);
                osDelay(100);
                IsTestRFID = true;
                IsInitRFID = false;
                IsLockCard = false;
                Current_card_type = 0;

                if(strcmp(REAL_DATA_PARAM.ElectricValveStatus, "1")==0)
                {
                    if(gmotorStatus == motor_null)
                    {
                        //printf("gmotorStatus:%d\r\n",gmotorStatus);
                        gmotorStatus = motor_close;
                        strcpy(REAL_DATA_PARAM.ElectricValveStatus,"0");//0:Valve closed, 1: Valve open
                        LL_VCC(1);
                        REAL_DATA_PARAM_Write();
                    }
                }
            }

            //开针阀开始校准
            if(NEEDLE_input == 0 && IsStartCALIBRATION == false)
            {
                IsStartCALIBRATION = true;
            }

            if(haveClickButton == true)
            {
                haveClickButton = false;
                if(NEEDLE_input == 0)//针阀开的时候
                {
                    if(strcmp(REAL_DATA_PARAM.ElectricValveStatus, "0")==0)
                    {
                        strcpy(REAL_DATA_PARAM.ElectricValveStatus,"1");
                        IsSaveREAL_DATA_PARAM = true;
                        gmotorStatus = motor_open;
                    }
                    else
                    {
                        gkeyValue = true;
                    }
                }
                else //针阀关的时候可以操作屏幕,查看电压
                {
                    gkeyValue = true;
                }
            }

            if(haveLongButton == true)
            {
                haveLongButton = false;
                if(strcmp(REAL_DATA_PARAM.ElectricValveStatus, "1")==0)
                {
                    if(vert2 != 0 && volt1 != 0 && vert2 > volt1)
                    {
                        REAL_DATA_CALIBRATION.slope = (y2 - y1)/(vert2 - volt1);
                        REAL_DATA_CALIBRATION.zero =y1 - REAL_DATA_CALIBRATION.slope * volt1;
                        IsSaveCALIBRATION = true;
                    }
                    printf("vert2 : %.3f\r\n",vert2);
                    printf("y2 : %.3f\r\n",y2);
                    printf("volt1 : %.3f\r\n",volt1);
                    printf("y1 : %.3f\r\n",y1);
                    printf("slope : %.3f\r\n",REAL_DATA_CALIBRATION.slope);
                    printf("zero : %.3f\r\n",REAL_DATA_CALIBRATION.zero);
                }
                REAL_DATA_Credit.Magnification = 1;
                REAL_DATA_Credit.CumulationGasL = 0;
                IsSaveREAL_DATA_Credit = true;
            }
        }

        //查询卡片余额
        if(commandType == 2)
        {
            int16_t TmpIndex = 0;

            if(REAL_DATA_Credit.CardTotal > 0)
            {
                while(TmpIndex < REAL_DATA_Credit.CardTotal)
                {
                    memset(&Tmp_Credit,0,sizeof(Current_Credit_t));
                    Current_Credit_READ(TmpIndex); //服务器查询卡片余额的时候从内存中寻找
                    //printf("CUSTOMER_ID : %s\r\n",Tmp_Credit.CUSTOMER_ID);
                    //printf("CARD_ID : %s\r\n",Tmp_Credit.CARD_ID);
                    if(stringCmp(Tmp_Credit.CUSTOMER_ID,CustomerCredit.CUSTOMER_ID,7) == PASSED
                            && stringCmp(Tmp_Credit.CARD_ID,CustomerCredit.CARD_ID,18) == PASSED)
                    {
                        sprintf(CustomerCredit.CREDIT_REMAINING,"%.1f",Tmp_Credit.CurrCredit);//把查到的余额赋值给解析出来的CustomerCredit
                        IsNeedCustomerCreditResponse = true;
                        break;
                    }
                    TmpIndex++;
                }
            }
            commandType = 0;
        }

        //当执行开锁指令后,却没有下一步指示,过一段时间后自动关锁
        //但是当打开之后下面的就不执行了
        //带自锁的需要关闭操作
        //与上面的摇把关锁互补,这里是判断时间
        if(OpenLockTime == 0 && glockSet == motor_open)
        {
            glockSet = motor_null;
            gassembleStatus = motor_close;
            strcpy(REAL_DATA_PARAM.TankLockStatus,"1");//0: UnLock, 1: Lock
            LL_VCC(1);
            REAL_DATA_PARAM_Write();

            if(sStep == 0)
            {
                gkeyValue = true;
            }

            if(Current_card_type == 2)
            {
                RFID_Reset(1);
                RFID_POWER(1);//自动关闭钢瓶锁的时候,需要重新测试RFID
                osDelay(100);
                IsTestRFID = true;
                IsInitRFID = false;
                IsLockCard = false;
            }

            Current_card_type = 0;

            LogWrite(CloseAssemble,"timeOut",NULL);
        }

        //存储盖子的状态
        if(LidStatus != (Opening_input == 1 ? true : false))
        {
            LidStatus = (Opening_input == 1 ? true : false);
            strcpy(REAL_DATA_PARAM.LidSensorStatus,(Opening_input == 1 ? "1" : "0"));//0:Lid closed, 1: Lid open
            LL_VCC(1);
            REAL_DATA_PARAM_Write();
        }

        //针阀的状态
        if(NEEDLEStatus != (NEEDLE_input == GPIO_PIN_RESET ? true : false))
        {
            NEEDLEStatus = (NEEDLE_input == GPIO_PIN_RESET ? true : false);
            strcpy(REAL_DATA_PARAM.NEEDLESensorStatus,(NEEDLE_input == GPIO_PIN_RESET ? "1" : "0"));//0:needle up(close), 1: needle down(open)
            LL_VCC(1);
            REAL_DATA_PARAM_Write();

            if(NEEDLEStatus == true)
            {
                LogWrite(OpenNeedle,"OpenNeedle",NULL);
            }
            else
            {
                LogWrite(CloseNeedle,"CloseNeedle",NULL);
            }
        }

        //按钮长按的操作
        if(haveLongButton == true)
        {
            haveLongButton = false;
            if(sStep == 0)
            {
                gkeyValue = true;
            }
            if(Current_card_type == 0 || (Current_card_type == 1 && strcmp(REAL_DATA_PARAM.ElectricValveStatus, "0")==0))
            {
                if(GSM_ON_FLAG == 0)
                {
                    //printf("key %d \r\n",tkeyValue);
                    //printf("CookingSessionSendNumber %d \r\n",REAL_DATA_Credit.CookingSessionSendNumber);
                    //printf("CookingSessionEnd %d \r\n",REAL_DATA_Credit.CookingSessionEnd);
                    if(REAL_DATA_Credit.CookingSessionSendNumber < REAL_DATA_Credit.CookingSessionEnd)
                    {
                        IsNeedSendCook = true;
                        //printf("OK %d \r\n",tkeyValue);
                    }

										IsNeedTimeing = true;
                    connectStep = 0;
                    GSM_ON_FLAG = 1;
                    printf("Sim80x_SetPower(true)2\r\n");
                    Sim80x_SetPower(true);
                }
                else
                {
                    IsNeedTimeing = false;
                    IsSendHalfTime = false;
                    //TimeRetryNumber = 0;

                    GSM_ON_FLAG = 0;
                    Sim80x_GPRSClose(12);
                }
            }
        }

        //卡片不是1,或者卡片是1但是阀门是关闭的|| (Current_card_type == 1 && strcmp(REAL_DATA_PARAM.ElectricValveStatus, "0")==0)
        //只有没卡的时候,才触发上网逻辑
        if(Current_card_type == 0)// && IsReadVoltage == false
        {
            //保护性措施,防止阀门不关闭,理论上不会执行到,但如果真的没有关闭
            //需要首先尝试多次关闭,如果流量还是偏大,需要报警
            if(strcmp(REAL_DATA_PARAM.ElectricValveStatus, "1")==0 )//|| ADTestVFlow > 1)
            {
                //if(gmotorStatus == motor_null)
                {
                    gmotorStatus = motor_close;
                    strcpy(REAL_DATA_PARAM.ElectricValveStatus,"0");//0:Valve closed, 1: Valve open

                    LL_VCC(1);
                    REAL_DATA_PARAM_Write();//保存阀门的实时信息
                }

                IsTestRFID = true;
            }

            run_rtc = Read_Time();
            //printf("power down1 %d:%d:%d\r\n",run_rtc.tm_hour,run_rtc.tm_minute,run_rtc.tm_seconds);
            //每天定时重启 02:22:00
            if(run_rtc.tm_hour == 2 && run_rtc.tm_minute == 22 && run_rtc.tm_seconds == 0 && Lcd_Status == false && IsNeedTimeRestart == true && IsNeedSendCook == false)
            {
                CONFIG_Meter.IsNormalReset = true;
                CONFIG_Meter_Write();

                //重新启动
                LogWrite(RestartNormal,"TimedRestart",NULL);
							  IntoLowPower();
//                HAL_NVIC_SystemReset();
            }

            //如果没有使用,屏幕是灭的状态,定时伪重启,前提是系统没有死机,没有使用看门狗的时候,这个时候不亮屏幕重启系统
            if((run_rtc.tm_minute == 55) && run_rtc.tm_seconds == 0 && Lcd_Status == false && IsNeedTimeRestart == true && CONFIG_Meter.NotHaveDog == true && IsNeedSendCook == false)
            {
                CONFIG_Meter.IsNormalReset = true;
                CONFIG_Meter_Write();
                //printf("power down2 %d:%d:%d\r\n",run_rtc.tm_hour,run_rtc.tm_minute,run_rtc.tm_seconds);
                //重新启动
                LogWrite(RestartNormal,"OneHourRestart",NULL);
							  IntoLowPower();
//                HAL_NVIC_SystemReset();
            }

            //一小时查询一次电压
            if(run_rtc.tm_minute == 23)//run_rtc.tm_minute == 0
            {
                //前五秒采集
                if(run_rtc.tm_seconds < 10)
                {
                    BATAD_POWER(1);
                    IsTestPower = true;
                    IsTestEnd = false;
                    //printf("BATAD_POWER(1)\r\n");
                }
                else if(run_rtc.tm_seconds >= 10 && run_rtc.tm_seconds < 20)//10到20秒开始发送
                {
                    //printf("BATAD_POWER(2)\r\n");
                    if(REAL_DATA_PARAM.BatVoltage <= CONFIG_Meter.LowBattery && IsLowPowerWarned == false)//如果电压过低而且没有过低电压报警
                    {
                        IsLowPowerWarned = true;
                        IsNeedWarning = true;//电压报警
                        if(GSM_ON_FLAG == 0)
                        {
                            connectStep = 0;
                            GSM_ON_FLAG = 1;
                            printf("Sim80x_SetPower(true)17\r\n");
                            Sim80x_SetPower(true);
                        }
                    }
                    else
                    {
                        BATAD_POWER(0);
                        IsTestPower = false;
                    }
                }
                else//大于十秒的时候
                {
                    //printf("BATAD_POWER(3)\r\n");
                    if(sStep == 0)//如果屏幕是关闭的,关闭电池测量
                    {
                        BATAD_POWER(0);
                        IsTestPower = false;
                    }
                }
            }

//				if(REAL_DATA_PARAM.BatVoltage <= CONFIG_Meter.LowBattery
//								|| Current_Credit.CurrCredit <= CONFIG_Meter.LowCredit
//								|| REAL_DATA_PARAM.TankQuality <= CONFIG_Meter.LowGasVolume)
//				{
//					IsNeedWarning = true;
//				}

            //定时报
//            if(IsNeedTimeing == true)//如果判断有定时报
//            {
//                if(TimeRetryNumber < 1)//重发次数小于三次,第一遍不算,最多一共发送四次
//                {
//                    if(GSM_ON_FLAG == 0)//如果没有上电,就上电开始发送
//                    {
//                        TimeRetryNumber++;

//                        connectStep = 0;
//                        GSM_ON_FLAG = 1;
//                        //printf("Sim80x_SetPower(true)16\r\n");
//                        Sim80x_SetPower(true);
//                    }
//                }
//                if(TimeRetryNumber >= 1)//如果再次启动大于等于三次,设置重发无效
//                {
//                    IsNeedTimeing = false;
//                    TimeRetryNumber = 0;
//                    //AL_IWDG_Refresh(&hiwdg);
//                    //Sim80x_GPRSClose(13);
//                }
//            }
//            else //如果运行的时候,一下判断是否需要发送定时报
            {
//							if((run_rtc.tm_hour + 24 - CONFIG_Meter.UpDuty / 60) % 24 == (CONFIG_Meter.StartDuty / 60) &&
//								(run_rtc.tm_minute + 60 - CONFIG_Meter.UpDuty % 60) % 60 == (CONFIG_Meter.StartDuty % 60) &&
//							   run_rtc.tm_seconds < 2)
							
							if(((run_rtc.tm_hour * 60 +	run_rtc.tm_minute >= CONFIG_Meter.StartDuty) ? 	
								((run_rtc.tm_hour * 60 +	run_rtc.tm_minute - CONFIG_Meter.StartDuty) % CONFIG_Meter.UpDuty == 0)	:
                ((run_rtc.tm_hour * 60 +	run_rtc.tm_minute + 1440 - CONFIG_Meter.StartDuty) % CONFIG_Meter.UpDuty == 0)) &&
							   run_rtc.tm_seconds < 2
							)
							{
									if(REAL_DATA_Credit.CookingSessionSendNumber < REAL_DATA_Credit.CookingSessionEnd)
									{
											IsNeedSendCook = true;
									}
									IsNeedTimeing = true;
									if(GSM_ON_FLAG == 0)
									{
											connectStep = 0;
											GSM_ON_FLAG = 1;
											//printf("Sim80x_SetPower(true)15\r\n");
											Sim80x_SetPower(true);
									}
							}
							else
							{
								if(run_rtc.tm_hour % 3 == 0  && run_rtc.tm_minute == 0  && run_rtc.tm_seconds < 2 && IsNeedSendCook == true)
								{
									if(GSM_ON_FLAG == 0)
									{
											connectStep = 0;
											GSM_ON_FLAG = 1;
											//printf("Sim80x_SetPower(true)15\r\n");
											Sim80x_SetPower(true);
									}
								}
							}
            }
        }
				
				//当盖上盖子的时候,清零这个值,那么当打开的时候由可以立即报警
				if(Opening_input == 0)
				{
				  WarnTime = 0;
				}
				
				//当盖子是打开的时候
        if((Opening_input == 1 || vibrationsNumber >= 3) 
           && (WarnTime == 0 || Get_SubSeconds(StartWarn) > 60 * 5)
				  )
        {
            //如果这个时候,锁子是关闭的 glockSet != motor_open &&  //glockSet == motor_close && IsNeedIllegal_lid_opening == true
            if(IsNeedIllegal_lid_opening == true)//(strcmp(REAL_DATA_PARAM.Electric_Meter.LidElectricLock,"1")==0)
            {
								if(Opening_input == 1)
								{
									WarnTime = 1;
									StartWarn = Read_Time();
									//IsNeedIllegal_lid_opening = false;
								}
                
                //printf("LidElectricLock:%s\r\n",REAL_DATA_PARAM.Electric_Meter.LidElectricLock);
                if(GSM_ON_FLAG == 0)
                {
									  IsNeedWarning = true;//非法开盖报警 从判断外移动到内部
									
                    gkeyValue = true;//让屏幕亮起来
                    connectStep = 0;
                    GSM_ON_FLAG = 1;
                    printf("Sim80x_SetPower(true)1\r\n");
                    Sim80x_SetPower(true);
                }
//                if(sStep != 0)//屏幕不亮的时候蜂鸣器响会有问题
//                {
//                    HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
//                    osDelay(500);
//                    HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
//                }
            }
        }
//		else
//		{
//			  //关闭盖子不需要报警
//			  IsNeedWarning = false;
//			  if(strcmp(CONFIG_Meter.Lid_Type, "1")==0 && lockValue == true)//如果是一代的话,增加30秒可以开盖
//				{
//					lockValue = false;
//					OpenLockTime = 30;
//					glockSet = motor_open;
//				}
//		}
				
				
        //printf("connectStep ----> %d\r\n",connectStep);
        //联网的逻辑
        if(GSM_ON_FLAG == 1)// && IsReadVoltage == false
        {
            switch(connectStep)
            {
            case 0://执行到这里就停止了
                //printf("start connectStep ----> %d\r\n",connectStep);
                TimeForPowerStart = HAL_GetTick();
                TimeForCurrStart = HAL_GetTick();
                connectStep = 1;//初始化联网需要的参数
                break;
            case 1://执行上电的操作与判断
                //printf("Sim80x.Status.Power %d\r\n",Sim80x.Status.Power);
                if(Sim80x.Status.Power==1)
                {
                    connectStep = 2;//上电成功为2
                    loopSumCurr = 0;
                    TimeForCurrStart = HAL_GetTick();
                }
                else
                {
                    if(HAL_GetTick() - TimeForCurrStart >= 10 * 1000 )//开机10秒AT模块没有反应,重试两次,一共三次,模块坏了,1长
                    {
                        loopSumCurr++;
                        if(loopSumCurr >= 3)
                        {
                            //模块坏了  也可能是注册过程中出现了  0
                            if(sStep != 0)
                            {
                                HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
                                osDelay(500);
                                HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
                                osDelay(500);
                                HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
                                osDelay(500);
                                HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
                            }
                            if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
                            {
                                HAL_IWDG_Refresh(&hiwdg);
                            }
                            Sim80x_GPRSClose(1);
                            //加屏幕提示
                            //DisplayChar(1,1,"---GPRS Error---");
                        }
                        else
                        {
                            if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
                            {
                                HAL_IWDG_Refresh(&hiwdg);
                            }
                            Sim80x_SetPower(false);
                            osDelay(2000);
                            if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
                            {
                                HAL_IWDG_Refresh(&hiwdg);
                            }
                            printf("Sim80x_SetPower(true)5\r\n");
                            Sim80x_SetPower(true);
                            TimeForCurrStart = HAL_GetTick();
                        }
                    }
                }
                break;
            case 2:
                if(strcmp(CONFIG_GPRS.USE_GPRS_APN, "1")==0)
                {
                    if(true == GPRS_CGDCONT(CONFIG_GPRS.APN))
                    {
                        TimeForCurrStart=HAL_GetTick();//PDP(Packet Data Protocol )成功的时间
                        loopSumCurr = 0;
                        connectStep = 3;
                    }
                    else
                    {
                        if(HAL_GetTick() - TimeForCurrStart >= 5 * 1000 )
                        {
                            loopSumCurr++;
                            if(loopSumCurr >= 3)
                            {
                                if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
                                {
                                    HAL_IWDG_Refresh(&hiwdg);
                                }
                                Sim80x_GPRSClose(2);

                                //加屏幕提示
                                //DisplayChar(1,1,"---GPRS Error---");
                            }
                            else
                            {
                                if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
                                {
                                    HAL_IWDG_Refresh(&hiwdg);
                                }
                                Sim80x_SetPower(false);
                                osDelay(2000);
                                printf("Sim80x_SetPower(true)4\r\n");
                                Sim80x_SetPower(true);
                                TimeForCurrStart = HAL_GetTick();
                            }
                        }
                    }
                }
                else
                {
                    loopSumCurr = 0;
                    connectStep = 3;
                    TimeForCurrStart=HAL_GetTick();//PDP(Packet Data Protocol )成功的时间
                }
                break;
            case 3:
                if(Sim80x.Status.SimCard != 0)
                {
                    if(Sim80x.Status.SimCard == 2)
                    {
                        //Sim80x_SendAtCommand("AT+CPIN=\"4294\"\r\n",2000,1,"OK");
											GPRS_SetPin(CONFIG_GPRS.Pin);
                    }
                    IsHaveSimError = false;
                    connectStep = 4;//寻找到SIM卡
                    loopSumCurr = 0;
                    TimeForCurrStart = HAL_GetTick();//开始注册开始计时
                }
                else
                {
                    if(HAL_GetTick() - TimeForCurrStart >= 15 * 1000 )
                    {
                        IsHaveSimError = true;
                        if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
                        {
                            HAL_IWDG_Refresh(&hiwdg);
                        }
                        Sim80x_GPRSClose(3);
                    }
                }
                break;
            case 4:
                if(Sim80x.Status.RegisterdToNetwork==1)//成功了下一条
                {
                    connectStep = 5;//注册成功为5
                    loopSumCurr = 0;
                    TimeForCurrStart = HAL_GetTick();//网络注册成功的时间
                    //DisplayChar(1,1,"--Register  OK--");
                }
                else if(Sim80x.Status.RegisterdToNetwork == 3 || HAL_GetTick() - TimeForCurrStart >= 60 * 1000 * 10)//不成功或者超时重新来过
                {

                    loopSumCurr++;
                    //printf("RegisterdToNetwork:%d\r\n",Sim80x.Status.RegisterdToNetwork);
                    //printf("RegisterdToNetwork:%d  |  %d\r\n",HAL_GetTick() - TimeForCurrStart,20 * 1000);
                    //printf("Reging loopSumCurr:%d\r\n",loopSumCurr);

                    if(loopSumCurr >= 3)
                    {
                        //没有注册成功
                        if(sStep != 0)
                        {
                            HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
                            osDelay(500);
                            HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
                        }
                        if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
                        {
                            HAL_IWDG_Refresh(&hiwdg);
                        }
                        Sim80x_GPRSClose(4);

                        //加屏幕提示注册未成功
                        //DisplayChar(1,1,"-Register ERROR-");
                    }
                    else
                    {
                        if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
                        {
                            HAL_IWDG_Refresh(&hiwdg);
                        }
                        Sim80x_SetPower(false);
                        osDelay(2000);
                        printf("Sim80x_SetPower(true)3\r\n");
                        Sim80x_SetPower(true);
                        TimeForCurrStart = HAL_GetTick();
                        //DisplayChar(1,1,"-ReRegister ing-");
                    }
                }
                else//测试中不停发送CGREG
                {
                    if(HAL_GetTick() - TimeForCurrStart > 1000)
                    {
                        Sim80x_SendAtCommand("AT+CSQ\r\n",200,1,"\r\n+CSQ:"); //信号质量
                        //Sim80x_SendAtCommand("AT+CBC\r\n",200,1,"\r\n+CBC:"); //模块的电压
                        Sim80x_SendAtCommand("AT+CGREG?\r\n",200,1,"\r\n+CGREG:");
                        //Gsm_MsgGetMemoryStatus();//选择短信存储地点
                        osDelay(2000);
                        //printf("okokokokokokokokokokokokokokokokokokokokokokokokokokokok\r\n");
                    }

                }
                break;
            case 5:
                if(GPRS_StartCGATT(loopSumCurr)==true)
                {
                    connectStep = 6;
                    loopSumCurr = 0;
                    TimeForCurrStart = HAL_GetTick();//附加网络成功的时间
                }
                else
                {
                    if(HAL_GetTick() - TimeForCurrStart >= 10 * 1000 )
                    {
                        loopSumCurr++;

                        if(loopSumCurr >= 3)
                        {
                            //附加网络失败
                            if(sStep != 0)
                            {
                                HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
                                osDelay(500);
                                HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
                            }
                            if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
                            {
                                HAL_IWDG_Refresh(&hiwdg);
                            }
                            Sim80x_GPRSClose(5);

                            //加屏幕提示注册未成功
                            //DisplayChar(1,1,"-Register ERROR-");
                        }
                        else
                        {
                            TimeForCurrStart = HAL_GetTick();
                            //DisplayChar(1,1,"-ReRegister ing-");
                        }
                    }
                }
                break;
            case 6:
                if(GPRS_DeactivatePDPContext()==false)
                {
                    loopSumCurr++;
                    printf("\r\nloopSumCurr6 %d\r\n",loopSumCurr);
                    if(loopSumCurr >= 3)
                    {
                        //附加网络失败
                        if(sStep != 0)
                        {
                            HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
                            osDelay(500);
                            HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
                        }
                        if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
                        {
                            HAL_IWDG_Refresh(&hiwdg);
                        }
                        Sim80x_GPRSClose(6);

                        //加屏幕提示注册未成功
                        //DisplayChar(1,1,"-Register ERROR-");
                    }
                    else
                    {
                        TimeForCurrStart = HAL_GetTick();
                        //DisplayChar(1,1,"-ReRegister ing-");
                    }
                }
                else
                {
                    connectStep = 7;
                    loopSumCurr = 0;
                    TimeForCurrStart = HAL_GetTick();//成功的时间
                }
                break;
            case 7:
                if(strcmp(CONFIG_GPRS.USE_GPRS_APN, "1") == 0)
                {
                    GPRS_SetAPN(CONFIG_GPRS.APN,CONFIG_GPRS.APN_UserName,CONFIG_GPRS.APN_Password);
                }
                else
                {
                    if(GPRS_GetAPN(pAPN,pAPN_UserName,pAPN_Password) == true)
                    {
                        GPRS_SetAPN(pAPN,pAPN_UserName,pAPN_Password);
                    }
                }
                connectStep = 8;
                loopSumCurr = 0;
                if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
                {
                    HAL_IWDG_Refresh(&hiwdg);
                }
                TimeForCurrStart = HAL_GetTick();//成功的时间
                break;
            case 8:
                //printf("\r\nloopSumPDP %d\r\n",loopSumPDP);
                if(loopSumPDP >= 3)//如果不论什么情况下建立PDP的次数为3次,结束本次连接
                {
                    if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
                    {
                        HAL_IWDG_Refresh(&hiwdg);
                    }
                    Sim80x_GPRSClose(8);
                    break;
                    //加屏幕提示注册未成功
                    //DisplayChar(1,1,"-Register ERROR-");
                }
                if(GPRS_StartUpGPRS()==false)
                {
                    connectStep = 6;
                    TimeForCurrStart = HAL_GetTick();
                    //DisplayChar(1,1,"-ReRegister ing-");
                }
                else
                {
                    connectStep = 9;
                    loopSumCurr = 0;
                    TimeForCurrStart = HAL_GetTick();//成功的时间
                }
                loopSumPDP++;
                break;
            case 9:
                GPRS_GetLocalIP(NULL);
                GPRS_ShowGPRSIPD();
                connectStep = 10;
                loopSumCurr = 0;
                if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
                {
                    HAL_IWDG_Refresh(&hiwdg);
                }
                TimeForCurrStart = HAL_GetTick();//成功的时间
                break;
            case 10:
                if(GPRS_ConnectToServer() == false)
                {
                    loopSumCurr++;
                    if(loopSumCurr >= 3)
                    {
                        if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
                        {
                            HAL_IWDG_Refresh(&hiwdg);
                        }
                        Sim80x_GPRSClose(10);

                        //加屏幕提示注册未成功
                        //DisplayChar(1,1,"-Register ERROR-");
                    }
                    else
                    {
                        //Sim80x.GPRS.Connection[0] = GPRSConnection_ConnectFail;
                        TimeForCurrStart = HAL_GetTick();
                        //DisplayChar(1,1,"-ReRegister ing-");
                    }
                }
                else
                {
                    connectStep = 11;
                    loopSumCurr = 0;
                    TimeForCurrStart = HAL_GetTick();//成功的时间
                }
                break;
            case 11:
                if(HAL_GetTick()-TimeForSlowRunGPRSFree > 1500)
                {
                    HAL_GPIO_WritePin(RUN_LED_GPIO_Port,RUN_LED_Pin,GPIO_PIN_SET);
                    osDelay(500);
                    HAL_GPIO_WritePin(RUN_LED_GPIO_Port,RUN_LED_Pin,GPIO_PIN_RESET);

                    //GPRS_GetCurrentConnectionStatus();

                    if(Sim80x.GPRS.Connection[0] != GPRSConnection_ConnectOK && Sim80x.GPRS.Connection[0] != GPRSConnection_AlreadyConnect)
                    {
                        printf("Sim80x.GPRS.Connection[0] %02X\r\n",Sim80x.GPRS.Connection[0]);
                        connectStep = 6;//联网过程中失败,断开上下文重新连接
                        //Sim80x.GPRS.Connection[0] = GPRSConnection_ConnectFail;
                        //Sim80x_GPRSClose(99);
                        break;
                    }
                    else
                    {
                        if(HAL_GetTick() - TimeForCurrStart >= 2000)
                        {
                            if(IsNeedRepaySetup == true)
                            {
                                //printf("IsNeedRepaySetup\r\n");
                                SendSetupPacket();
                                IsNeedRepaySetup = false;
                                TimeForCurrStart = HAL_GetTick();
																HearRetryNumber = 0;
                            }
                            else if(IsNeedRepayCYCH == true)
                            {
                                //printf("IsNeedRepayCYCH\r\n");
                                SendResetPacket();
                                IsNeedRepayCYCH = false;
                                TimeForCurrStart = HAL_GetTick();
																HearRetryNumber = 0;
                            }
                            else if(IsNeedRepayControl == true)//控制报文回复
                            {
                                //printf("IsNeedRepayControl\r\n");
                                SendControlPacket();
                                IsNeedRepayControl = false;
                                TimeForCurrStart = HAL_GetTick();
															  HearRetryNumber = 0;
                            }
                            else if(IsNeedRepayReCharge == true)//控制报文回复
                            {
                                //printf("IsNeedRepayReCharge\r\n");
                                //SendRechargePacket();
                                if(SendRechargePacket() == true)
                                {
                                    HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
                                    osDelay(200);
                                    HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);

                                    RechargeToMeter(&RechargePacket);

                                    int16_t TmpIndex = 0;

                                    //如果充值的卡为当前卡,直接充值
                                    if(stringCmp(Current_Credit.CUSTOMER_ID,RechargePacket.CUSTOMER_ID,7) == PASSED
                                            && stringCmp(Current_Credit.CARD_ID,RechargePacket.CARD_ID,18) == PASSED)
                                    {
                                        Current_Credit.CurrCredit += rechargeCredit;
                                        REAL_Current_Credit_Write();
                                        Current_Credit_Write(Current_Credit);//当前表卡是需要充值的卡的时候,处理充值命令
                                    }
                                    else//如果不是这个当前卡,就需要寻找充值
                                    {
                                        if(REAL_DATA_Credit.CardTotal > 0)//寻找到直接充值,并保存
                                        {
                                            while(TmpIndex < REAL_DATA_Credit.CardTotal)
                                            {
                                                memset(&Tmp_Credit,0,sizeof(Current_Credit_t));
                                                Current_Credit_READ(TmpIndex);	//充值的时候先读取再挨个比对
                                                if(stringCmp(Tmp_Credit.CUSTOMER_ID,RechargePacket.CUSTOMER_ID,7) == PASSED
                                                        && stringCmp(Tmp_Credit.CARD_ID,RechargePacket.CARD_ID,18) == PASSED)
                                                {
                                                    Tmp_Credit.CurrCredit = Tmp_Credit.CurrCredit + rechargeCredit;
                                                    Current_Credit_Write(Tmp_Credit);//当前充值的卡不是正在使用的卡片的时候,在记录中找到了卡片
                                                    break;
                                                }
                                                TmpIndex++;
                                            }
                                        }
                                        //寻找不到新建卡
                                        if(TmpIndex == REAL_DATA_Credit.CardTotal)
                                        {
                                            memset(&Tmp_Credit,0,sizeof(Current_Credit_t));
                                            Tmp_Credit.CurrIndex = TmpIndex;
                                            Tmp_Credit.CUSTOMERNoLength = 7;
                                            strncpy(Tmp_Credit.CUSTOMER_ID,RechargePacket.CUSTOMER_ID,8);
                                            Tmp_Credit.CARDNoLength = 18;
                                            strncpy(Tmp_Credit.CARD_ID,RechargePacket.CARD_ID,19);

                                            Tmp_Credit.CurrCredit = rechargeCredit;

                                            REAL_DATA_Credit.CardTotal++;
                                            LL_VCC(1);
                                            REAL_DATA_Credit_Write();//充值中在内存中找不到卡片,新建卡片,卡片索引加一
                                            Current_Credit_Write(Tmp_Credit);//当前充值的卡不是正在使用的卡片的时候,在记录中找不到卡片
                                        }
                                    }
                                }
                                IsNeedRepayReCharge = false;
                                TimeForCurrStart = HAL_GetTick();
																HearRetryNumber = 0;
                            }
                            else if(IsNeedTimeing == true)//定时报
                            {
                                //printf("IsNeedTimeing\r\n");
                                if(IsSendHalfTime == false)
                                {
                                    if(SendReportDataPacket() == true)
																		{
																			IsSendHalfTime = true;
																		}
																	  TimeForCurrStart = HAL_GetTick();
                                }
                                else
                                {
																	if(HAL_GetTick()-TimeForCurrStart > 1000 * 5)
																	{
                                    if(SendReportStatePacket() == true)
																		{
																			IsSendHalfTime = false;
																			IsNeedTimeing = false;
																		}
                                    //TimeRetryNumber = 0;
                                    TimeForCurrStart = HAL_GetTick();
																	}
                                }
																HearRetryNumber = 0;
                            }
                            else if(IsNeedWarning == true)//报警信息
                            {
																if(HAL_GetTick()-TimeForCurrStart > 1000 * 5)
																{
																		//IsNeedIllegal_lid_opening = true;
																		//printf("IsNeedWarning\r\n");
																		SendWarnPacket();
																		IsNeedWarning = false;
																		TimeForCurrStart = HAL_GetTick();
																}
																HearRetryNumber = 0;
                            }
                            else if(IsNeedSendCook == true)//心跳包触发,如果有需要上传的信息上传
                            {
                                //if have CSRP,Send data every five seconds,if revice Reply,send next CSRP
                                if(REAL_DATA_Credit.CookingSessionEnd > REAL_DATA_Credit.CookingSessionSendNumber &&
																	(
																	 (IsReceivedCSRPReply == true && HAL_GetTick()-TimeForCurrStart > 1000 * 10)//如果收到回复也要等十秒
																		 || (IsReceivedCSRPReply == false && IsSendedCSRP == false && HAL_GetTick()-TimeForCurrStart > 1000 * 10) //保证第一次可以进入
																  )
																)
																{
																	  IsSendedCSRP = true;
																	  IsReceivedCSRPReply = false;
                                    LL_VCC(1);
                                    Cooking_Session_READ(REAL_DATA_Credit.CookingSessionSendNumber);//发送的时候从开始位置开始读取,发送成功,索引加一
																		
                                    //													printf("CookingSessionReport2 %d\r\n",sizeof(CookingSessionReport_t));
                                    //													printf("CookingSessionReport2 %s\r\n",CookingSessionReport.CUSTOMER_ID);
                                    //													printf("CookingSessionReport2 %s\r\n",CookingSessionReport.CREDIT_SESSION_START);
                                    //													printf("CookingSessionReport2 %s\r\n",CookingSessionReport.CREDIT_SESSION_END);

                                    //													printf("CookingSessionReport2 %s\r\n",CookingSessionReport.START_CUMULATIVE_VOLUME);
                                    //													printf("CookingSessionReport2 %s\r\n",CookingSessionReport.END_CUMULATIVE_VOLUME);
                                    //
                                    //													printf("CookingSessionReport2 %s\r\n",CookingSessionReport.SESSION_START_TIME);
                                    //													printf("CookingSessionReport2 %s\r\n",CookingSessionReport.SESSION_END_TIME);
                                    //													printf("CookingSessionReport2 %s\r\n",CookingSessionReport.GAS_REMAINING);
																	  
                                   
                                    if(SendCookingSessionPacket() == true)//Send successfully internal do not process index,change to reply
																		{
																			LL_VCC(1);
																			REAL_DATA_Credit_Write();//发送完cooking ,保存序号
																		}
																	  TimeForCurrStart = HAL_GetTick();
                                }
                                else
                                {
																		if(REAL_DATA_Credit.CookingSessionEnd == REAL_DATA_Credit.CookingSessionSendNumber)
																		{
																			IsNeedSendCook = false;
																			TimeForCurrStart = HAL_GetTick();
																		}
                                }
																
																IsSaveREAL_DATA_Credit = true;
                            }
                            else if(IsNeedCustomerCreditResponse == true)
                            {
                                //printf("IsNeedCustomerCreditResponse\r\n");
                                IsNeedCustomerCreditResponse = false;
                                SendCustomerCreditPacket();
                                TimeForCurrStart = HAL_GetTick();
															  HearRetryNumber = 0;
                            }
                            else if(IsNeedInformationResponse == true)
                            {
                                //printf("IsNeedInformationResponse\r\n");
                                IsNeedInformationResponse = false;
                                SendInformationPacket();

                                commandType = 0;
                                TimeForCurrStart = HAL_GetTick();
															  HearRetryNumber = 0;
                            }
                            else
                            {
                                //printf("SendHeartbeatPacket\r\n");
                                if(HAL_GetTick()-TimeForCurrStart > 1000 * 10)
                                {
                                    SendHeartbeatPacket();
                                    TimeForCurrStart=HAL_GetTick();
                                    HearRetryNumber++;
                                }
                            }
                        }
                    }

                    if(
											//(IsNeedHearting == true && HAL_GetTick() - TimeForCurrStart >= 20 * 1000) || //如果是心跳包发送,从连接上开始计时6秒断开
                            (IsNeedWarning == false
                                && IsNeedTimeing == false
                                //&& IsNeedSendCook == false
                                && IsNeedCustomerCreditResponse == false
                                && IsNeedInformationResponse == false
                                && IsNeedRepaySetup == false
                                && IsNeedRepayControl == false
                                && IsNeedRepayCYCH == false
                                && IsNeedRepayReCharge == false
                                //&& HAL_GetTick() - TimeForCurrStart >= 30 * 1000
																&& HearRetryNumber >= 3
																)
                      )
                    {
                        HearRetryNumber = 0;

                        IsNeedTimeing = false;
                        IsSendHalfTime = false;
                        //TimeRetryNumber = 0;

                        if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
                        {
                            HAL_IWDG_Refresh(&hiwdg);
                        }
                        Sim80x_GPRSClose(11);
                        //printf("connectStep4 %d\r\n",connectStep);
                    }

                    TimeForSlowRunGPRSFree=HAL_GetTick();
                }
                break;
            }

            if(connectStep > 0)
            {
							if(HAL_GetTick() - TimeForPowerStart >= 60 * 1000 * ((strstr(CONFIG_Meter.MeterNo,"KE") != NULL && Sim80x.Modem_Type == SIM7020E) ? 12 : 3))//上电开始算超过3分钟
                {
                    HearRetryNumber = 0;

                    IsNeedTimeing = false;
                    IsSendHalfTime = false;//是否已经发送了一半的定时报
                    //TimeRetryNumber = 0;

                    if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
                    {
                        HAL_IWDG_Refresh(&hiwdg);
                    }
                    Sim80x_GPRSClose(30);
                    //printf("connectStep4 %d\r\n",connectStep);
                }

                if(connectStep > 4 && HAL_GetTick() - TimeForSlowRunFree > 5000)
                {
                    Sim80x_SendAtCommand("AT+CSQ\r\n",200,1,"\r\n+CSQ:"); //信号质量
                    //Sim80x_SendAtCommand("AT+CBC\r\n",200,1,"\r\n+CBC:"); //模块的电压
                    TimeForSlowRunFree=HAL_GetTick();
                }
            }
        }

//				printf("Lcd_Status %d\r\n",Lcd_Status);
//				printf("sStep %d\r\n",sStep);
//				printf("IsTestPower %d\r\n",IsTestPower);
//				printf("connectStep %d\r\n",connectStep);
//				printf("GSM_ON_FLAG %d\r\n",GSM_ON_FLAG);
				
        //printf("StartRunTime %d\r\n",StartRunTime);
        //进入低功耗
        if(((Current_card_type == 0 //没有卡片
             && strcmp(REAL_DATA_PARAM.ElectricValveStatus, "0")==0)// && IsReadVoltage == false
             ||(Current_card_type == 1 //有卡片
                && strcmp(REAL_DATA_PARAM.ElectricValveStatus, "1")==0 	 //阀门是开着的
                   //&& StartRunTime > 15 //运行超过3秒
                && IsTestEnd == true
                && (IsStartCooking5 == true && Get_SubSeconds(StartCookingSeconds) >= 120)) //如果开始做饭超过五分钟
           )
                && Lcd_Status == false  //屏幕是关闭
                //&& gkeyValue == false
                && sStep == 0
                && IsTestPower == false  //没有进行电量采集
                && gmotorStatus == motor_null && gassembleStatus == motor_null //阀门都没有需要动作的情况下
                && connectStep == -1     //没有联网
                //&& 1 != 1
                && GSM_ON_FLAG == 0   //没有尝试联网
				)   
        {
            IsIntoLowPower = true;
            if(IsStartCooking5 == false && Get_SubSeconds(StartCookingSeconds) >= 120)
            {
                //IsStartCooking5 = true;//如果是开启做饭中低功耗,请取消注释
                //HAL_TIM_Base_Stop_IT(&htim6);
                //HAL_TIM_Base_Stop_IT(&htim7);
            }

//				if(Current_card_type == 1)
//			  {
//					printf("Cook-----------------------------------ing\r\n");
//					printf("zeroFlowTime %d\r\n",zeroFlowTime);
//					printf("SESSION_START_TIME %s\r\n",CookingSessionReport.SESSION_START_TIME);
//					printf("Get_SubSeconds %d\r\n",Get_SubSeconds(StartCookingSeconds));
//					printf("IsStartCooking5 : %d\r\n",IsStartCooking5);
//				}

            /* Enable Power Clock*/
            //			__HAL_RCC_PWR_CLK_ENABLE();

            IntoLowPower();

            //			RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
            //	    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

            /*
            	 1. 停止模式是在Cortex-M3的深睡眠模式基础上结合了外设的时钟控制机制，在停止模式下电压
            		调节器可运行在正常或低功耗模式。此时在1.8V供电区域的的所有时钟都被停止， PLL、 HSI和
            		HSE的RC振荡器的功能被禁止， SRAM和寄存器内容被保留下来。
            	 2. 在停止模式下，所有的I/O引脚都保持它们在运行模式时的状态。
            	 3. 一定要关闭滴答定时器，实际测试发现滴答定时器中断也能唤醒停机模式。
            */
            //		SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;  /* 关闭滴答定时器 */

            RTC_TimeTypeDef sTime;
            RTC_DateTypeDef sDate;
            HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
            HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
            //printf("Into LowPower %02d:%02d:%02d\r\n",sTime.Hours,sTime.Minutes,sTime.Seconds);

            HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
						//printf("aaaaaaa\r\n");
            //			printf("gkeyValue : %d\r\n\r\n",gkeyValue);

//            HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
//            HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
            //printf("out LowPower %02d:%02d:%02d\r\n",sTime.Hours,sTime.Minutes,sTime.Seconds);
            //printf("CPU_RunTime %d\r\n",CPU_RunTime);
            StartRunTime = 0;
            sStep = 0;

            //printf("__HAL_RCC_GET_SYSCLK_SOURCE()1 %d\r\n",__HAL_RCC_GET_SYSCLK_SOURCE());
            /*
            		1、当一个中断或唤醒事件导致退出停止模式时， HSI RC振荡器被选为系统时钟。
            		2、退出低功耗的停机模式后，需要重新配置使用HSE。
            */
            /* Set the new HSE configuration ---------------------------------------*/
            if(MYSYSCLK == 8)
						{
								//开启外部高速时钟
								__HAL_RCC_HSE_CONFIG(RCC_HSE_ON);
								while (__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == RESET)
								{
										//是否开启完成
								}
								//选择系统时钟为HSE
								__HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_HSE);
								//printf("__HAL_RCC_GET_SYSCLK_SOURCE()2 %d\r\n",__HAL_RCC_GET_SYSCLK_SOURCE());
								//判断是否启用成功
								while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_HSE)
								{
										//是否配置成功
										printf("__HAL_RCC_GET_SYSCLK_SOURCE()3 %d\r\n",__HAL_RCC_GET_SYSCLK_SOURCE());
										//如果不成功怎么办,重启吧
								}
								//printf("__HAL_RCC_GET_SYSCLK_SOURCE()4 %d\r\n",__HAL_RCC_GET_SYSCLK_SOURCE());
								//			SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; /* 使能滴答定时器 */
					  }
						else
						{
								//开启外部高速时钟
								__HAL_RCC_HSE_CONFIG(RCC_HSE_ON);
								while (__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == RESET)
								{
									//printf("1");
								}
								/* Disable the main PLL. */
								__HAL_RCC_PLL_DISABLE();
								
								/* Wait till PLL is disabled */
								while(__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY)  != RESET)
								{
									//printf("11");
								}
								

								/* Configure the main PLL clock source, predivider and multiplication factor. */
								__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE,
																		 RCC_PREDIV_DIV1,
																		 RCC_PLL_MUL6);
								/* Enable the main PLL. */
								__HAL_RCC_PLL_ENABLE();
								
								/* Wait till PLL is ready */
								while(__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY)  == RESET)
								{
									//printf("111");
								}
                //选择SYSCLK为PLL
								__HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_PLLCLK);

								while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_PLLCLK)
								{
									//printf("1111");
								}
								//printf("__HAL_RCC_GET_SYSCLK_SOURCE()4 %d\r\n",__HAL_RCC_GET_SYSCLK_SOURCE());
								//			SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; /* 使能滴答定时器 */
						}	
            __HAL_RCC_GPIOA_CLK_ENABLE();
            __HAL_RCC_GPIOB_CLK_ENABLE();
            __HAL_RCC_GPIOC_CLK_ENABLE();
            __HAL_RCC_GPIOD_CLK_ENABLE();
            __HAL_RCC_GPIOF_CLK_ENABLE();
            //关闭电机的引脚
            //		  HAL_GPIO_WritePin(MOTOR1_H_PORT,MOTOR1_H_PIN,GPIO_PIN_RESET);
            //		  HAL_GPIO_WritePin(MOTOR1_L_PORT,MOTOR1_L_PIN,GPIO_PIN_RESET);
            //
            //			HAL_GPIO_WritePin(MOTOR2_H_PORT,MOTOR2_H_PIN,GPIO_PIN_RESET);
            //			HAL_GPIO_WritePin(MOTOR2_L_PORT,MOTOR2_L_PIN,GPIO_PIN_RESET);
            //
            //			HAL_GPIO_WritePin(MOTOR3_H_PORT,MOTOR3_H_PIN,GPIO_PIN_RESET);
            //			HAL_GPIO_WritePin(MOTOR3_L_PORT,MOTOR3_L_PIN,GPIO_PIN_RESET);

            //打开RFID
            HAL_GPIO_WritePin(RC522_Reset_GPIO_Port,RC522_Reset_Pin,GPIO_PIN_SET);//置高,保持不复位
            //以下三个置高,表示都不能用
            HAL_GPIO_WritePin(SPI_CS3_GPIO_Port,SPI_CS3_Pin,GPIO_PIN_SET);//RFID CS 置高成不能用
            MB_SPI_CS_DISABLE;     //小Flash CS 不使能
            FLASH_SPI_CS_DISABLE;  //大Flash CS 不使能

            HAL_SPI_MspInit(&hspi2);
            //HAL_UART_MspInit(&huart3);
            //MX_USART3_UART_Init();
            //HAL_UART_Receive_IT(&huart3,&Usart3RxTemp,1);//从低功耗启动的时候重新中断接收数据
            uint32_t i = 0;
            while(HAL_UART_Receive_IT(&huart3,&Usart3RxTemp,1) != HAL_OK )
            {
                i++;
                if( i > 10000 )
                {
                    huart3.RxState = HAL_UART_STATE_READY;
                    __HAL_UNLOCK(&huart3);
                    i = 0;
                }
            }
            rec_count = 0;

            i = 0;
            //HAL_UART_Receive_IT(&_SIM80X_USART,&Sim80x.UsartRxTemp,1);
            while(HAL_UART_Receive_IT(&_SIM80X_USART,&Sim80x.UsartRxTemp,1) != HAL_OK )
            {
                i++;
                if( i > 10000 )
                {
                    _SIM80X_USART.RxState = HAL_UART_STATE_READY;
                    __HAL_UNLOCK(&_SIM80X_USART);
                    i = 0;
                }
            }
						osDelay(100);
            IsIntoLowPower = false;
            //printf("Lcd_Status:%d\r\n",Lcd_Status);
            if(Lcd_Status == true && IsStartCooking5 == true)//如果是按键退出低功耗模式
            {
                IsTestEnd = false;
                IsStartCooking5 = false;
                StartCookingSeconds = Read_Time();
                //HAL_TIM_Base_Start_IT(&htim7);

                //printf("IsStartCooking5 4\r\n");
            }
            else
            {
                //一分钟测试一次,这个时候正常打开测试,测试完成后,这一分钟按照本次测量走,测量完成后进去低功耗
                if(strcmp(REAL_DATA_PARAM.ElectricValveStatus, "1")==0 && Read_Time().tm_seconds == 0)//如果阀门是开着的
                {
                    //gkeyValue = true;
                    IsTestEnd = false;
                    //打开升压
                    ENBOOST_PWR(1);
                    GPRS_PWR(1);

                    //HAL_ADCEx_Calibration_Start(&hadc);
                    //HAL_ADC_Start_DMA(&hadc, (uint32_t*)&ADC_Data, 4);

                    //HAL_TIM_Base_Start_IT(&htim7);
                }
            }
						
            HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
            //HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
            //printf("Out LowPower %02d:%02d:%02d\r\n",sTime.Hours,sTime.Minutes,sTime.Seconds);
        }

        //从单独一个线程移动到这里,并且在进入低功耗是判断是否有阀门有动作没有完成
        //一个一个的执行
        if(glockStatus != motor_null)
        {
            lock_test();
            IsSaveREAL_DATA_PARAM = true;
        }
        else
        {
            if(gmotorStatus != motor_null)
            {
                motor_test();
                IsSaveREAL_DATA_PARAM = true;
            }
            else
            {
                if(gassembleStatus != motor_null)
                {
                    assemble_test();
                    IsSaveREAL_DATA_PARAM = true;
                }
            }
        }

        if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
        {
            HAL_IWDG_Refresh(&hiwdg);
        }

        osDelay(10);
    }
  /* USER CODE END StartDefaultTask */
}

/* StartkeyTask function */
void StartkeyTask(void const * argument)
{
  /* USER CODE BEGIN StartkeyTask */
    unsigned int tkeyValue,key2Value,key3Value;//,openLidValue;
    /* Infinite loop */
    for(;;)
    {
        tkeyValue = key_read(&key1_Status);
			
        //屏幕按键有动作
        if(tkeyValue != 0)
        {
            LCD_Delay_Time = 10 * 100;//每次有按键按下的时候,延长屏幕亮起时间
            //printf("LCD_Delay_Time3 %d\r\n",LCD_Delay_Time);
            //printf("keyValue   %d \r\n",tkeyValue);
            if(tkeyValue == 1) //Click button
            {
                haveClickButton = true;
                //LogWrite(ClickButton,100,"ClickButton");
            }

            if(tkeyValue == 2) //double Click button
            {
                //IsReadVoltage = true;

                if(strncmp(LockPassword + gkeySum, "2",1)==0)
                {
                    printf("gkeySum   %d \r\n",gkeySum);
                    gkeySum++;
                }
                else
                {
                    gkeySum = 0;
                }
								
                if(Opening_input == 0) //如果盖子为盖着的,同时双击按钮,以后有报警事件就会出发
                {
                    IsNeedIllegal_lid_opening = true;
                    WarnTime = 0;
                }
            }
            else
            {
                gkeySum = 0;
            }

            if(tkeyValue == 3)
            {
                haveLongButton = true;
            }
        }

        //内部按键的操Key2
        key2Value = key_read(&Key2_Status);

        if(key2Value != 0)
        {
            //printf("LCD_Delay_Time4 %d\r\n",LCD_Delay_Time);
            LCD_Delay_Time = 10 * 100;//每次有按键按下的时候,延长屏幕亮起时间
					  vibrationsNumber++;
					  //printf("vibrationsNumber %d\r\n",vibrationsNumber);
        }

        //光敏电阻的操作
//		    openLidValue = key_read(&OpenLid_Status);

//				if(openLidValue != 0)
//				{
//					 //printf("openLidValue %d\r\n",openLidValue);
//					 //LCD_Delay_Time = 10 * 100;//每次有按键按下的时候,延长屏幕亮起时间

//					//当光敏电阻一直拉低的时候表明盖子盖上了
//					if(openLidValue == 3)
//					{
//						IsNeedIllegal_lid_opening = true;
//						WarnTime = 0;
//					}
//				}

        key3Value = key_read(&Key3_Status);

        if(key3Value != 0)
        {
            //printf("LCD_Delay_Time6 %d\r\n",LCD_Delay_Time);
            LCD_Delay_Time = 10 * 100;//每次有按键按下的时候,延长屏幕亮起时间
            //printf("keyValue   %d \r\n",tkeyValue);
            if(key3Value == 1)
            {
                {
                    if(sStep == 0)
                    {
                        gkeyValue = true;
                    }

                    //							  RFID_POWER(1);
                    //					      osDelay(100);
                    //					      IsTestRFID = true;
                    //					      IsInitRFID = false;
                    //					      IsLockCard = false;
                    //如果针阀是开,拆卸锁是开,电机没有动作,防止电机还没有动作完成,立即动作了摇把,电机动作异常
                    if(strcmp(REAL_DATA_PARAM.TankLockStatus, "0")==0 && gassembleStatus == motor_null)
                    {
                        //printf("keyValue1\r\n");
                        strcpy(REAL_DATA_PARAM.TankLockStatus,"1");
                        gassembleStatus = motor_close;
                        RFID_Reset(1);
                        RFID_POWER(1);
                        osDelay(100);
                        IsTestRFID = true;
                        IsInitRFID = false;
                        IsLockCard = false;
                        IsSaveREAL_DATA_PARAM = true;
                        Current_card_type = 0;
                    }
                }
            }

            if(key3Value == 2)
            {
                if(strcmp(LockPassword + gkeySum, "1")==0)//电报开锁
                {
                    //重新启动 不用重新启动了
                    //HAL_NVIC_SystemReset();
                    strcpy(REAL_DATA_PARAM.TankLockStatus,"0");
                    gassembleStatus = motor_open;
                    OpenLockTime = 10;
                    glockSet = motor_open;
                    //printf("open\r\n");
                    IsTestRFID = false;
                    osDelay(100);
                    //RFID_POWER(0);
                    IsInitRFID = false;
                    IsNeedIllegal_lid_opening = false;
                }
                else
                {

                }
            }

            if(key3Value == 3)
            {
//					printf("key3Value   %d \r\n",sStep);
//					if(sStep == 0)
//					{
//						gkeyValue = true;
//					}
                if(strncmp(LockPassword + gkeySum, "3",1)==0)
                {
                    //printf("gkeySum   %d \r\n",gkeySum);
                    gkeySum++;
                }
                else
                {
                    gkeySum = 0;
                }
            }
        }

        osDelay(25);
    }
  /* USER CODE END StartkeyTask */
}

/* StartLedTask function */
void StartLedTask(void const * argument)
{
  /* USER CODE BEGIN StartLedTask */
    //那个牛逼检测内存需要的
    //uint32_t g_memsize;
    //uint8_t CPU_RunInfo[400];		//保存任务运行时间信息
    //uint32_t SubRunTime = 0;
    //SubRunTime = StartRunTime;
    long date,time;//UTC
    static uint8_t LCD_Refresh_Time = 0;
    //uint32_t i;
    /* Infinite loop */
    for(;;)
    {
//				if(StartRunTime -SubRunTime > 1)
//				{
//					SubRunTime = StartRunTime;
//					g_memsize = xPortGetFreeHeapSize();
//					printf("系统当前内存大小为 %d 字节\r\n",g_memsize);
//
//					memset(CPU_RunInfo,0,400);				//信息缓冲区清零
//
//					vTaskList((char *)&CPU_RunInfo);  //获取任务运行时间信息
//
//					printf("---------------------------------------------\r\n");
//					printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
//					printf("%s", CPU_RunInfo);
//					printf("---------------------------------------------\r\n");
//
//					memset(CPU_RunInfo,0,400);				//信息缓冲区清零
//
//					vTaskGetRunTimeStats((char *)&CPU_RunInfo);
//
//					printf("任务名       运行计数         使用率\r\n");
//					printf("%s", CPU_RunInfo);
//					printf("---------------------------------------------\r\n\n");
//					//vTaskDelay(5000);   /* 延时500个tick */
//				}

        //如果运行了半小时都没有进入低功耗,由没有刷卡的情况下
        //这个是可以起作用的奥
        if(StartRunTime > 1800 && Current_card_type == 0)//&& IsReadVoltage == false
        {
            IsSaveCONFIG_Meter = true;
            IsNeedRestart = true;
        }
        //当运行系统启动时间大于10秒后才允许第二次重启
        if(CPU_RunTime > 10)
        {
            IsNeedTimeRestart = true;
        }

//				if(SubRunTime <= CPU_RunTime - 60 && CPU_RunTime >= 60)
//				{
//					 SubRunTime = CPU_RunTime;
//					 //printf("CPU_RunTime %d\r\n",CPU_RunTime);
//				}

				if(IsIntoLowPower == false)
				{
						GetRTC(&time,&date);

						if(gkeyValue == true)
						{
								//printf("1\r\n");
								gkeyValue = false;
								sStep++;
								//printf("LCD_Delay_Time7 %d\r\n",LCD_Delay_Time);
								LCD_Delay_Time = 10 * 100;
								Lcd_Status = true;
								//不管是哪一款硬件,只要是屏幕亮起来的时候就开升压,开GPRS
								ENBOOST_PWR(1);
								GPRS_PWR(1);
						}
						if(Lcd_Status == true)
						{
							  //printf("2\r\n");
								if(LCD_Delay_Time != 0)
								{
										//printf("GSM_ON_FLAG %d\r\n",GSM_ON_FLAG);
										//printf("Current_card_type %d\r\n",Current_card_type);
										//printf("TankLockStatus %s\r\n",REAL_DATA_PARAM.TankLockStatus);
										if(GSM_ON_FLAG == 0 //首先没有联网
														&& (Current_card_type == 0 //没卡的时候息屏
																|| (Current_card_type == 1
																		&& strcmp(Current_Credit.CARD_ID, "111111111111111111")!=0
																		//&& strcmp(Current_Credit.CARD_ID, "222222222222222222")!=0
																		&& strcmp(Current_Credit.CARD_ID, "333333333333333333")!=0
																	 ))//正常用的时候息屏
														&& strcmp(REAL_DATA_PARAM.TankLockStatus, "1")==0 //钢瓶锁着的时候
											)
										{
												LCD_Delay_Time--;
											  //printf("LCD_Delay_Time: %d\r\n",LCD_Delay_Time);
										}
										//printf("LCD_Delay_Time %d\r\n",LCD_Delay_Time);
										{
												if(sStep == 1)
												{
														//屏幕亮起一直测试电压,整点测试电压
														BATAD_POWER(1);
														IsTestPower = true;

														if(Current_card_type == 0) //没有卡的时候打开RFID,实际上如果这个时候关闭阀门,可能由于多线程还没有刷新这个值
																//|| (Current_card_type == 1 && strcmp(REAL_DATA_PARAM.ElectricValveStatus, "0")==0) //用户卡阀门是关闭的打开RFID测试
																//|| Current_card_type == 2)
														{
																IsTestRFID = true;
														}

														//屏幕亮起来
														CS_W(1);
														RD_W(0);
														WR_W(0);
														DAT_W(0);
														osDelay(20);//初始化引脚后需要延时上电,否则会花屏
														LCD_POWER(1);
														LCD_LIGHT(1);
														LCD_Init();

														sStep++;
												}

												if(Current_card_type == 0 && sStep >= 4) //无卡
												{
														sStep = 2;
												}
												if((Current_card_type == 2) && sStep >= 3)//开锁卡
												{
														sStep = 2;
												}
												if((Current_card_type == 4) && sStep >= 3)
												{
														sStep = 2;
												}
												if(Current_card_type == 1)
												{
														if(strcmp(Current_Credit.CARD_ID, "111111111111111111")==0
																		|| strcmp(Current_Credit.CARD_ID, "222222222222222222")==0
																		|| strcmp(Current_Credit.CARD_ID, "333333333333333333")==0)
														{
																if(sStep >= 6)
																{
																		sStep = 2;
																}
		//										else
		//										{
		//											if(sStep == 2)
		//											{
		//												sStep = 4;
		//											}
		//										}
														}
														else
														{
																if(sStep >= 3)
																{
																		sStep = 2;
																}
														}
												}
												if(Current_card_type == 3 && sStep >= 4)
												{
														sStep = 2;
												}

												if(sStep == 2)//画面1
												{
														if(LCD_Refresh_Time < 10)
														{
																LCD_Refresh_Time++;
														}
														else
														{
																RefreshSignal();
																RefreshBatVoltage();
																RefreshValve();

																if(REAL_DATA_PARAM.TankQuality <= CONFIG_Meter.LowGasVolume || REAL_DATA_PARAM.TankQuality <= 0.5)
																{
																		RefreshCylinder();
																}

																if(Current_card_type == 0 || Current_card_type == 2 || Current_card_type == 4)//无卡,开锁,重置
																{
																		if(CONFIG_Meter.IsHaveRFID == false)
																		{
																				if(Current_Credit.CurrCredit > 999999)
																				{
																						LCDPanelStruct.DisNumber = Current_Credit.CurrCredit;
																						LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																						LCDPanelStruct.POINT6_Sign = OFF;
																						LCDPanelStruct.MONEY_Sign = ON;
																				}
																				else
																				{
																						LCDPanelStruct.DisNumber = Current_Credit.CurrCredit * 10;
																						LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																						LCDPanelStruct.POINT6_Sign = ON;
																						LCDPanelStruct.MONEY_Sign = ON;
																				}
																		}
																		else
																		{
																				if(No_valid_Card == false)
																				{
		//															strncpy(LCDPanelStruct.DisChar," --01--",7);
		//															LCDPanelStruct.DisType = 1;
		//															LCDPanelStruct.WRAN_Sign = ON;
		//															LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);

																						//if(Current_card_type == 0)
																						{
																								LCDPanelStruct.DisNumber = REAL_DATA_PARAM.TankQuality * 10;
																								LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																								LCDPanelStruct.POINT6_Sign = ON;
																								//LCDPanelStruct.cylinder_Sign = ON;
																								LCDPanelStruct.KgSign_Sign = ON;
																						}
																				}
																				else
																				{
																						strncpy(LCDPanelStruct.DisChar," --02--",7);
																						LCDPanelStruct.DisType = 1;
																						RefreshWarn();
																						LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																				}
																		}
																		LCD_Refresh(&LCDPanelStruct);
																}
																else if(Current_card_type == 1)
																{
																		//printf("Current_Credit.CurrCredit %f\r\n",Current_Credit.CurrCredit);
																		if(Current_Credit.CurrCredit > 999999)
																		{
																				LCDPanelStruct.DisNumber = Current_Credit.CurrCredit;
																				LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																				LCDPanelStruct.POINT6_Sign = OFF;
																		}
																		else
																		{
																				LCDPanelStruct.DisNumber = Current_Credit.CurrCredit * 10;
																				LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																				LCDPanelStruct.POINT6_Sign = ON;
																		}

																		if(Current_Credit.CurrCredit <= CONFIG_Meter.LowCredit)
																		{
																				RefreshMoney();
																		}
																		else
																		{
																				LCDPanelStruct.MONEY_Sign = ON;
																		}

																		LCD_Refresh(&LCDPanelStruct);
																}
																else if(Current_card_type == 3)//校准的时候的AD值
																{
																		LCDPanelStruct.DisNumber = ADC_Data[0];
																		LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																		LCD_Refresh(&LCDPanelStruct);
																}

																LCD_Refresh_Time = 0;
														}
												}

												if(sStep == 3)//画面2
												{
														if(LCD_Refresh_Time < 10)
														{
																LCD_Refresh_Time++;
														}
														else
														{
																RefreshSignal();
																RefreshBatVoltage();
																RefreshValve();

																if(REAL_DATA_PARAM.TankQuality <= CONFIG_Meter.LowGasVolume || REAL_DATA_PARAM.TankQuality <= 0.5)
																{
																		RefreshCylinder();
																}
																//											if(Current_card_type == 3)//校准后的累计流量,长按存储校准数据,并清零此项,便于验证计算
																//											{
																//											  LCDPanelStruct.num = REAL_DATA_Credit.CumulationGasL * 10;
																//												LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																//												LCDPanelStruct.POINT6_Sign = ON;
																//												LCD_Refresh(&LCDPanelStruct);
																//											}
																if(Current_card_type == 0 || Current_card_type == 2 || Current_card_type == 4)//无卡,开锁,重置
																{
																		if(No_valid_Card == true)
																		{
																				strncpy(LCDPanelStruct.DisChar," --02--",7);
																				LCDPanelStruct.DisType = 1;
																				RefreshWarn();
																				LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																		}
																		else
																		{
																				LCDPanelStruct.DisNumber = time;
																				LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																				LCDPanelStruct.POINT1_Sign = ON;
																				LCDPanelStruct.POINT2_Sign = ON;
																				LCDPanelStruct.POINT4_Sign = ON;
																				LCDPanelStruct.POINT5_Sign = ON;
																				LCDPanelStruct.POINT3_Sign = OFF;
																				LCDPanelStruct.POINT6_Sign = OFF;
																				LCDPanelStruct.cylinder_Sign = OFF;
																				LCDPanelStruct.KgSign_Sign = OFF;
																		}
																		LCD_Refresh(&LCDPanelStruct);
																}
																if(Current_card_type == 3)//校准后的电压
																{
																		LCDPanelStruct.DisNumber = ADTestVFlow * 1000;
																		LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																		LCDPanelStruct.POINT3_Sign = ON;
																		LCDPanelStruct.KgSign_Sign = OFF;
																		LCD_Refresh(&LCDPanelStruct);
																}
																if(Current_card_type == 1)//使用的千克数
																{
																		if(strcmp(Current_Credit.CARD_ID, "111111111111111111")==0
																						|| strcmp(Current_Credit.CARD_ID, "222222222222222222")==0
																						|| strcmp(Current_Credit.CARD_ID, "333333333333333333")==0)
																				//|| strcmp(Current_Credit.CARD_ID, "444444444444444444")==0)
																		{
																				LCDPanelStruct.DisNumber = REAL_DATA_Credit.UsedKg * 1000;
																				LCDPanelStruct.POINT3_Sign = ON;
																				LCDPanelStruct.POINT6_Sign = OFF;
																		}
																		else
																		{
																				LCDPanelStruct.DisNumber = REAL_DATA_Credit.UsedKg * 10;
																				LCDPanelStruct.POINT3_Sign = OFF;
																				LCDPanelStruct.POINT6_Sign = ON;
																		}
																		LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																		LCDPanelStruct.KgSign_Sign = ON;
																		LCD_Refresh(&LCDPanelStruct);
																}

																LCD_Refresh_Time = 0;
														}
												}

												if(sStep == 4)//画面3 特殊卡显示电压值
												{
														if(LCD_Refresh_Time < 10)
														{
																LCD_Refresh_Time++;
														}
														else
														{
																RefreshSignal();
																RefreshBatVoltage();
																RefreshValve();

																if(REAL_DATA_PARAM.TankQuality <= CONFIG_Meter.LowGasVolume || REAL_DATA_PARAM.TankQuality <= 0.5)
																{
																		RefreshCylinder();
																}

																if(Current_card_type == 1)
																{
																		//printf("ADTestVFlow %f\r\n",ADTestVFlow);
																		LCDPanelStruct.DisNumber = (ADTestVFlow) * 1000;//电压值 + flow_zero
																		LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																		LCDPanelStruct.POINT3_Sign = ON;
																		LCDPanelStruct.KgSign_Sign = OFF;
																		LCD_Refresh(&LCDPanelStruct);
																}

																LCD_Refresh_Time = 0;
														}
												}

												if(sStep == 5)//画面4 特殊卡显示实时流量
												{
														if(LCD_Refresh_Time < 10)
														{
																LCD_Refresh_Time++;
														}
														else
														{
																RefreshSignal();
																RefreshBatVoltage();
																RefreshValve();

																if(REAL_DATA_PARAM.TankQuality <= CONFIG_Meter.LowGasVolume || REAL_DATA_PARAM.TankQuality <= 0.5)
																{
																		RefreshCylinder();
																}

																if(Current_card_type == 1)
																{
																		//printf("ADTestVFlow %f\r\n",ADTestVFlow);
																		LCDPanelStruct.DisNumber = TmpRealTimeFlow * 1000;//因为校准后的流量既是煤气的体积,这里是实时流量
																		LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																		LCDPanelStruct.POINT3_Sign = ON;
																		LCDPanelStruct.KgSign_Sign = OFF;
																		LCD_Refresh(&LCDPanelStruct);
																}

																LCD_Refresh_Time = 0;
														}
												}
										}
								}
								else if(LCD_Delay_Time == 0)
								{
										sStep = 0;
										//            for(i = 0; i< 32; i++)
										//            {
										//                Write_1621(i,0x0);
										//            }

										LCD_Close();

										LCD_LIGHT(0);
										LCD_POWER(0);

										//					CS_W(0);//对于低功耗影响不大
										//					RD_W(0);
										//					WR_W(0);
										//					DAT_W(0);

										//屏幕灭不测试电压
										IsTestPower= false;
										BATAD_POWER(0);

										IsTestRFID = false;
										IsInitRFID = false;

										Lcd_Status = false;
								}
						}
				}

        osDelay(10);
    }
  /* USER CODE END StartLedTask */
}

/* StartCmdAnalyzTask function */
void StartCmdAnalyzTask(void const * argument)
{
  /* USER CODE BEGIN StartCmdAnalyzTask */
    uint32_t i,cmd_sum;
    int32_t rec_arg_num;
    char cmd_buf[CMD_LEN];
    HAL_UART_Receive_IT(&huart3,&Usart3RxTemp,1);
    /* Infinite loop */
    while(1)
    {
        uint32_t rec_num;

        ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
        //printf("StartCmdAnalyzTask\r\n");
        rec_num=get_true_char_stream(cmd_analyze.processed_buf,cmd_analyze.rec_buf);

        /*从接收数据中提取命令*/
        for(i=0; i<CMD_LEN; i++)
        {
            if((i>0)&&((cmd_analyze.processed_buf[i]==' ')||(cmd_analyze.processed_buf[i]==0x0D)))
            {
                cmd_buf[i]='\0';        //字符串结束符
                break;
            }
            else
            {
                cmd_buf[i]=cmd_analyze.processed_buf[i];
            }
        }

        rec_arg_num=cmd_arg_analyze(&cmd_analyze.processed_buf[i],rec_num);

        cmd_sum = sizeof(cmd_list)/sizeof(cmd_list[0]);

        for(i=0; i<cmd_sum; i++)
        {
            if(strcmp(cmd_buf,cmd_list[i].cmd_name)==0)       //字符串相等
            {
                if(rec_arg_num<0 || rec_arg_num>cmd_list[i].max_args)
                {
                    printf("arg_num more :%d\r\n",rec_arg_num);
                }
                else
                {
                    LCD_Delay_Time = 10 * 100;
                    cmd_list[i].handle(rec_arg_num,(void *)cmd_analyze.cmd_arg);
                }
                break;
            }

        }
        if(i>=sizeof(cmd_list)/sizeof(cmd_list[0]))
        {
            printf("not find cmd: %s\r\n",cmd_buf);
        }

        osDelay(25);
    }
  /* USER CODE END StartCmdAnalyzTask */
}

/* Sim80xBuffTask function */
//curl -X GET "https://virtserver.swaggerhub.com/KopaTechnology/KTMeterControl/0.0.1/meter/KE000000012/command"
//https://ateei9d448.execute-api.eu-west-1.amazonaws.com/testing/meter/settings/TZ00000525
#define SEVER_URL "https://ateei9d448.execute-api.eu-west-1.amazonaws.com/"
#define SEVER_VERSION "testing/"

#define M26GETCOMMANDLEN 13
#define M26POSTCOMMANDLEN 15

typedef uint8_t (*Cmd_Analysis)(char *pCmd);

//add by lwl
typedef struct SeverInfo
{
	char *Sendsever; //http://virtserver.swaggerhub.com/KopaTechnology/KTMeterControl/
	char *MeterId;	//meter/{meterId}/
	char *SeverVer;//1.0.0/
	char *CardID;//card/{cardId}/
//	char Type[]
} Stru_Sever_Info_t;
//add by lwl
typedef struct ATInfo{
	uint8_t  u8CmdNum;
	char     *SendCommand;
//	char     ReceiveAnswer[128];
	Cmd_Analysis pFun;
}stru_P4_command_t;


stru_P4_command_t Send_AT_cmd[]={
	          //  u8CmdNum  SendCommand																			pFun
/*0*/			{     	1,			 "AT\r\n",																	Analysis_AT_Cmd					},
/*1*/			{     	2,			 "AT+CSQ\r\n",															Analysis_CSQ_Cmd				},
/*2*/			{     	3,			 "AT+QIREGAPP\r\n",													Analysis_QIREGAPP_Cmd		},
/*3*/			{     	4,			 "AT+QIACT\r\n",														Analysis_QIACT_Cmd			},
/*4*/			{     	5,			 "AT+QILOCIP\r\n",													Analysis_QILOCIP_Cmd		},
/*5*/			{     	6,			 "AT+QSSLCFG=\"sni\",0,1\r\n",							Analysis_SNI_Cmd				},
/*6*/			{     	7,			 "AT+QSSLCFG=\"https\",1\r\n",							Analysis_QSSLCFG_Cmd		},
/*7*/			{     	8,			 "AT+QSSLCFG=\"httpsctxi\",0\r\n",					Analysis_QSSLCFG_Cmd		},
/*8*/			{     	9,			 NULL											,									Analysis_QHTTPURL_Cmd		}, //AT+QHTTPURL=88,60
/*9*/			{     	10,			 NULL,																			Analysis_SEVER_Addr_Cmd	},
/*10*/		{     	11,			 "AT+QHTTPGET=60,120\r\n",									Analysis_QHTTPGET_Cmd		},  //GET请求
/*11*/		{     	12,			 "AT+QHTTPREAD\r\n",												Analysis_QHTTPREAD_Cmd	},
/*12*/		{     	13,			 "AT+QIDEACT\r\n",													Analysis_QIDEACT_Cmd		},
/*13*/		{     	14,			 "at+qhttpcfg=\"CONTENT-TYPE\",\"application/json\"\r\n",					Analysis_QSSLCFG_Cmd		},//设置JSON格式
/*14*/		{     	15,			 NULL,																			Analysis_QHTTPPOST_Cmd		},//AT+QHTTPPOST=272\r\n
/*15*/		{     	16,			 NULL,																			Analysis_POSTDATA_Cmd		}, //POST 指令携带的数据
};


uint8_t u8GetNum[M26GETCOMMANDLEN]= {0,1,2,3,4,5,6,7,8,9,10,11,12};
uint8_t u8PostNum[M26POSTCOMMANDLEN] = {0,1,2,3,4,5,6,7,8,9,13,14,15,11,12};
uint8_t u8SniNum[3] = {0,1,5};

//type:command,parameters,cookingSession,hardware,info,warning
static char * Sever_Address_GET(const Stru_Sever_Info_t* SeverInfo,char* type)
{
	uint8_t u8Lenth = 0;
	char* ptUrlInfo;
//	u8Lenth = strlen(type);

	u8Lenth = strlen(SeverInfo->Sendsever)+strlen(SeverInfo->SeverVer)+strlen(SeverInfo->MeterId)+strlen(SeverInfo->CardID)+strlen(type)+10;

	ptUrlInfo = (char *) malloc(u8Lenth);
	memset(ptUrlInfo,0,u8Lenth);
	printf("malloc end\r\n");
	u8Lenth = 0;
	u8Lenth = strlen(SeverInfo->Sendsever);
	if(u8Lenth != 0)
	{
		strcat(ptUrlInfo,SeverInfo->Sendsever);
	}
	//版本号拼接
	u8Lenth = 0;
	u8Lenth = strlen(SeverInfo->SeverVer);
	if(u8Lenth != 0)
	{
		strcat(ptUrlInfo,SeverInfo->SeverVer);
	}
	
	u8Lenth = 0;
	u8Lenth = strlen(SeverInfo->MeterId);
	if(u8Lenth != 0)
	{
		strcat(ptUrlInfo,SeverInfo->MeterId);
	}
	
	u8Lenth = 0;
	u8Lenth = strlen(SeverInfo->CardID);
	if(u8Lenth != 0)
	{
		strcat(ptUrlInfo,SeverInfo->CardID);
	}
	
	u8Lenth = 0;
	u8Lenth = strlen(type);
	if(u8Lenth != 0)
	{
		strcat(ptUrlInfo,type);
	}
	
	strcat(ptUrlInfo,"\r\n");
	
	return ptUrlInfo;
}
static char * Post_Data_Cmd(char *postdata)
{
	char* ptPostInfo;
	uint16_t u16Lenth = 0;
	u16Lenth = strlen(postdata)+10;
	ptPostInfo = (char *) malloc(u16Lenth);
	
	u16Lenth = 0;
	u16Lenth = strlen(postdata);
	if(u16Lenth != 0)
	{
		strcat(ptPostInfo,postdata);
	}
	strcat(ptPostInfo,"\r\n");
}

QueueHandle_t SendATQueue = NULL;
SemaphoreHandle_t  Semaphore_Uart_Rec = NULL;
void AppObjCreateQueue (void)
{
	/* 创建10个uint8_t型消息队列 */
	SendATQueue = xQueueCreate(1, sizeof(uint8_t));
    if( SendATQueue == 0 )
    {
        printf("create failed\r\n");
    }
}
void AppObjCreate (void)
{
	/* 创建二值信号量，首次创建信号量计数值是0 */
	Semaphore_Uart_Rec = xSemaphoreCreateBinary();
	
	if(Semaphore_Uart_Rec == NULL)
	{
		printf("Semaphore creat failed!\r\n");
			/* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
	}
}
//该函数用于实现对发送URL（sever）长度的计算，并将计算结果添充到Send_AT_cmd[7].SendCommand
static void UrlLength(uint16_t length)
{
	char *ptUrlCharLength,chUrl[20]="AT+QHTTPURL=";
	char str;
	uint16_t i ;
	ptUrlCharLength = (char *) malloc(length+strlen(chUrl));
	itoa(length,ptUrlCharLength,10); 
	strcat(chUrl,ptUrlCharLength);
	strcat(chUrl,",60\r\n");
	for(i =0;i<strlen(chUrl);i++)
	{
		str = chUrl[i];
		Send_AT_cmd[8].SendCommand[i] = str;
	}
}
//AT+QHTTPURL 9
//AT+QHTTPPOST  15
static void CmdLength(uint16_t urllength,uint8_t cmd_num)
{
	char ptUrlCharLength[4],chUrl[20];
	char str;
	uint16_t i ;
//	ptUrlCharLength = (char *) malloc(urllength+strlen(chUrl));
	itoa(urllength,ptUrlCharLength,10); 
	if(cmd_num == Send_AT_cmd[8].u8CmdNum)  //如果需要填充AT+QHTTPURL指令
	{
		strcpy(chUrl,"AT+QHTTPURL=");
		strcat(chUrl,ptUrlCharLength);
//		strcat(chUrl,",60\r\n");
		strcat(chUrl,"\r\n");
		for(i =0;i<strlen(chUrl);i++)
		{
			str = chUrl[i];
			Send_AT_cmd[8].SendCommand[i] = str;
		}
	}

	if(cmd_num == Send_AT_cmd[14].u8CmdNum)  //如果需要填充AT+QHTTPPOST=272
	{
		strcpy(chUrl,"AT+QHTTPPOST=");
		strcat(chUrl,ptUrlCharLength);
		strcat(chUrl,"\r\n");
		for(i =0;i<strlen(chUrl);i++)
		{
			str = chUrl[i];
			Send_AT_cmd[14].SendCommand[i] = str;
		}
	}	

}
static void SendGetCommand()
{
	uint8_t i = 0;
//	for(i=0;i< sizeof(Send_AT_cmd)/sizeof(Send_AT_cmd[0]);i++)M26GETCOMMANDLEN
	for(i=0;i< M26GETCOMMANDLEN;i++)
	{
		if((u8GetNum[i]!=5))
		{
			printf("send:%s\r\n\r\n",Send_AT_cmd[u8GetNum[i]].SendCommand);
			Sim80x.AtCommand.FindAnswer = 0;
			xQueueSend(SendATQueue,(void *) &Send_AT_cmd[u8GetNum[i]].u8CmdNum,(TickType_t)10);	 
			Sim80x_SendAtCommand(Send_AT_cmd[u8GetNum[i]].SendCommand,1000,1,"AT\r\r\nOK\r\n");
			osDelay(2000);
			while(!Sim80x.AtCommand.FindAnswer)
			{
				Sim80x_SendAtCommand(Send_AT_cmd[u8GetNum[i]].SendCommand,1000,1,"OK\r\n");
				osDelay(2000);
			}
		}
		
	}
}

static void SendPostCommand()
{
	uint8_t i = 0;
	uint8_t u8Lenth = 0;
//	for(i=0;i< sizeof(Send_AT_cmd)/sizeof(Send_AT_cmd[0]);i++)M26GETCOMMANDLEN
	for(i=0;i< M26POSTCOMMANDLEN;i++)
	{
		if((u8PostNum[i]!=5))
		{
			printf("send:%s\r\n\r\n",Send_AT_cmd[u8PostNum[i]].SendCommand);
			Sim80x.AtCommand.FindAnswer = 0;
			xQueueSend(SendATQueue,(void *) &Send_AT_cmd[u8PostNum[i]].u8CmdNum,(TickType_t)10);	 
			Sim80x_SendAtCommand(Send_AT_cmd[u8PostNum[i]].SendCommand,1000,1,"AT\r\r\nOK\r\n");
			osDelay(2000);
			while(!Sim80x.AtCommand.FindAnswer)
			{
//				if(u8PostNum[i]!= 15)
				{
					Sim80x_SendAtCommand(Send_AT_cmd[u8PostNum[i]].SendCommand,1000,1,"OK\r\n");
					osDelay(2000);
				}				
			}
		}
//		osDelay(5000);
	}
}

static void M26_Sni_Init(void )
{
	uint8_t i = 0;
	for(i = 0;i < 3;i++)
	{
		{
			printf("send %s\r\n\r\n",Send_AT_cmd[u8SniNum[i]].SendCommand);
			Sim80x.AtCommand.FindAnswer = 0;
			xQueueSend(SendATQueue,(void *) &Send_AT_cmd[u8SniNum[i]].u8CmdNum,(TickType_t)10);	 
			Sim80x_SendAtCommand(Send_AT_cmd[u8SniNum[i]].SendCommand,1000,1,"AT\r\r\nOK\r\n");
			osDelay(2000);
			while(!Sim80x.AtCommand.FindAnswer)
			{
				{
					Sim80x_SendAtCommand(Send_AT_cmd[u8SniNum[i]].SendCommand,1000,1,"OK\r\n");
					osDelay(2000);
				}				
			}
		}		
	}
}


void M26test_Task(void const * argument)
{
	Stru_Sever_Info_t *struSeverInfo;
	uint8_t result = 0 , i = 0; //于用标识，是否响应了当前的指令
	uint32_t count_times = 1;
	char *ptUrl,*ptPost;
	char *ptPostData;
	volatile uint16_t u8UrlLength = 0;
	Send_AT_cmd[8].SendCommand =(char *)malloc(20);
	Send_AT_cmd[14].SendCommand =(char *)malloc(20);
	struSeverInfo = (struct SeverInfo *) malloc(sizeof(struct SeverInfo));
//	struSeverInfo->CardID = (char *) malloc(sizeof(char)*50);
//	struSeverInfo->MeterId = (char *) malloc(sizeof(char)*50);
//	struSeverInfo->Sendsever = (char *) malloc(strlen(SEVER_URL)+10);
//	struSeverInfo->SeverVer = (char *) malloc(sizeof(char)*10);
	HAL_GPIO_WritePin(_SIM80X_POWER_KEY_GPIO,_SIM80X_POWER_KEY_PIN,GPIO_PIN_RESET);
						
	HAL_GPIO_WritePin(_SIM80X_POWER_KEY_GPIO,_SIM80X_POWER_KEY_PIN,GPIO_PIN_SET);
	HAL_UART_Receive_IT(&_SIM80X_USART,&Sim80x.UsartRxTemp,1);
	printf("sni init\r\n");
	M26_Sni_Init();
	while(1)
	{
		struSeverInfo->Sendsever = SEVER_URL;
		u8UrlLength = strlen(struSeverInfo->Sendsever);
		struSeverInfo->SeverVer = SEVER_VERSION;
		struSeverInfo->CardID = "";

		printf("************************************************************************************************\r\n");
		printf("test times %d\r\n",count_times);
		count_times++;
		//get command  curl -X GET "https://ateei9d448.execute-api.eu-west-1.amazonaws.com/testing/meter/command/TZ00000525"
//		printf("--------------------------------------------------------------------------------------------------");
//		struSeverInfo->MeterId = "meter/command/TZ00000525";
//		ptUrl = Sever_Address_GET( struSeverInfo,"");
//		Send_AT_cmd[9].SendCommand = ptUrl;
//		u8UrlLength = strlen(ptUrl)-2;
////		UrlLength(u8UrlLength);
//		CmdLength(u8UrlLength,9);  //根据发送URL的长度
//		SendGetCommand();
//		free(ptUrl);		
//		printf("--------------------------------------------------------------------------------------------------");
		printf("--------------------------------------------------------------------------------------------------");
		//get settings  curl -X GET "https://ateei9d448.execute-api.eu-west-1.amazonaws.com/testing/meter/settings/TZ00000525"
		struSeverInfo->MeterId = "meter/settings/TZ00000525";
		ptUrl = Sever_Address_GET( struSeverInfo,"");
		printf("Sever_Address_GET\r\n");
		Send_AT_cmd[9].SendCommand = ptUrl;
		u8UrlLength = strlen(ptUrl)-2;
//		UrlLength(u8UrlLength);
		CmdLength(u8UrlLength,9);  //根据发送URL的长度
		SendGetCommand();
		free(ptUrl);
		printf("--------------------------------------------------------------------------------------------------");
		printf("--------------------------------------------------------------------------------------------------");
		//get firmware  curl -X GET "https://ateei9d448.execute-api.eu-west-1.amazonaws.com/testing/meter/firmware/TZ00000525"
		struSeverInfo->MeterId = "meter/firmware/TZ00000525";
		ptUrl = Sever_Address_GET( struSeverInfo,"");
		printf("Sever_Address_GET\r\n");
		Send_AT_cmd[9].SendCommand = ptUrl;
		u8UrlLength = strlen(ptUrl)-2;
//		UrlLength(u8UrlLength);
		CmdLength(u8UrlLength,9);  //根据发送URL的长度
		SendGetCommand();
		free(ptUrl);
		printf("--------------------------------------------------------------------------------------------------");
		
		printf("--------------------------------------------------------------------------------------------------");
		// post  cookingSession   https://ateei9d448.execute-api.eu-west-1.amazonaws.com/testing/meter/cookingSession/TZ00000235/9088450934850394385
		struSeverInfo->MeterId = "meter/cookingSession/KE1000000012/9088450934850394385";
		ptPostData = "{\"cookingSessionId\":\"xxxxxxxxxxxxxxx\",\
		\"startTime\":\"2016-08-29T09:12:33.001Z\",\
		\"endTime\":\"2016-08-29T09:12:33.001Z\",\
		\"endReason\":2,\
		\"endCumulativeMass\":105.97,\
		\"startCumulativeMass\":100.9,\
		\"gasConsumed\":4,\
		\"startCredit\":34,\
		\"endCredit\":22,\
		\"creditConsumed\":12,\
		\"gasRemaining\":12000\
		}";		
		ptUrl = Sever_Address_GET( struSeverInfo,"");
		printf("Sever_Address_GET\r\n");
		Send_AT_cmd[9].SendCommand = ptUrl;
		u8UrlLength = strlen(ptUrl)-2;
		CmdLength(u8UrlLength,9);  //根据发送URL的长度		
		ptPost = Post_Data_Cmd( ptPostData);
		Send_AT_cmd[15].SendCommand = ptPost;
		u8UrlLength = strlen(ptPost)-2;
		CmdLength(u8UrlLength,15);  //根据发送POST的长度
		SendPostCommand();
		free(ptUrl);
		free(ptPost);
		
		printf("--------------------------------------------------------------------------------------------------");
		printf("--------------------------------------------------------------------------------------------------");
		//post settings   https://ateei9d448.execute-api.eu-west-1.amazonaws.com/testing/meter/settings/TZ00000235
		struSeverInfo->MeterId = "meter/settings/TZ00000235";
		ptPostData = "{\"command\": \"STUP\",\
		\"serverIPaddress\": \"198.51.100.42\",\
		\"serverPort\": 5070,\
		\"gasLevel\": 9800,\
		\"dataUploadPeriod\": 360,\
		\"warningLowBatteryVoltage\": 4.5,\
		\"warningLowCreditBalance\": 80,\
		\"warningLowGasVolumeAlarm\": 2000,\
		\"metercurrency\": \"KES\",\
		\"uploadFrequency\": 360,\
		\"uploadTime\": 0,\
		\"sensorSlope\": 2.3,\
		\"sensor:Intercept\": 0.1,\
		\"infoDatestamp\": \"2016-08-29T09:12:33.001Z\"\
		}";		
		ptUrl = Sever_Address_GET( struSeverInfo,"");
		printf("Sever_Address_GET\r\n");
		Send_AT_cmd[9].SendCommand = ptUrl;
		u8UrlLength = strlen(ptUrl)-2;
		CmdLength(u8UrlLength,9);  //根据发送URL的长度		
		ptPost = Post_Data_Cmd( ptPostData);
		Send_AT_cmd[15].SendCommand = ptPost;
		u8UrlLength = strlen(ptPost)-2;
		CmdLength(u8UrlLength,15);  //根据发送POST的长度
		SendPostCommand();
		free(ptUrl);
		free(ptPost);
		printf("--------------------------------------------------------------------------------------------------");
		printf("--------------------------------------------------------------------------------------------------");
		
		//post status  https://ateei9d448.execute-api.eu-west-1.amazonaws.com/testing/meter/status/TZ00000235
		struSeverInfo->MeterId = "meter/status/TZ00000235";
		ptPostData = "{\"batteryVoltage\": 2,\
		\"gasTemperature\": 15,\
	\"tankLockStatus\": true,\
	\"tankSensorStatus\": false,\
	\"gsmSignalIntensity\": 25,\
	\"needleSensorStatus\": false,\
	\"lidLightSensorStatus\": false\
	\"electronicValveStatus\": false,\
	\"lidElectricLockStatus\": false\
	}";		
		ptUrl = Sever_Address_GET( struSeverInfo,"");
		printf("Sever_Address_GET\r\n");
		Send_AT_cmd[9].SendCommand = ptUrl;
		u8UrlLength = strlen(ptUrl)-2;
		CmdLength(u8UrlLength,9);  //根据发送URL的长度		
		ptPost = Post_Data_Cmd( ptPostData);
		Send_AT_cmd[15].SendCommand = ptPost;
		u8UrlLength = strlen(ptPost)-2;
		CmdLength(u8UrlLength,15);  //根据发送POST的长度
		SendPostCommand();
		free(ptUrl);
		free(ptPost);		
		printf("--------------------------------------------------------------------------------------------------");

	}
}



//######################################################################################################################

uint8_t Analysis_AT_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	return 0;
}

uint8_t Analysis_POSTDATA_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		return 1;
	}
	return 0;
}

uint8_t Analysis_CSQ_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		ptFindResult = strstr(ptStrStart,"CSQ");
		if(ptFindResult != NULL)
		{
			if(!(strstr(ptStrStart,"99") || strstr(ptStrStart,"0,0")))
				return 1;
		}
	}
	
	return 0;
}

uint8_t Analysis_QIREGAPP_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	return 0;
}

uint8_t Analysis_QIACT_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	return 0;
}

uint8_t Analysis_QILOCIP_Cmd(char *pdata)
{
	return 1;
}

uint8_t Analysis_SEVER_Addr_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		return 1;
	}
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		return 1;
	}		
	return 0;
}

uint8_t Analysis_SNI_Cmd(char *pdata)
{
	return 1;
}

uint8_t Analysis_QHTTPPOST_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"CONNECT");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	return 0;
}

uint8_t Analysis_QSSLCFG_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	return 0;
}

uint8_t Analysis_QHTTPURL_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"CONNECT");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	return 0;
}

uint8_t Analysis_QHTTPGET_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	return 0;
}

uint8_t Analysis_QHTTPREAD_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	ptFindResult = strstr(ptStrStart,"ERROR");
	if(ptFindResult != NULL)
	{
		return 1;
	}	
	return 0;
}

uint8_t Analysis_QIDEACT_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	static uint8_t u8ErrorCnt = 0;
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"OK");
	if((ptFindResult != NULL) || (u8ErrorCnt == 10))
	{
		return 1;
	}
	u8ErrorCnt ++;		

	return 0;
}

void  Sim80x_BufferProcess(void)
{
	char      *strStart,*str1,*str2;
	int32_t   tmp_int32_t;
	BaseType_t xResult;
	uint8_t u8ATNum=0,u8AnalysisResult = 0;

//    strStart = (char*)&Sim80x.UsartRxBuffer[0];
	while(1)
	{
		xResult = xSemaphoreTake(Semaphore_Uart_Rec, (TickType_t)portMAX_DELAY);
		if(xResult == pdTRUE)
		{
			xQueueReceive(SendATQueue, (void *)&u8ATNum, (TickType_t)0);
			if(u8ATNum != 0)
			{
				printf("AT_NO.=%d,rec:%s\r\n",u8ATNum,&Sim80x.UsartRxBuffer[0]);
				u8AnalysisResult=Send_AT_cmd[u8ATNum-1].pFun(NULL);
				if(u8AnalysisResult != 0)
					Sim80x.AtCommand.FindAnswer = 1;
				
			}
			memset(Sim80x.UsartRxBuffer,0,_SIM80X_BUFFER_SIZE);
			Sim80x.UsartRxIndex = 0;
			Sim80x.Status.Busy=0;
		}
		
	}
 
	
	
    //##################################################
    //---       Buffer Process
    //##################################################
//#if (_SIM80X_DEBUG==2)
//    printf("%s",strStart);
//#endif
    Sim80x.UsartRxIndex=0;
//    memset(Sim80x.UsartRxBuffer,0,_SIM80X_BUFFER_SIZE);
    
}

void Sim80xBuffTask(void const * argument)
{
  /* USER CODE BEGIN Sim80xBuffTask */
    /* Infinite loop */
    while(1)
    {
        //ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
//        if(((Sim80x.UsartRxIndex>4) && (HAL_GetTick()-Sim80x.UsartRxLastTime > 50)))
        {
            Sim80x.BufferStartTime = HAL_GetTick();
            Sim80x_BufferProcess();
            Sim80x.BufferExeTime = HAL_GetTick()-Sim80x.BufferStartTime;
        }

        osDelay(10);
    }
  /* USER CODE END Sim80xBuffTask */
}

/* USER CODE BEGIN Application */
void IntoLowPower()
{
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
