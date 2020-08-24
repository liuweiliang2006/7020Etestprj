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
#include "hextochar.h"
#include "chartohex.h"
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
uint8_t gkeySum = 0;     //��ǰ���������λ��

//������Ϣ
bool No_valid_Card = false;//��Ч��,Ĭ�ϼ� ������Ч
volatile bool IsNeedRestart = false;
bool IsNeedTimeRestart = false;  //�Ƿ�ʱ����

//log
uint32_t logStart = 0;
uint32_t logEnd = 0;
bool IsNeedReportLog = false;  //�Ƿ񵼳���־

uint32_t VoltageStart = 0;
uint32_t VoltageEnd = 0;
bool IsNeedReportVoltage  = false;

//��Ļ�й�
volatile bool gkeyValue = false;   //���ݵ���Ļ����ֵ
uint8_t sStep = 0;       //��ǰ��Ļ��ֵ
uint32_t LCD_Delay_Time  = 0;    //��ʾ��ʱ��
bool Lcd_Status = false; //��Ļ״̬Ĭ�Ϲ�

LogTypeDef_t LogTypeDef;

//���������
//bool lockValue = false;   //��΢����ֵ,��������true,������Ļ������������
bool LidStatus = false;   //���ӵ���ʵ״̬
bool NEEDLEStatus = false;   //�뷧����ʵ״̬
uint32_t OpenLockTime = 0;    //������ʱ,��ÿ�η��Ϳ���ָ���,���ָ��ʱ����û�в���,�Զ�����

//�����������
volatile int16_t connectStep = -1;      //���ӵĲ���,ֹͣ�����ӵ�����,ÿ�ɹ�һ����һ -1Ϊ����֮ǰ

//���ͱ������
bool IsHaveSimError = false;
bool IsNeedTimeing = false;
bool IsSendHalfTime = false;//�Ƿ��Ѿ�������һ��Ķ�ʱ��
//uint8_t TimeRetryNumber = 0;

uint8_t HearRetryNumber = 0; //��û��������Ҫ������beat�ķ��ʹ���
bool IsNeedWarning = false;
uint8_t WarnRetryNumber = 0;
bool IsNeedCustomerCreditResponse = false;
bool IsNeedInformationResponse = false;
bool IsNeedSendCook = false; //���극�ϱ�

bool IsSendedCSRP = false; //�Ƿ��͹��ڵȴ��ظ�
bool IsReceivedCSRPReply = false; //�Ƿ���յ��ظ�

bool IsNeedIllegal_lid_opening = false;//�Ƿ���Ҫ�������Ǳ���,Ĭ���Ǵ�����,ֻ�е�ˢ��,����֮��Ͳ�������
uint32_t WarnTime = 0;//��һ�α�����ʱ�� //���ڵ͹��ĵ�ʱ��,���ж�ʱ�����в�����,���������ʱ���Ĺ��ܸ���RTC����

bool IsLowPowerWarned = false;

uint32_t TimeForPowerStart=0;  //��ʼ�ϵ��ʱ��
uint32_t TimeForCurrStart=0;   //��ǰ���迪ʼ���е�ʱ��
uint32_t loopSumCurr=0;        //ѭ�����ԵĴ���
uint32_t loopSumPDP=0;         //ѭ�����ԵĴ��� ���ƶ�����������,���ӵ�ʱ����ܲ��ɹ�,���ʱ��Ҫ��ǰ������,���п���һֱ���ɹ�,������Ҫ���ӱ���
//////////////////////////////////////////////////////////////////
//bool IstefffmpSended = false;//�Ƿ��Ѿ�������
//extern uint32_t sumCnt;
//extern uint16_t identCnt;

//ˢ�����
volatile uint8_t Current_card_type = 0;  //��ǰ��������

//��Ҫ�ظ����������
volatile uint8_t commandType = 0;

//char* updata = "123456";
//char updata2[7];
//volatile uint32_t gGasCnt = 0;

//�洢�Ľṹ��
CONFIG_GPRS_t CONFIG_GPRS;           //����������Ϣ
bool IsSaveCONFIG_GPRS = false;

CONFIG_Meter_t CONFIG_Meter;				 //�Ǳ��һЩӲ����Ϣ
bool IsSaveCONFIG_Meter = false;

REAL_DATA_PARAM_t REAL_DATA_PARAM;   //�ź�ǿ�ȵ�
bool IsSaveREAL_DATA_PARAM = false;

REAL_DATA_Credit_t REAL_DATA_Credit; //�Ǳ��ֵ��ֵ��Ϣ
bool IsSaveREAL_DATA_Credit = false;

Current_Credit_t Current_Credit;
Current_Credit_t Tmp_Credit;

Current_Log_t Current_Log;
bool IsSaveREAL_Current_Log = false;

Current_Voltage_t Current_Voltage;
bool IsSaveREAL_Current_Voltage = false;

float StartCredit = 0; //ÿ��������ʼ������
tim_t StartCookingSeconds; //ÿ��������ʼ��ʱ��
volatile bool IsStartCooking5 = false; //�������������Ҵ���5����


REAL_DATA_CALIBRATION_t REAL_DATA_CALIBRATION;
bool IsSaveCALIBRATION = false;
bool IsStartCALIBRATION = false;

REAL_Flow_CALIBRATION_t REAL_Flow_CALIBRATION;
bool IsSaveFlowCALIBRATION = false;

update_t update;
bool IsSaveUpdate = false;

//�������
key_Status_t key1_Status;						//��ǰ�����״̬  �����ش���  ����Ϊ��
key_Status_t Key2_Status;				    //��������״̬    �����ش���  ����״̬Ϊ��  ����б������Ϊ�̵����Ͽ�,�ɵ͵���,�����ش���
key_Status_t Key3_Status;				    //�뷧��״̬      �½��ش���  ����״̬Ϊ��  ���¼̵���,���Ǵ��뷧Ϊ��,����

bool haveClickButton = false;            //�Ƿ���ֵ����¼�
bool haveLongButton = false;             //�Ƿ���ֳ����¼�

key_Status_t OpenLid_Status;			  //��紫������״̬,���ŵ�ʱ��Ϊ0,����Ϊ1,�����ش���

//����ʱ����
volatile uint32_t zeroFlowTime = 0;//�������������ʱ��ÿһ������һ,������һ����ʱ,�رյ�ŷ�
volatile uint32_t StartRunTime = 0;//ÿ�δӵ͹��Ļ���������,ֻ�д���2������ٴν���͹���
volatile bool IsIntoLowPower = false;//�ǽ���͹�����,���ʱ�������߽���͹��ĵ�����,���ɼ�����

volatile uint32_t vibrationsNumber = 0;//���ڼ���Ǳ������Ŀ

//�������û���
extern cmd_analyze_struct cmd_analyze;
extern uint32_t rec_count;
//��Ļ�ṹ��
extern LCDPanel_TypeDef LCDPanelStruct;

//AD������
extern bool IsTestPower;//�Ƿ����˵�Դ����
extern bool IsTestFlow;//�Ƿ�������������

extern bool IsTestEnd;//���������Ƿ����

extern uint32_t ADC_Data[4];


extern float volt1;
extern float	y1;

extern float vert2;
extern float	y2;

extern float ADTestVFlow;
//extern float flow_zero;//����������ֵ,�ڷ��Źرյ�״̬�ɼ�

extern float TmpRealTimeFlow; //У׼��ʱ������ѹ,ʵ���õ�ʱ����ʵʱ����
extern float rechargeCredit;

//extern bool IsReadVoltage;

//uint8_t t_W25Q64[] = "132486";
//uint8_t r_W25Q64[7];
//extern TestStatus TransferStatus1;

