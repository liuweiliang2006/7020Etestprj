#include "WaringPacket.h"
#include "package.h"
#include "string.h"
#include "rtc.h"
#include "key.h"
#include "Sim80x.h"
#include "StringUtils.h"
#include "LogUtils.h"

waringPacket_t waringPacket;
extern CONFIG_Meter_t CONFIG_Meter;
extern REAL_DATA_PARAM_t REAL_DATA_PARAM;
extern REAL_DATA_Credit_t REAL_DATA_Credit; //仪表充值减值信息
extern Current_Credit_t Current_Credit;
extern uint16_t identCnt;
extern uint32_t sumCnt;
//给报警结构体赋值
void refreshWaringPacket(waringPacket_t *wPacket)
{
	  sumCnt = 0;
		long date,time;//UTC
	  GetRTC(&time,&date);
		char datatime[12];
	  sprintf(datatime,"%08ld%04ld",date,time / 100);
    strncpy(wPacket->meterNumer,CONFIG_Meter.MeterNo,10);

//	  for(int i = 0;i<32;i++)
//	  {
//		  printf("CUSTOMER_ID %X\r\n",REAL_DATA_PARAM.CUSTOMER_ID[i]);
//		}
//		printf("CUSTOMER_ID %s\r\n", REAL_DATA_PARAM.CUSTOMER_ID);
	
	  if(stringIsNull(REAL_DATA_PARAM.CUSTOMER_ID,7) == FAILED)
		{
			strncpy(wPacket->CUSTOMER_ID,"XXXXXXX",7);
		}
		else
		{
			strncpy(wPacket->CUSTOMER_ID,REAL_DATA_PARAM.CUSTOMER_ID,7);
		}
	  sumCnt += identCnt;
    sprintf(wPacket->identifier,"%04d",identCnt++);
	  
	  strncpy(wPacket->Low_battery,REAL_DATA_PARAM.BatVoltage > CONFIG_Meter.LowBattery ? "0" : "1",1);
		//printf("Current_Credit.CurrCredit %f\r\n" ,Current_Credit.CurrCredit);
		//printf("CONFIG_Meter.LowCredit %f\r\n" ,CONFIG_Meter.LowCredit);
    strncpy(wPacket->low_amount_of_prepaid_credit,Current_Credit.CurrCredit > CONFIG_Meter.LowCredit ? "0" : "1",1);
	  strncpy(wPacket->Low_gas_amount,REAL_DATA_PARAM.TankQuality > CONFIG_Meter.LowGasVolume ? "0" : "1",1);
	  strncpy(wPacket->Illegal_lid_opening,(Opening_input == 1) ? "1" : "0",1);// && strcmp(REAL_DATA_PARAM.LidElectricLock,"1") == 0
	  strncpy(wPacket->Electric_Lock_status,REAL_DATA_PARAM.LidElectricLock,1);
		strncpy(wPacket->NEEDLE_Sensor_status,REAL_DATA_PARAM.NEEDLESensorStatus,1);
	  strncpy(wPacket->Electric_Valve_status,REAL_DATA_PARAM.ElectricValveStatus,1);
	  strncpy(wPacket->tank_Sensor_status,REAL_DATA_PARAM.TankSensorStatus,1);
	  strncpy(wPacket->tank_Lock_status,REAL_DATA_PARAM.TankLockStatus,1);
	  sumCnt += REAL_DATA_PARAM.GprsCsq;
	  sprintf(wPacket->GSM_signal_intensity,"%2d",REAL_DATA_PARAM.GprsCsq);
		sumCnt += 25;
	  sprintf(wPacket->GAS_TEMPERATURE,"%2d",25);
		sumCnt += atoi(datatime);
    strncpy(wPacket->datetime,datatime,12);
    //strncpy(wPacket->verification,"234",3);
}
//TOTAL

//编码报警包
void encodeWaringPacket(char *sendMeagess,waringPacket_t *waringPackage)
{
    strncat(sendMeagess,"##",2);
	  strncat(sendMeagess,"WARN",4);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,waringPackage->meterNumer,10);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,waringPackage->CUSTOMER_ID,7);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,waringPackage->identifier,4);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,waringPackage->Low_battery,1);
	  strncat(sendMeagess,"/",1);
    strncat(sendMeagess,waringPackage->low_amount_of_prepaid_credit,1);
	  strncat(sendMeagess,"/",1);
    strncat(sendMeagess,waringPackage->Low_gas_amount,1);
	  strncat(sendMeagess,"/",1);
    strncat(sendMeagess,waringPackage->Illegal_lid_opening,1);
	  strncat(sendMeagess,"/",1);
    strncat(sendMeagess,waringPackage->Electric_Lock_status,1);
	  strncat(sendMeagess,"/",1);
    strncat(sendMeagess,waringPackage->NEEDLE_Sensor_status,1);
	  strncat(sendMeagess,"/",1);
    strncat(sendMeagess,waringPackage->Electric_Valve_status,1);
		strncat(sendMeagess,"/",1);
    strncat(sendMeagess,waringPackage->tank_Sensor_status,1);
    strncat(sendMeagess,"/",1);
		strncat(sendMeagess,waringPackage->tank_Lock_status,1);
    strncat(sendMeagess,"/",1);
		strncat(sendMeagess,waringPackage->GSM_signal_intensity,2);
    strncat(sendMeagess,"/",1);
		strncat(sendMeagess,waringPackage->GAS_TEMPERATURE,2);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,waringPackage->datetime,12);
    strncat(sendMeagess,"/",1);
    sprintf(waringPackage->verification,"%04X",crc16(sendMeagess + 2,strlen(sendMeagess) - 2));
    strncat(sendMeagess,waringPackage->verification,4);
    strncat(sendMeagess,"**",2);
}

//####################################################################################################
//发送报警
void SendWarnPacket(void)
{
	char sendBuffer[200]={0};
	refreshWaringPacket(&waringPacket);
	encodeWaringPacket(sendBuffer,&waringPacket);
  //char sendBuffer[100]={"11111111"};
	if(GPRS_TcpSendMessage(sendBuffer))
	{
			//printf("GPRS_TcpSendMessage ---------> OK\r\n");
			//printf("SendWarnPacket: %s \r\n",sendBuffer);
		  LogWrite(Send,"WARN",sendBuffer);
	}
	else
	{
			//printf("GPRS_TcpSendMessage ---------> ERROR");
			//Sim80x.GPRS.Connection[0] = GPRSConnection_ConnectFail;
		  LogWrite(Send,"WARNError",sendBuffer);
	}
	//memset(sendBuffer,0,sizeof(sendBuffer));
}