/*�����*/
const cmd_list_struct cmd_list[]= {
    /* ����    					������Ŀ    	������        ������Ϣ                         */
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
void Sim7020Etest_Task(void const * argument);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */
void IntoLowPower();
/* USER CODE END FunctionPrototypes */

/* Hook prototypes */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
    //�����ڴ�ĳ�ʼֵ,�Լ���λ���

    //��������ֵ,�Ժ�Ͳ��ù���
    key1_Status.index_input = 1;
    key1_Status.key_m = key_state_0;
    key1_Status.key_time_1 = 0;
    key1_Status.key_state = key_state_0;
    key1_Status.key_time = 0;

    //���ǵ�΢��
    Key2_Status.index_input = 2;
    Key2_Status.key_m = key_state_0;
    Key2_Status.key_time_1 = 0;
    Key2_Status.key_state = key_state_0;
    Key2_Status.key_time = 0;

    //�뷧��΢��OpenLid_Status
    Key3_Status.index_input = 3;
    Key3_Status.key_m = key_state_0;
    Key3_Status.key_time_1 = 0;
    Key3_Status.key_state = key_state_0;
    Key3_Status.key_time = 0;

    //���������״̬OpenLid_Status
		OpenLid_Status.index_input = 4;
		OpenLid_Status.key_m = key_state_0;
		OpenLid_Status.key_time_1 = 0;
		OpenLid_Status.key_state = key_state_0;
		OpenLid_Status.key_time = 0;

    //��ʼ����ʱ�����ó�����ṹ�帳��ֵ
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
    CONFIG_Meter.UpDuty = 360;// ���� 24Сʱ
    CONFIG_Meter.StartDuty = 180;// ���� 24Сʱ
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

    LidStatus = (Opening_input == 1 ? true : false);         //���ӵ���ʵ״̬
    NEEDLEStatus = (NEEDLE_input == GPIO_PIN_RESET ? true : false);;   //�뷧����ʵ״̬

    strcpy(REAL_DATA_PARAM.LidSensorStatus,Opening_input == 1 ? "1" : "0");
    strcpy(REAL_DATA_PARAM.LidElectricLock,"0");//δ��,���ǿ�
    strcpy(REAL_DATA_PARAM.NEEDLESensorStatus,NEEDLE_input == GPIO_PIN_SET ? "0" : "1");
    strcpy(REAL_DATA_PARAM.ElectricValveStatus,"0");//�����ǹ��ŵ�
    strcpy(REAL_DATA_PARAM.TankSensorStatus,"1");//����
    strcpy(REAL_DATA_PARAM.TankLockStatus,"0");//���ǹرյ�

    REAL_DATA_PARAM.TankQuality = 13;//��λǧ��

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

    LL_VCC(1);   //�洢���ϵ粢Ū�ɿ���д��״̬

    ENBOOST_PWR(1); //6.5����ѹ,��������,��ŷ����� 1.1mA
    GPRS_PWR(1);

    //�������Ե�ѹ,������Ե�ѹ,�Լ���Ļ����ʱ����Ե�ѹ
    BATAD_POWER(1);	//0.05mA
    IsTestPower = true;

    HAL_Delay(100);
//
//	__HAL_RCC_SPI2_CLK_DISABLE();

    //printf("SPI_read_MB85RS16A %d\r\n",SPI_read_MB85RS16A(0));
    //�����ʼ�����ֽ�Ϊ22,˵�����ǵ�һ��������
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

    MX_RTC_Init();//ÿ���Զ����ɵĲ�����Ҫ�ֹ�ע�͵�

    HAL_NVIC_SetPriority(RTC_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(RTC_IRQn);

    //û�й����ڼ�,˵���й�������,�������Ź�
    if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
    {
        //MX_IWDG_Init();//ÿ���Զ����ɺ��Ժ���Ҫע�͵���һ��,�����������Ź��Ĵ���,�������������27��,��Ҫ�رտ��Ź�
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
    CALIBRATION_Voltage_READ();//��У׼��ѹ

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

    CALIBRATION_Flow_READ();//������У׼

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

    //���÷���״̬
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

        gkeyValue = true;//����������,����Ļ
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
    if(REAL_DATA_Credit.CardTotal > 0)//����ڴ����п�,�ȴ��ڴ���Ѱ��
    {
        while(TmpIndex < REAL_DATA_Credit.CardTotal)
        {
            memset(&Tmp_Credit,0,sizeof(Current_Credit_t));
            Current_Credit_READ(TmpIndex);//ˢ����ʱ�򰤸���ȡ�ڴ��еĿ���Ϣ,���ȶ�
            //���������Ŀ�Ƭ��Ϣ��ӡ����
            printf("CUSTOMER_ID %s\r\n",Tmp_Credit.CUSTOMER_ID);
            printf("CUSTOMER_ID.length %d\r\n",Tmp_Credit.CUSTOMERNoLength);
            printf("CARD_ID %s\r\n",Tmp_Credit.CARD_ID);
            printf("CARD_ID.length %d\r\n",Tmp_Credit.CARDNoLength);
            printf("CurrCredit %f\r\n",Tmp_Credit.CurrCredit);

            //ÿ�ζ��ж��Ƿ��뵱ǰ������Ϣһ��,�����һ�°��յ�ǰ��Ƭ��,����д����ڴ�
            //����������Ƿ�ֹ���ڴ�дʧ��,����������������,����������˵��Թ����ж�ʧ������ʧ����
            //�����ֵ�˵�����û��ʹ��,����û��ˢ�µ�ǰ��,���ֵ��������Ͷ�ʧ��
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

	osThreadDef(Sim7020Etest, Sim7020Etest_Task, osPriorityNormal, 0, 512);
  myTaskCmdAnalyzHandle = osThreadCreate(osThread(Sim7020Etest), NULL);
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
    //�е�ִ�п��ط�,��Ǯ,��ֵ�Ĳ��� ˢ���ȵĲ���

    tim_t run_rtc;
	  tim_t StartWarn; //��ʼ������ʱ��

    long date,time;//UTC

    //rfid��Ҫ�ı���
    uchar getdata[16];
    uint8_t ascii_i;
    uint8_t keyA[6] = {0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC};
    //uint8_t keyB[6] = {0x20, 0x19, 0x02, 0x20, 0x03, 0x44};
    uint8_t process;
    uint8_t status_RFID;//ִ�������״̬
    uint8_t Card_cap;//��Ƭ����
//	uint8_t M_Card_type = 0;  //��������,Ӧ�õ�����
    uint8_t Type_serial_number[2];   //�������ͱ���M1ʲô��
    uint8_t Card_serial_number[5];   //��Ƭ�Ĺ��к���,���ɸ���
    char Use_serial_number[32];      //ʹ�õĿ�Ƭ����
    uint8_t TmpCARDNoLength = 0;
    char Use_CUSTOMER_number[32];    //�ͻ���
    uint8_t TmpCUSTOMERNoLength = 0;
    char Use_Convert[32];           //�ӿ����ȡ��ת����
    uint8_t TmpConvertLength = 0;
    float Tmp_convert = 0;//ת���ɸ�������ת����
		char Use_Date[32];           //�ӿ����ȡ�Ŀ��Կ�������
		uint8_t TmpCardDateLength = 0;
		long Card_lock_date; //��Ƭ��������Ŀ�������

    //�����ϵ���Ҫ��ʲô,���緢��������,����ʱ�䵽��,��ʱ�ϱ�,����Ҫ������
    uint32_t TimeForSlowRunFree = 0;//��ѯCSQ��ʱ��
    //uint32_t TimeForBEATRun = 0;//��������ʱ��

    uint32_t TimeForSlowRunGPRSFree = 0;
		//uint32_t TimeForSlowCSRP = 0;
		
    //char str[64];
    char pAPN[17];
    char pAPN_UserName[17];
    char pAPN_Password[17];

    osDelay(500);

    
    //����Ƿ���������
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
		
//		Cooking_Session_READ(REAL_DATA_Credit.CookingSessionSendNumber);//���͵�ʱ��ӿ�ʼλ�ÿ�ʼ��ȡ,���ͳɹ�,������һ
//    char sendBuffer[200]={0};
//	  refreshCookingSessionReport(&CookingSessionReport);
//	  encodeCookingSessionReport(sendBuffer,&CookingSessionReport);
//    printf("sendBuffer:%s\r\n",sendBuffer);
    //printf("power down0 %ld\r\n",time);
    /* Infinite loop */
    for(;;)
    {
        //ϵͳ�������
        //printf("\r\npower\r\n");
        //HAL_GPIO_TogglePin(RUN_LED_GPIO_Port,RUN_LED_Pin);

        //�洢����
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

        //RFID���Դ���,��ϵͳΪ��RFIDˢ����ʱ��
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
                    //Find cards Ѱ��,���ؿ�����
                    status_RFID = MFRC522_Request(PICC_REQIDL, Type_serial_number);
                    if (status_RFID == MI_OK) //��⵽��
                    {
                        //printf("\r\n~~~Card detected~~~\r\n");
                        //printf("Card Type : %3d\r\n",Type_serial_number[0]);

                        // Anti-collision, return card serial number == 4 bytes
                        //����ײ,���ؿ������к�
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
                        memset(&Current_Credit,0,sizeof(Current_Credit_t));//û�м�⵽��,����������
                        Current_card_type = 0;
                        IsLockCard = false;
                        No_valid_Card = false; //Ĭ���޿�Ϊ��Ч
                    }
                }
                //��ѡ������ʱ��,����Ȼ��ͣ�Ķ�ȡ,ֱ����ȡ���ɹ���ʱ��,˵����Ƭ�Ƴ���,Ȼ�����������ˢ������ξ������˵�ǰ��Ƭ����Ϣ
                if(IsLockCard == true && Current_card_type == 0)
                {
                    // Be Carefull !! ::
                    // write data on 0,1,2,4,5,6,8,... and Don't write data on 3,7,11,.... blocks .
                    // write Key on 3,7,11,... blocks.
                    // Read from and Wirte to block 1 and 2
                    //��ȡ�ڲ���Ƭ����
                    //Try to authenticate each block first with the A key.
                    process=MFRC522_Auth(PICC_AUTHENT1A,0,keyA ,Card_serial_number);
                    //process= MFRC522_Write(ii,senddata);
                    if (process == MI_OK)
                    {
                        process= MFRC522_Read(1, getdata);//��ÿ�Ƭ������
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
                                Current_card_type = 1;//������
                            }
                            if(getdata[0] == '2'&&getdata[1] == '2'&&getdata[2] == '2'&&getdata[3] == '2'&&getdata[4] == '2'&&getdata[5] == '2')
                            {
                                //printf("ok2\r\n");
                                //M_Card_type = 2;
                                Current_card_type = 2;//������
                            }
                            if(getdata[0] == '3'&&getdata[1] == '3'&&getdata[2] == '3'&&getdata[3] == '3'&&getdata[4] == '3'&&getdata[5] == '3')
                            {
                                //printf("ok3\r\n");
                                //M_Card_type = 3;
                                Current_card_type = 3;//����У׼
                            }
                            if(getdata[0] == '4'&&getdata[1] == '4'&&getdata[2] == '4'&&getdata[3] == '4'&&getdata[4] == '4'&&getdata[5] == '4')
                            {
                                //printf("ok4\r\n");
                                //M_Card_type = 4;
                                Current_card_type = 4;//����
                            }

                            //�����������,�Ͷ�ȡ��Ƭ����Ϣ
                            switch(Current_card_type)
                            {
																case 1:// && strcmp(REAL_DATA_PARAM.Electric_Meter.ElectricValveStatus, "0")==0
																		//��ȡ��Ƭ�ı��
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
																						//IsLockCard = false;//��ȡ��Ƭ������ʧ��
																						No_valid_Card = true;
																				}
																		}
																		else
																		{
																				//IsLockCard = false;//��֤��Ƭ������ʧ��
																				No_valid_Card = true;
																		}
																		//��ȡ�ͻ�����
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
																						if(REAL_DATA_Credit.CardTotal > 0)//����ڴ����п�,�ȴ��ڴ���Ѱ��
																						{
																								while(TmpIndex < REAL_DATA_Credit.CardTotal)
																								{
																										memset(&Tmp_Credit,0,sizeof(Current_Credit_t));
																										Current_Credit_READ(TmpIndex);//ˢ����ʱ�򰤸���ȡ�ڴ��еĿ���Ϣ,���ȶ�
																										if(stringCmp(Tmp_Credit.CUSTOMER_ID,Use_CUSTOMER_number,7) == PASSED
																														&& stringCmp(Tmp_Credit.CARD_ID,Use_serial_number,18) == PASSED)
																										{
																												//���������Ŀ�Ƭ��Ϣ����Ϊ��ǰ��Ƭ����Ϣ
																												//�����ǰ�Ŀ�Ƭ���������Ƭ,�Ƚ���ֵ,����С����,����С����,Ҳ����,��һ�г�ֵ����ô����
		//																					if(stringCmp(Current_Credit.CUSTOMER_ID,Use_CUSTOMER_number,7) == PASSED
		//																					&& stringCmp(Current_Credit.CARD_ID,Use_serial_number,18) == PASSED)
		//																				  {
		//																						if(Tmp_Credit.CurrCredit != Current_Credit.CurrCredit)
		//																						{
																												Current_Credit.CurrCredit = Tmp_Credit.CurrCredit;//�������
		//																						}
		//																						printf("CurrCredit:%f\r\n", Current_Credit.CurrCredit);
		//																						printf("Tmp_Credit:%f\r\n", Tmp_Credit.CurrCredit);
		//																				  }
																												Current_Credit.CUSTOMERNoLength = 7;//�ͻ�����
																												strncpy(Current_Credit.CUSTOMER_ID,Use_CUSTOMER_number,32);//�ͻ���
																												Current_Credit.CARDNoLength = 18;//��Ƭ����
																												strncpy(Current_Credit.CARD_ID,Use_serial_number,32);//��Ƭ��

																												Current_Credit.CurrIndex = Tmp_Credit.CurrIndex;//��Ƭ���

																												REAL_Current_Credit_Write();
																												//���滹��Ҫ��ֵ
																												//																	if(Current_Credit.CurrCredit <=0)
																												//																	{
																												//																		//TODO ��ȡ���
																												//																	}
																												break;
																										}
																										TmpIndex++;
																								}
																						}
																						if(TmpIndex == REAL_DATA_Credit.CardTotal)//ˢ�����Ҳ������½�
																						{
																								memset(&Current_Credit,0,sizeof(Current_Credit_t));
																								Current_Credit.CurrIndex = TmpIndex;//��ֵ����
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
																								REAL_DATA_Credit_Write();//�������� ˢ�����½���
																								Current_Credit_Write(Current_Credit);//ˢ���������½�����ʱ��
																								//TODO ��ȡ��� ���û�ȡ���,����ֱ�ӽ���
																						}
																				}
																				else
																				{
																						//IsLockCard = false;//��ȡʧ��
																						No_valid_Card = true;
																				}
																		}
																		else
																		{
																				//IsLockCard = false;//��֤�����ź�����ʧ��
																				No_valid_Card = true;
																		}
																		break;
																		//������ֹͣˢ��
																case 2://����
//																		if(strcmp(REAL_DATA_PARAM.Electric_Meter.LidElectricLock, "1")==0)
//																		{
//																		 strcpy(REAL_DATA_PARAM.Electric_Meter.LidElectricLock,"0");
//																		 glockStatus = motor_open;
//																		}
																    //��ȡ��Ƭ������
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
																						//IsLockCard = false;//��ȡ��Ƭ������ʧ��
																						No_valid_Card = true;
																				}
																		}
																		else
																		{
																				//IsLockCard = false;//��֤��Ƭ������ʧ��
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
																					//TODO ���濪����־ ,�����Ϳ�������

																					LogWrite(OpenAssemble,"OpenAssemble",NULL);
																			}
																		}
																		break;
																		//У׼�߼�
																case 3://У׼����
																		//��ȡת��ϵ��
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

																						if(Tmp_convert == 0)//���ת���ʵ����㴥��У׼�߼�
																						{
																								IsTestFlow = true;
																								IsTestRFID = false;
																								osDelay(100);
																								//RFID_POWER(0);
																								IsInitRFID = false;

																						}
																						else//�����ת��ϵ����ֵ
																						{
																								REAL_DATA_Credit.Magnification = Tmp_convert;
																								LL_VCC(1);
																								REAL_DATA_Credit_Write();	//����ת��ϵ��
																								//IsLockCard = false;
																								Current_card_type = 0;
																								osDelay(500);
																						}
																				}
																				else
																				{
																						//IsLockCard = false;//��ȡʧ��
																						No_valid_Card = true;
																				}
																		}
																		else
																		{
																				//IsLockCard = false;//��֤ʧ��
																				No_valid_Card = true;
																		}
																		break;
																case 4://�ظ�����
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
                            IsLockCard = false;//��ȡ��Ƭ����ʧ��
                            No_valid_Card = true;
                        }
                    }
                    else
                    {
                        IsLockCard = false;//��֤��Ƭ����ʧ��
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
//			  LL_VCC(1);   //�洢���ϵ粢Ū�ɿ���д��״̬
//				REAL_Current_Credit_Write();//���û��ˢ��һֱ�洢��ǰ��Ϣ
//		}

        //��ʱ���Ѳɼ�
        //BATAD_POWER(1);
        //REAL_DATA_PARAM.BatVoltage = ADC_BAT();

        //û�п����������Ϩ����Ļ��,������ʱ���Ǳ����п�,�����Ƭ��Ϣ,�����Դ�ʩ,
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
        //����
        //���ر�ˢ�����Ե�ʱ��,�������;�������,ֻ��Ҫִ�ж�Ӧ�Ŀ�Ƭ�����ͺ�
        //����ʢû�е�ѹ��ʱ�䳬��3����,�Զ��ط�,���ߵ�ѹ����,û��Ǯ,û���� �رշ���
        if(Current_card_type == 1)
        {
            if(haveClickButton == true)
            {
                haveClickButton = false;
                //�ڸ�����������������,����
                if(Current_Credit.CurrCredit > 0
                        && REAL_DATA_PARAM.TankQuality > 0.5
                        && REAL_DATA_PARAM.BatVoltage >= 3.6
                        && strcmp(REAL_DATA_PARAM.ElectricValveStatus, "0")==0
                        //&& Key2_input == 1 //Ϊ�˼���100L��10L��PCB,ɾ���������
								        //&& Opening_input == 0
                        && NEEDLE_input == 0
                        && gmotorStatus == motor_null
                        && sStep != 0 //������ʱ�򲻿���
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
                else //������ʱ�򲻲�����Ļ
                {
                    gkeyValue = true;
                }
            }

            //�ط�
            if((
								(zeroFlowTime > 10 && Get_SubSeconds(StartCookingSeconds) < 120)//����ʼ����С��5���ӵ�ʱ���ж�����������10�Զ��ط�
								|| (TmpRealTimeFlow < 0.3 && Get_SubSeconds(StartCookingSeconds) >= 120)//����ʼ�������ڵ���5���ӵ�ʱ���ж�����������1�Զ��ط�
								//|| REAL_DATA_PARAM.BatVoltage < 3.6 ���������в��жϵ�ѹ,��Ϊ��ѹ�仯û����ô��,��һ�п��������������ѵ�ѹ˲������,Ҳ�����ǳ������
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
                Current_card_type = 0;//��������,�������ʱ�������ʾ��,�򲻿�ˢ��
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
                        Cooking_Session_Write();//ÿ�����극����������������洢cooking

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
                    REAL_DATA_PARAM_Write();//���淧�ŵ�ʵʱ��Ϣ
                    REAL_DATA_Credit_Write();//���������Ǳ�ĳ�ֵ��Ϣ �ط�ʱ

                    //else
                    {
                        //���浱ǰ�Ǳ�ĳ�ֵ��Ϣ,��RFID��ʱ����ʵ���ñ���,��ʵ֤���ڴ�Ĵ洢���д����ݴ���Ī����ʧ����
                        //Ϊ�˱��������������,���ֵ�ĳ�ÿ�δ洢,����ÿ�����극��������ȡ������,�������´ζ�ȡ��ʱ�����ȶ�,��СֵΪ׼
                        REAL_Current_Credit_Write();
                    }

                    //if(CONFIG_Meter.IsHaveRFID)
                    {
                        Current_Credit_Write(Current_Credit);//ʹ���йرշ��ŵ�ʱ��,ÿ��д�����¶�ȡ,Ӧ��һ��,�����һ��,��Ϊ����,������ʱ����д��
                        //memset(&Tmp_Credit,0,sizeof(Current_Credit_t));
                        //Current_Credit_READ(Current_Credit.CurrIndex);
                    }
										
										//printf("Current_Credit:%f\r\n",Current_Credit.CurrCredit);
										//printf("Tmp_Credit:%f\r\n",Tmp_Credit.CurrCredit);
										
                    //����ո�д��ȥ�Ĳ���������Ҫд���Ĳ���,˵��д��ʧ��,���ʱ������,�ȽϺ�����д��
//                    if(Current_Credit.CurrCredit != Tmp_Credit.CurrCredit)
//                    {
//                        CONFIG_Meter.IsNormalReset = true;
//											  
//                        CONFIG_Meter_Write();
//                        //��������
//                        LogWrite(RestartUnNorm,"bigFlashError",NULL);
//											
////                        HAL_NVIC_SystemReset();
//                    }
//                    else
                    {
                        HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
                        osDelay(500);
                        HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);

                        Current_card_type = 0;//���λ��ʵ�����Ƕ����

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
                                IsNeedWarning = true;//�ر�ʱ�б���
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

        //��������߼�,���������Ŀ�Ƭ�ǿ����������޿�,
        //�����Թ���,ת��ҡ�ѹ���
        if(Current_card_type == 2 || Current_card_type == 0)//����
        {
            if(haveClickButton == true)
            {
                //LogWrite(ClickButton,"ClickButton",NULL);
                haveClickButton = false;
                gkeyValue = true;
            }

            //����뷧�ǿ�,��ж���ǿ�,���û�ж���,��ֹ�����û�ж������,����������ҡ��,��������쳣
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

        //�����ǰ��У׼�Ŀ��Ĵ����߼�
        if(Current_card_type == 3)
        {
            //���뷧,����У׼�����ڵ�,�ر�У׼�߼�
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

            //���뷧��ʼУ׼
            if(NEEDLE_input == 0 && IsStartCALIBRATION == false)
            {
                IsStartCALIBRATION = true;
            }

            if(haveClickButton == true)
            {
                haveClickButton = false;
                if(NEEDLE_input == 0)//�뷧����ʱ��
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
                else //�뷧�ص�ʱ����Բ�����Ļ,�鿴��ѹ
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

        //��ѯ��Ƭ���
        if(commandType == 2)
        {
            int16_t TmpIndex = 0;

            if(REAL_DATA_Credit.CardTotal > 0)
            {
                while(TmpIndex < REAL_DATA_Credit.CardTotal)
                {
                    memset(&Tmp_Credit,0,sizeof(Current_Credit_t));
                    Current_Credit_READ(TmpIndex); //��������ѯ��Ƭ����ʱ����ڴ���Ѱ��
                    //printf("CUSTOMER_ID : %s\r\n",Tmp_Credit.CUSTOMER_ID);
                    //printf("CARD_ID : %s\r\n",Tmp_Credit.CARD_ID);
                    if(stringCmp(Tmp_Credit.CUSTOMER_ID,CustomerCredit.CUSTOMER_ID,7) == PASSED
                            && stringCmp(Tmp_Credit.CARD_ID,CustomerCredit.CARD_ID,18) == PASSED)
                    {
                        sprintf(CustomerCredit.CREDIT_REMAINING,"%.1f",Tmp_Credit.CurrCredit);//�Ѳ鵽����ֵ������������CustomerCredit
                        IsNeedCustomerCreditResponse = true;
                        break;
                    }
                    TmpIndex++;
                }
            }
            commandType = 0;
        }

        //��ִ�п���ָ���,ȴû����һ��ָʾ,��һ��ʱ����Զ�����
        //���ǵ���֮������ľͲ�ִ����
        //����������Ҫ�رղ���
        //�������ҡ�ѹ�������,�������ж�ʱ��
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
                RFID_POWER(1);//�Զ��رո�ƿ����ʱ��,��Ҫ���²���RFID
                osDelay(100);
                IsTestRFID = true;
                IsInitRFID = false;
                IsLockCard = false;
            }

            Current_card_type = 0;

            LogWrite(CloseAssemble,"timeOut",NULL);
        }

        //�洢���ӵ�״̬
        if(LidStatus != (Opening_input == 1 ? true : false))
        {
            LidStatus = (Opening_input == 1 ? true : false);
            strcpy(REAL_DATA_PARAM.LidSensorStatus,(Opening_input == 1 ? "1" : "0"));//0:Lid closed, 1: Lid open
            LL_VCC(1);
            REAL_DATA_PARAM_Write();
        }

        //�뷧��״̬
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

        //��ť�����Ĳ���
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

        //��Ƭ����1,���߿�Ƭ��1���Ƿ����ǹرյ�|| (Current_card_type == 1 && strcmp(REAL_DATA_PARAM.ElectricValveStatus, "0")==0)
        //ֻ��û����ʱ��,�Ŵ��������߼�
        if(Current_card_type == 0)// && IsReadVoltage == false
        {
            //�����Դ�ʩ,��ֹ���Ų��ر�,�����ϲ���ִ�е�,��������û�йر�
            //��Ҫ���ȳ��Զ�ιر�,�����������ƫ��,��Ҫ����
            if(strcmp(REAL_DATA_PARAM.ElectricValveStatus, "1")==0 )//|| ADTestVFlow > 1)
            {
                //if(gmotorStatus == motor_null)
                {
                    gmotorStatus = motor_close;
                    strcpy(REAL_DATA_PARAM.ElectricValveStatus,"0");//0:Valve closed, 1: Valve open

                    LL_VCC(1);
                    REAL_DATA_PARAM_Write();//���淧�ŵ�ʵʱ��Ϣ
                }

                IsTestRFID = true;
            }

            run_rtc = Read_Time();
            //printf("power down1 %d:%d:%d\r\n",run_rtc.tm_hour,run_rtc.tm_minute,run_rtc.tm_seconds);
            //ÿ�춨ʱ���� 02:22:00
            if(run_rtc.tm_hour == 2 && run_rtc.tm_minute == 22 && run_rtc.tm_seconds == 0 && Lcd_Status == false && IsNeedTimeRestart == true && IsNeedSendCook == false)
            {
                CONFIG_Meter.IsNormalReset = true;
                CONFIG_Meter_Write();

                //��������
                LogWrite(RestartNormal,"TimedRestart",NULL);
							  IntoLowPower();
//                HAL_NVIC_SystemReset();
            }

            //���û��ʹ��,��Ļ�����״̬,��ʱα����,ǰ����ϵͳû������,û��ʹ�ÿ��Ź���ʱ��,���ʱ������Ļ����ϵͳ
            if((run_rtc.tm_minute == 55) && run_rtc.tm_seconds == 0 && Lcd_Status == false && IsNeedTimeRestart == true && CONFIG_Meter.NotHaveDog == true && IsNeedSendCook == false)
            {
                CONFIG_Meter.IsNormalReset = true;
                CONFIG_Meter_Write();
                //printf("power down2 %d:%d:%d\r\n",run_rtc.tm_hour,run_rtc.tm_minute,run_rtc.tm_seconds);
                //��������
                LogWrite(RestartNormal,"OneHourRestart",NULL);
							  IntoLowPower();
//                HAL_NVIC_SystemReset();
            }

            //һСʱ��ѯһ�ε�ѹ
            if(run_rtc.tm_minute == 23)//run_rtc.tm_minute == 0
            {
                //ǰ����ɼ�
                if(run_rtc.tm_seconds < 10)
                {
                    BATAD_POWER(1);
                    IsTestPower = true;
                    IsTestEnd = false;
                    //printf("BATAD_POWER(1)\r\n");
                }
                else if(run_rtc.tm_seconds >= 10 && run_rtc.tm_seconds < 20)//10��20�뿪ʼ����
                {
                    //printf("BATAD_POWER(2)\r\n");
                    if(REAL_DATA_PARAM.BatVoltage <= CONFIG_Meter.LowBattery && IsLowPowerWarned == false)//�����ѹ���Ͷ���û�й��͵�ѹ����
                    {
                        IsLowPowerWarned = true;
                        IsNeedWarning = true;//��ѹ����
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
                else//����ʮ���ʱ��
                {
                    //printf("BATAD_POWER(3)\r\n");
                    if(sStep == 0)//�����Ļ�ǹرյ�,�رյ�ز���
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

            //��ʱ��
//            if(IsNeedTimeing == true)//����ж��ж�ʱ��
//            {
//                if(TimeRetryNumber < 1)//�ط�����С������,��һ�鲻��,���һ�������Ĵ�
//                {
//                    if(GSM_ON_FLAG == 0)//���û���ϵ�,���ϵ翪ʼ����
//                    {
//                        TimeRetryNumber++;

//                        connectStep = 0;
//                        GSM_ON_FLAG = 1;
//                        //printf("Sim80x_SetPower(true)16\r\n");
//                        Sim80x_SetPower(true);
//                    }
//                }
//                if(TimeRetryNumber >= 1)//����ٴ��������ڵ�������,�����ط���Ч
//                {
//                    IsNeedTimeing = false;
//                    TimeRetryNumber = 0;
//                    //AL_IWDG_Refresh(&hiwdg);
//                    //Sim80x_GPRSClose(13);
//                }
//            }
//            else //������е�ʱ��,һ���ж��Ƿ���Ҫ���Ͷ�ʱ��
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
				
				//�����ϸ��ӵ�ʱ��,�������ֵ,��ô���򿪵�ʱ���ɿ�����������
				if(Opening_input == 0)
				{
				  WarnTime = 0;
				}
				
				//�������Ǵ򿪵�ʱ��
        if((Opening_input == 1 || vibrationsNumber >= 3) 
           && (WarnTime == 0 || Get_SubSeconds(StartWarn) > 60 * 5)
				  )
        {
            //������ʱ��,�����ǹرյ� glockSet != motor_open &&  //glockSet == motor_close && IsNeedIllegal_lid_opening == true
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
									  IsNeedWarning = true;//�Ƿ����Ǳ��� ���ж����ƶ����ڲ�
									
                    gkeyValue = true;//����Ļ������
                    connectStep = 0;
                    GSM_ON_FLAG = 1;
                    printf("Sim80x_SetPower(true)1\r\n");
                    Sim80x_SetPower(true);
                }
//                if(sStep != 0)//��Ļ������ʱ����������������
//                {
//                    HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
//                    osDelay(500);
//                    HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
//                }
            }
        }
//		else
//		{
//			  //�رո��Ӳ���Ҫ����
//			  IsNeedWarning = false;
//			  if(strcmp(CONFIG_Meter.Lid_Type, "1")==0 && lockValue == true)//�����һ���Ļ�,����30����Կ���
//				{
//					lockValue = false;
//					OpenLockTime = 30;
//					glockSet = motor_open;
//				}
//		}
				
				
        //printf("connectStep ----> %d\r\n",connectStep);
        //�������߼�
        if(GSM_ON_FLAG == 1)// && IsReadVoltage == false
        {
            switch(connectStep)
            {
            case 0://ִ�е������ֹͣ��
                //printf("start connectStep ----> %d\r\n",connectStep);
                TimeForPowerStart = HAL_GetTick();
                TimeForCurrStart = HAL_GetTick();
                connectStep = 1;//��ʼ��������Ҫ�Ĳ���
                break;
            case 1://ִ���ϵ�Ĳ������ж�
                //printf("Sim80x.Status.Power %d\r\n",Sim80x.Status.Power);
                if(Sim80x.Status.Power==1)
                {
                    connectStep = 2;//�ϵ�ɹ�Ϊ2
                    loopSumCurr = 0;
                    TimeForCurrStart = HAL_GetTick();
                }
                else
                {
                    if(HAL_GetTick() - TimeForCurrStart >= 10 * 1000 )//����10��ATģ��û�з�Ӧ,��������,һ������,ģ�黵��,1��
                    {
                        loopSumCurr++;
                        if(loopSumCurr >= 3)
                        {
                            //ģ�黵��  Ҳ������ע������г�����  0
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
                            //����Ļ��ʾ
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
                        TimeForCurrStart=HAL_GetTick();//PDP(Packet Data Protocol )�ɹ���ʱ��
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

                                //����Ļ��ʾ
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
                    TimeForCurrStart=HAL_GetTick();//PDP(Packet Data Protocol )�ɹ���ʱ��
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
                    connectStep = 4;//Ѱ�ҵ�SIM��
                    loopSumCurr = 0;
                    TimeForCurrStart = HAL_GetTick();//��ʼע�Ὺʼ��ʱ
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
                if(Sim80x.Status.RegisterdToNetwork==1)//�ɹ�����һ��
                {
                    connectStep = 5;//ע��ɹ�Ϊ5
                    loopSumCurr = 0;
                    TimeForCurrStart = HAL_GetTick();//����ע��ɹ���ʱ��
                    //DisplayChar(1,1,"--Register  OK--");
                }
                else if(Sim80x.Status.RegisterdToNetwork == 3 || HAL_GetTick() - TimeForCurrStart >= 60 * 1000 * 10)//���ɹ����߳�ʱ��������
                {

                    loopSumCurr++;
                    //printf("RegisterdToNetwork:%d\r\n",Sim80x.Status.RegisterdToNetwork);
                    //printf("RegisterdToNetwork:%d  |  %d\r\n",HAL_GetTick() - TimeForCurrStart,20 * 1000);
                    //printf("Reging loopSumCurr:%d\r\n",loopSumCurr);

                    if(loopSumCurr >= 3)
                    {
                        //û��ע��ɹ�
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

                        //����Ļ��ʾע��δ�ɹ�
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
                else//�����в�ͣ����CGREG
                {
                    if(HAL_GetTick() - TimeForCurrStart > 1000)
                    {
                        Sim80x_SendAtCommand("AT+CSQ\r\n",200,1,"\r\n+CSQ:"); //�ź�����
                        //Sim80x_SendAtCommand("AT+CBC\r\n",200,1,"\r\n+CBC:"); //ģ��ĵ�ѹ
                        Sim80x_SendAtCommand("AT+CGREG?\r\n",200,1,"\r\n+CGREG:");
                        //Gsm_MsgGetMemoryStatus();//ѡ����Ŵ洢�ص�
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
                    TimeForCurrStart = HAL_GetTick();//��������ɹ���ʱ��
                }
                else
                {
                    if(HAL_GetTick() - TimeForCurrStart >= 10 * 1000 )
                    {
                        loopSumCurr++;

                        if(loopSumCurr >= 3)
                        {
                            //��������ʧ��
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

                            //����Ļ��ʾע��δ�ɹ�
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
                        //��������ʧ��
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

                        //����Ļ��ʾע��δ�ɹ�
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
                    TimeForCurrStart = HAL_GetTick();//�ɹ���ʱ��
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
                TimeForCurrStart = HAL_GetTick();//�ɹ���ʱ��
                break;
            case 8:
                //printf("\r\nloopSumPDP %d\r\n",loopSumPDP);
                if(loopSumPDP >= 3)//�������ʲô����½���PDP�Ĵ���Ϊ3��,������������
                {
                    if(CONFIG_Meter.NotHaveDog == false && IsNeedRestart == false)
                    {
                        HAL_IWDG_Refresh(&hiwdg);
                    }
                    Sim80x_GPRSClose(8);
                    break;
                    //����Ļ��ʾע��δ�ɹ�
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
                    TimeForCurrStart = HAL_GetTick();//�ɹ���ʱ��
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
                TimeForCurrStart = HAL_GetTick();//�ɹ���ʱ��
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

                        //����Ļ��ʾע��δ�ɹ�
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
                    TimeForCurrStart = HAL_GetTick();//�ɹ���ʱ��
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
                        connectStep = 6;//����������ʧ��,�Ͽ���������������
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
                            else if(IsNeedRepayControl == true)//���Ʊ��Ļظ�
                            {
                                //printf("IsNeedRepayControl\r\n");
                                SendControlPacket();
                                IsNeedRepayControl = false;
                                TimeForCurrStart = HAL_GetTick();
															  HearRetryNumber = 0;
                            }
                            else if(IsNeedRepayReCharge == true)//���Ʊ��Ļظ�
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

                                    //�����ֵ�Ŀ�Ϊ��ǰ��,ֱ�ӳ�ֵ
                                    if(stringCmp(Current_Credit.CUSTOMER_ID,RechargePacket.CUSTOMER_ID,7) == PASSED
                                            && stringCmp(Current_Credit.CARD_ID,RechargePacket.CARD_ID,18) == PASSED)
                                    {
                                        Current_Credit.CurrCredit += rechargeCredit;
                                        REAL_Current_Credit_Write();
                                        Current_Credit_Write(Current_Credit);//��ǰ������Ҫ��ֵ�Ŀ���ʱ��,�����ֵ����
                                    }
                                    else//������������ǰ��,����ҪѰ�ҳ�ֵ
                                    {
                                        if(REAL_DATA_Credit.CardTotal > 0)//Ѱ�ҵ�ֱ�ӳ�ֵ,������
                                        {
                                            while(TmpIndex < REAL_DATA_Credit.CardTotal)
                                            {
                                                memset(&Tmp_Credit,0,sizeof(Current_Credit_t));
                                                Current_Credit_READ(TmpIndex);	//��ֵ��ʱ���ȶ�ȡ�ٰ����ȶ�
                                                if(stringCmp(Tmp_Credit.CUSTOMER_ID,RechargePacket.CUSTOMER_ID,7) == PASSED
                                                        && stringCmp(Tmp_Credit.CARD_ID,RechargePacket.CARD_ID,18) == PASSED)
                                                {
                                                    Tmp_Credit.CurrCredit = Tmp_Credit.CurrCredit + rechargeCredit;
                                                    Current_Credit_Write(Tmp_Credit);//��ǰ��ֵ�Ŀ���������ʹ�õĿ�Ƭ��ʱ��,�ڼ�¼���ҵ��˿�Ƭ
                                                    break;
                                                }
                                                TmpIndex++;
                                            }
                                        }
                                        //Ѱ�Ҳ����½���
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
                                            REAL_DATA_Credit_Write();//��ֵ�����ڴ����Ҳ�����Ƭ,�½���Ƭ,��Ƭ������һ
                                            Current_Credit_Write(Tmp_Credit);//��ǰ��ֵ�Ŀ���������ʹ�õĿ�Ƭ��ʱ��,�ڼ�¼���Ҳ�����Ƭ
                                        }
                                    }
                                }
                                IsNeedRepayReCharge = false;
                                TimeForCurrStart = HAL_GetTick();
																HearRetryNumber = 0;
                            }
                            else if(IsNeedTimeing == true)//��ʱ��
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
                            else if(IsNeedWarning == true)//������Ϣ
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
                            else if(IsNeedSendCook == true)//����������,�������Ҫ�ϴ�����Ϣ�ϴ�
                            {
                                //if have CSRP,Send data every five seconds,if revice Reply,send next CSRP
                                if(REAL_DATA_Credit.CookingSessionEnd > REAL_DATA_Credit.CookingSessionSendNumber &&
																	(
																	 (IsReceivedCSRPReply == true && HAL_GetTick()-TimeForCurrStart > 1000 * 10)//����յ��ظ�ҲҪ��ʮ��
																		 || (IsReceivedCSRPReply == false && IsSendedCSRP == false && HAL_GetTick()-TimeForCurrStart > 1000 * 10) //��֤��һ�ο��Խ���
																  )
																)
																{
																	  IsSendedCSRP = true;
																	  IsReceivedCSRPReply = false;
                                    LL_VCC(1);
                                    Cooking_Session_READ(REAL_DATA_Credit.CookingSessionSendNumber);//���͵�ʱ��ӿ�ʼλ�ÿ�ʼ��ȡ,���ͳɹ�,������һ
																		
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
																			REAL_DATA_Credit_Write();//������cooking ,�������
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
											//(IsNeedHearting == true && HAL_GetTick() - TimeForCurrStart >= 20 * 1000) || //���������������,�������Ͽ�ʼ��ʱ6��Ͽ�
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
							if(HAL_GetTick() - TimeForPowerStart >= 60 * 1000 * ((strstr(CONFIG_Meter.MeterNo,"KE") != NULL && Sim80x.Modem_Type == SIM7020E) ? 12 : 3))//�ϵ翪ʼ�㳬��3����
                {
                    HearRetryNumber = 0;

                    IsNeedTimeing = false;
                    IsSendHalfTime = false;//�Ƿ��Ѿ�������һ��Ķ�ʱ��
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
                    Sim80x_SendAtCommand("AT+CSQ\r\n",200,1,"\r\n+CSQ:"); //�ź�����
                    //Sim80x_SendAtCommand("AT+CBC\r\n",200,1,"\r\n+CBC:"); //ģ��ĵ�ѹ
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
        //����͹���
        if(((Current_card_type == 0 //û�п�Ƭ
             && strcmp(REAL_DATA_PARAM.ElectricValveStatus, "0")==0)// && IsReadVoltage == false
             ||(Current_card_type == 1 //�п�Ƭ
                && strcmp(REAL_DATA_PARAM.ElectricValveStatus, "1")==0 	 //�����ǿ��ŵ�
                   //&& StartRunTime > 15 //���г���3��
                && IsTestEnd == true
                && (IsStartCooking5 == true && Get_SubSeconds(StartCookingSeconds) >= 120)) //�����ʼ�������������
           )
                && Lcd_Status == false  //��Ļ�ǹر�
                //&& gkeyValue == false
                && sStep == 0
                && IsTestPower == false  //û�н��е����ɼ�
                && gmotorStatus == motor_null && gassembleStatus == motor_null //���Ŷ�û����Ҫ�����������
                && connectStep == -1     //û������
                //&& 1 != 1
                && GSM_ON_FLAG == 0   //û�г�������
				)   
        {
            IsIntoLowPower = true;
            if(IsStartCooking5 == false && Get_SubSeconds(StartCookingSeconds) >= 120)
            {
                //IsStartCooking5 = true;//����ǿ��������е͹���,��ȡ��ע��
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
            	 1. ֹͣģʽ����Cortex-M3����˯��ģʽ�����Ͻ���������ʱ�ӿ��ƻ��ƣ���ֹͣģʽ�µ�ѹ
            		��������������������͹���ģʽ����ʱ��1.8V��������ĵ�����ʱ�Ӷ���ֹͣ�� PLL�� HSI��
            		HSE��RC�����Ĺ��ܱ���ֹ�� SRAM�ͼĴ������ݱ�����������
            	 2. ��ֹͣģʽ�£����е�I/O���Ŷ���������������ģʽʱ��״̬��
            	 3. һ��Ҫ�رյδ�ʱ����ʵ�ʲ��Է��ֵδ�ʱ���ж�Ҳ�ܻ���ͣ��ģʽ��
            */
            //		SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;  /* �رյδ�ʱ�� */

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
            		1����һ���жϻ����¼������˳�ֹͣģʽʱ�� HSI RC������ѡΪϵͳʱ�ӡ�
            		2���˳��͹��ĵ�ͣ��ģʽ����Ҫ��������ʹ��HSE��
            */
            /* Set the new HSE configuration ---------------------------------------*/
            if(MYSYSCLK == 8)
						{
								//�����ⲿ����ʱ��
								__HAL_RCC_HSE_CONFIG(RCC_HSE_ON);
								while (__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == RESET)
								{
										//�Ƿ������
								}
								//ѡ��ϵͳʱ��ΪHSE
								__HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_HSE);
								//printf("__HAL_RCC_GET_SYSCLK_SOURCE()2 %d\r\n",__HAL_RCC_GET_SYSCLK_SOURCE());
								//�ж��Ƿ����óɹ�
								while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_HSE)
								{
										//�Ƿ����óɹ�
										printf("__HAL_RCC_GET_SYSCLK_SOURCE()3 %d\r\n",__HAL_RCC_GET_SYSCLK_SOURCE());
										//������ɹ���ô��,������
								}
								//printf("__HAL_RCC_GET_SYSCLK_SOURCE()4 %d\r\n",__HAL_RCC_GET_SYSCLK_SOURCE());
								//			SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; /* ʹ�ܵδ�ʱ�� */
					  }
						else
						{
								//�����ⲿ����ʱ��
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
                //ѡ��SYSCLKΪPLL
								__HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_PLLCLK);

								while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_PLLCLK)
								{
									//printf("1111");
								}
								//printf("__HAL_RCC_GET_SYSCLK_SOURCE()4 %d\r\n",__HAL_RCC_GET_SYSCLK_SOURCE());
								//			SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; /* ʹ�ܵδ�ʱ�� */
						}	
            __HAL_RCC_GPIOA_CLK_ENABLE();
            __HAL_RCC_GPIOB_CLK_ENABLE();
            __HAL_RCC_GPIOC_CLK_ENABLE();
            __HAL_RCC_GPIOD_CLK_ENABLE();
            __HAL_RCC_GPIOF_CLK_ENABLE();
            //�رյ��������
            //		  HAL_GPIO_WritePin(MOTOR1_H_PORT,MOTOR1_H_PIN,GPIO_PIN_RESET);
            //		  HAL_GPIO_WritePin(MOTOR1_L_PORT,MOTOR1_L_PIN,GPIO_PIN_RESET);
            //
            //			HAL_GPIO_WritePin(MOTOR2_H_PORT,MOTOR2_H_PIN,GPIO_PIN_RESET);
            //			HAL_GPIO_WritePin(MOTOR2_L_PORT,MOTOR2_L_PIN,GPIO_PIN_RESET);
            //
            //			HAL_GPIO_WritePin(MOTOR3_H_PORT,MOTOR3_H_PIN,GPIO_PIN_RESET);
            //			HAL_GPIO_WritePin(MOTOR3_L_PORT,MOTOR3_L_PIN,GPIO_PIN_RESET);

            //��RFID
            HAL_GPIO_WritePin(RC522_Reset_GPIO_Port,RC522_Reset_Pin,GPIO_PIN_SET);//�ø�,���ֲ���λ
            //���������ø�,��ʾ��������
            HAL_GPIO_WritePin(SPI_CS3_GPIO_Port,SPI_CS3_Pin,GPIO_PIN_SET);//RFID CS �ø߳ɲ�����
            MB_SPI_CS_DISABLE;     //СFlash CS ��ʹ��
            FLASH_SPI_CS_DISABLE;  //��Flash CS ��ʹ��

            HAL_SPI_MspInit(&hspi2);
            //HAL_UART_MspInit(&huart3);
            //MX_USART3_UART_Init();
            //HAL_UART_Receive_IT(&huart3,&Usart3RxTemp,1);//�ӵ͹���������ʱ�������жϽ�������
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
            if(Lcd_Status == true && IsStartCooking5 == true)//����ǰ����˳��͹���ģʽ
            {
                IsTestEnd = false;
                IsStartCooking5 = false;
                StartCookingSeconds = Read_Time();
                //HAL_TIM_Base_Start_IT(&htim7);

                //printf("IsStartCooking5 4\r\n");
            }
            else
            {
                //һ���Ӳ���һ��,���ʱ�������򿪲���,������ɺ�,��һ���Ӱ��ձ��β�����,������ɺ��ȥ�͹���
                if(strcmp(REAL_DATA_PARAM.ElectricValveStatus, "1")==0 && Read_Time().tm_seconds == 0)//��������ǿ��ŵ�
                {
                    //gkeyValue = true;
                    IsTestEnd = false;
                    //����ѹ
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

        //�ӵ���һ���߳��ƶ�������,�����ڽ���͹������ж��Ƿ��з����ж���û�����
        //һ��һ����ִ��
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
			
        //��Ļ�����ж���
        if(tkeyValue != 0)
        {
            LCD_Delay_Time = 10 * 100;//ÿ���а������µ�ʱ��,�ӳ���Ļ����ʱ��
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
								
                if(Opening_input == 0) //�������Ϊ���ŵ�,ͬʱ˫����ť,�Ժ��б����¼��ͻ����
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

        //�ڲ������Ĳ�Key2
        key2Value = key_read(&Key2_Status);

        if(key2Value != 0)
        {
            //printf("LCD_Delay_Time4 %d\r\n",LCD_Delay_Time);
            LCD_Delay_Time = 10 * 100;//ÿ���а������µ�ʱ��,�ӳ���Ļ����ʱ��
					  vibrationsNumber++;
					  //printf("vibrationsNumber %d\r\n",vibrationsNumber);
        }

        //��������Ĳ���
//		    openLidValue = key_read(&OpenLid_Status);

//				if(openLidValue != 0)
//				{
//					 //printf("openLidValue %d\r\n",openLidValue);
//					 //LCD_Delay_Time = 10 * 100;//ÿ���а������µ�ʱ��,�ӳ���Ļ����ʱ��

//					//����������һֱ���͵�ʱ��������Ӹ�����
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
            LCD_Delay_Time = 10 * 100;//ÿ���а������µ�ʱ��,�ӳ���Ļ����ʱ��
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
                    //����뷧�ǿ�,��ж���ǿ�,���û�ж���,��ֹ�����û�ж������,����������ҡ��,��������쳣
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
                if(strcmp(LockPassword + gkeySum, "1")==0)//�籨����
                {
                    //�������� ��������������
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
    //�Ǹ�ţ�Ƽ���ڴ���Ҫ��
    //uint32_t g_memsize;
    //uint8_t CPU_RunInfo[400];		//������������ʱ����Ϣ
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
//					printf("ϵͳ��ǰ�ڴ��СΪ %d �ֽ�\r\n",g_memsize);
//
//					memset(CPU_RunInfo,0,400);				//��Ϣ����������
//
//					vTaskList((char *)&CPU_RunInfo);  //��ȡ��������ʱ����Ϣ
//
//					printf("---------------------------------------------\r\n");
//					printf("������      ����״̬ ���ȼ�   ʣ��ջ �������\r\n");
//					printf("%s", CPU_RunInfo);
//					printf("---------------------------------------------\r\n");
//
//					memset(CPU_RunInfo,0,400);				//��Ϣ����������
//
//					vTaskGetRunTimeStats((char *)&CPU_RunInfo);
//
//					printf("������       ���м���         ʹ����\r\n");
//					printf("%s", CPU_RunInfo);
//					printf("---------------------------------------------\r\n\n");
//					//vTaskDelay(5000);   /* ��ʱ500��tick */
//				}

        //��������˰�Сʱ��û�н���͹���,��û��ˢ���������
        //����ǿ��������õİ�
        if(StartRunTime > 1800 && Current_card_type == 0)//&& IsReadVoltage == false
        {
            IsSaveCONFIG_Meter = true;
            IsNeedRestart = true;
        }
        //������ϵͳ����ʱ�����10��������ڶ�������
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
								//��������һ��Ӳ��,ֻҪ����Ļ��������ʱ��Ϳ���ѹ,��GPRS
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
										if(GSM_ON_FLAG == 0 //����û������
														&& (Current_card_type == 0 //û����ʱ��Ϣ��
																|| (Current_card_type == 1
																		&& strcmp(Current_Credit.CARD_ID, "111111111111111111")!=0
																		//&& strcmp(Current_Credit.CARD_ID, "222222222222222222")!=0
																		&& strcmp(Current_Credit.CARD_ID, "333333333333333333")!=0
																	 ))//�����õ�ʱ��Ϣ��
														&& strcmp(REAL_DATA_PARAM.TankLockStatus, "1")==0 //��ƿ���ŵ�ʱ��
											)
										{
												LCD_Delay_Time--;
											  //printf("LCD_Delay_Time: %d\r\n",LCD_Delay_Time);
										}
										//printf("LCD_Delay_Time %d\r\n",LCD_Delay_Time);
										{
												if(sStep == 1)
												{
														//��Ļ����һֱ���Ե�ѹ,������Ե�ѹ
														BATAD_POWER(1);
														IsTestPower = true;

														if(Current_card_type == 0) //û�п���ʱ���RFID,ʵ����������ʱ��رշ���,�������ڶ��̻߳�û��ˢ�����ֵ
																//|| (Current_card_type == 1 && strcmp(REAL_DATA_PARAM.ElectricValveStatus, "0")==0) //�û��������ǹرյĴ�RFID����
																//|| Current_card_type == 2)
														{
																IsTestRFID = true;
														}

														//��Ļ������
														CS_W(1);
														RD_W(0);
														WR_W(0);
														DAT_W(0);
														osDelay(20);//��ʼ�����ź���Ҫ��ʱ�ϵ�,����Ứ��
														LCD_POWER(1);
														LCD_LIGHT(1);
														LCD_Init();

														sStep++;
												}

												if(Current_card_type == 0 && sStep >= 4) //�޿�
												{
														sStep = 2;
												}
												if((Current_card_type == 2) && sStep >= 3)//������
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

												if(sStep == 2)//����1
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

																if(Current_card_type == 0 || Current_card_type == 2 || Current_card_type == 4)//�޿�,����,����
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
																else if(Current_card_type == 3)//У׼��ʱ���ADֵ
																{
																		LCDPanelStruct.DisNumber = ADC_Data[0];
																		LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																		LCD_Refresh(&LCDPanelStruct);
																}

																LCD_Refresh_Time = 0;
														}
												}

												if(sStep == 3)//����2
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
																//											if(Current_card_type == 3)//У׼����ۼ�����,�����洢У׼����,���������,������֤����
																//											{
																//											  LCDPanelStruct.num = REAL_DATA_Credit.CumulationGasL * 10;
																//												LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																//												LCDPanelStruct.POINT6_Sign = ON;
																//												LCD_Refresh(&LCDPanelStruct);
																//											}
																if(Current_card_type == 0 || Current_card_type == 2 || Current_card_type == 4)//�޿�,����,����
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
																if(Current_card_type == 3)//У׼��ĵ�ѹ
																{
																		LCDPanelStruct.DisNumber = ADTestVFlow * 1000;
																		LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																		LCDPanelStruct.POINT3_Sign = ON;
																		LCDPanelStruct.KgSign_Sign = OFF;
																		LCD_Refresh(&LCDPanelStruct);
																}
																if(Current_card_type == 1)//ʹ�õ�ǧ����
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

												if(sStep == 4)//����3 ���⿨��ʾ��ѹֵ
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
																		LCDPanelStruct.DisNumber = (ADTestVFlow) * 1000;//��ѹֵ + flow_zero
																		LCDPanelStruct.RFID_Sign = (IsTestRFID == true ? ON : OFF);
																		LCDPanelStruct.POINT3_Sign = ON;
																		LCDPanelStruct.KgSign_Sign = OFF;
																		LCD_Refresh(&LCDPanelStruct);
																}

																LCD_Refresh_Time = 0;
														}
												}

												if(sStep == 5)//����4 ���⿨��ʾʵʱ����
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
																		LCDPanelStruct.DisNumber = TmpRealTimeFlow * 1000;//��ΪУ׼�����������ú�������,������ʵʱ����
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

										//					CS_W(0);//���ڵ͹���Ӱ�첻��
										//					RD_W(0);
										//					WR_W(0);
										//					DAT_W(0);

										//��Ļ�𲻲��Ե�ѹ
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

        /*�ӽ�����������ȡ����*/
        for(i=0; i<CMD_LEN; i++)
        {
            if((i>0)&&((cmd_analyze.processed_buf[i]==' ')||(cmd_analyze.processed_buf[i]==0x0D)))
            {
                cmd_buf[i]='\0';        //�ַ���������
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
            if(strcmp(cmd_buf,cmd_list[i].cmd_name)==0)       //�ַ������
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

#define M26GETCOMMANDLEN 9
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
/*0*/			{     	1,			 "AT\r\n",																	Analysis_AT_Cmd							},
/*1*/			{     	2,			 "AT+CSQ\r\n",															Analysis_CSQ_Cmd						},
/*2*/			{     	3,			 "AT+CGREG?\r\n",														Analysis_CGREG_Cmd					},
/*3*/			{     	4,			 "AT+CGACT?\r\n",														Analysis_CGACT_Cmd					},
/*4*/			{     	5,			 NULL,																			Analysis_CHTTPCREATE_Cmd		}, //AT+CHTTPCREATE="https://ateei9d448.execute-api.eu-west-1.amazonaws.com/"
/*5*/			{     	6,			 "AT+CHTTPCON=0\r\n",												Analysis_CHTTPCON_Cmd				},//AT+CHTTPSEND=0,0,"/testing/meter/settings/TZ00000525"
/*6*/			{     	7,			 NULL,																			Analysis_CHTTPSEND_Cmd			},
/*7*/			{     	8,			 "AT+CHTTPDISCON=0\r\n",										Analysis_CHTTPDISCON_Cmd		},
/*8*/			{     	9,			 "AT+CHTTPDESTROY=0\r\n",										Analysis_CHTTPDESTROY_Cmd		},
};


uint8_t u8GetNum[M26GETCOMMANDLEN]= {0,1,2,3,4,5,6,7,8};
uint8_t u8PostNum[M26POSTCOMMANDLEN] = {0,1,2,3,4,5,6,7,8,9,13,14,15,11,12};
uint8_t u8SniNum[3] = {0,1,5};

//type:command,parameters,cookingSession,hardware,info,warning
//��������ָ�룬���ú�ʹ�����Ӧ�ͷ�
static char * Sever_Address_GET(char *severaddr)
{
	uint8_t u8Lenth = 0;
	char* ptUrlInfo;
	
	u8Lenth = strlen(severaddr);
	ptUrlInfo = (char *)malloc(u8Lenth*sizeof(char)+strlen("AT+CHTTPCREATE=")+10);
	if(ptUrlInfo != NULL)//����ɹ�
	{
		strcat(ptUrlInfo,"AT+CHTTPCREATE=");
		strcat(ptUrlInfo,"\"");
		strcat(ptUrlInfo,severaddr);
		strcat(ptUrlInfo,"\"");
		strcat(ptUrlInfo,"\r\n");
	}	
	return ptUrlInfo;
}

static char * HTTPS_Get_Data(char *version,char *meterid)
{
	char* ptUrlInfo;

	ptUrlInfo = (char *)malloc(strlen(version)+strlen(meterid)+strlen("AT+CHTTPSEND=0,0,")+15);
	if(ptUrlInfo != NULL)//����ɹ�
	{
		strcat(ptUrlInfo,"AT+CHTTPSEND=0,0,");
		strcat(ptUrlInfo,"\"/");
		strcat(ptUrlInfo,version);
		strcat(ptUrlInfo,meterid);
		strcat(ptUrlInfo,"\"");
		strcat(ptUrlInfo,"\r\n");
	}	
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

static void SendGetCommand()
{
	uint8_t i = 0;
	uint32_t Waiting_ms = 0;
//	for(i=0;i< sizeof(Send_AT_cmd)/sizeof(Send_AT_cmd[0]);i++)M26GETCOMMANDLEN
	for(i=0;i< M26GETCOMMANDLEN;i++)
	{
//		if((u8GetNum[i]!=5))
		{
			printf("send:%s\r\n\r\n",Send_AT_cmd[u8GetNum[i]].SendCommand);
			Sim80x.AtCommand.FindAnswer = 0;
			xQueueSend(SendATQueue,(void *) &Send_AT_cmd[u8GetNum[i]].u8CmdNum,(TickType_t)10);	 
			Sim80x_SendAtCommand(Send_AT_cmd[u8GetNum[i]].SendCommand,1000,1,"OK\r\n");
			osDelay(2000);
			while(!Sim80x.AtCommand.FindAnswer)
			{
				if((i!=4)&&(i!=5)&&(i!=6))
				Sim80x_SendAtCommand(Send_AT_cmd[u8GetNum[i]].SendCommand,1000,1,"OK\r\n");
				osDelay(2000);
			}
		}
		
	}
}
QueueHandle_t SendATQueue = NULL;
SemaphoreHandle_t  Semaphore_Uart_Rec = NULL;
void AppObjCreateQueue (void)
{
	/* ����10��uint8_t����Ϣ���� */
	SendATQueue = xQueueCreate(1, sizeof(uint8_t));
    if( SendATQueue == 0 )
    {
        printf("create failed\r\n");
    }
}
void AppObjCreate (void)
{
	/* ������ֵ�ź������״δ����ź�������ֵ��0 */
	Semaphore_Uart_Rec = xSemaphoreCreateBinary();
	
	if(Semaphore_Uart_Rec == NULL)
	{
		printf("Semaphore creat failed!\r\n");
			/* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
	}
}


void Sim7020Etest_Task(void const * argument)
{
	Stru_Sever_Info_t *struSeverInfo;
	uint8_t result = 0 , i = 0; //���ñ�ʶ���Ƿ���Ӧ�˵�ǰ��ָ��
	uint32_t count_times = 1;
	char *ptUrl,*ptData;
	char *ptPostData;
	volatile uint16_t u16UrlLength = 0;
	
	Send_AT_cmd[14].SendCommand =(char *)malloc(20);
	struSeverInfo = (struct SeverInfo *) malloc(sizeof(struct SeverInfo));

	HAL_GPIO_WritePin(_SIM80X_POWER_KEY_GPIO,_SIM80X_POWER_KEY_PIN,GPIO_PIN_RESET);
						
	HAL_GPIO_WritePin(_SIM80X_POWER_KEY_GPIO,_SIM80X_POWER_KEY_PIN,GPIO_PIN_SET);
	HAL_UART_Receive_IT(&_SIM80X_USART,&Sim80x.UsartRxTemp,1);
	while(1)
	{
		struSeverInfo->Sendsever = SEVER_URL;
		u16UrlLength = strlen(struSeverInfo->Sendsever);
		struSeverInfo->SeverVer = SEVER_VERSION;
		struSeverInfo->CardID = "";
		
		struSeverInfo->MeterId = "/meter/settings/TZ00000525";		
		ptUrl = Sever_Address_GET(struSeverInfo->Sendsever);
		u16UrlLength = strlen(ptUrl);
		Send_AT_cmd[4].SendCommand = ptUrl;
		
		ptData = HTTPS_Get_Data(struSeverInfo->SeverVer,struSeverInfo->MeterId);
		u16UrlLength = strlen(ptData);
		Send_AT_cmd[6].SendCommand = ptData;
		SendGetCommand();
		
		free(ptUrl);
		free(ptData);

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

uint8_t Analysis_CGREG_Cmd(char *pdata)
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
uint8_t Analysis_CGACT_Cmd(char *pdata)
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
uint8_t Analysis_CHTTPCREATE_Cmd(char *pdata)	
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
uint8_t Analysis_CHTTPSEND_Cmd(char *pdata)
{
	char *ptStrStart ;
	char *ptFindResult ;
	ptStrStart = (char*)Sim80x.UsartRxBuffer;
	ptFindResult = strstr(ptStrStart,"+CHTTPNMIC");
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
uint8_t Analysis_CHTTPCON_Cmd(char *pdata)
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
uint8_t Analysis_CHTTPDISCON_Cmd	(char *pdata)
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

uint8_t Analysis_CHTTPDESTROY_Cmd(char *pdata)
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
