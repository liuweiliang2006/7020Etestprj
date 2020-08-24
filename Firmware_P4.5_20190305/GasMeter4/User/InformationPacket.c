#include "InformationPacket.h"
#include "package.h"
#include "string.h"
#include "rtc.h"
#include "key.h"
#include "Sim80x.h"
#include "StringUtils.h"
#include "LogUtils.h"

InformationPacket_t InformationPacket;
extern CONFIG_Meter_t CONFIG_Meter;
extern REAL_DATA_PARAM_t REAL_DATA_PARAM;
extern uint16_t identCnt;
extern uint32_t sumCnt;
//
void refreshInformationPacket(InformationPacket_t *rPacket)
{
	  sumCnt = 0;
		long date,time;//UTC
	  GetRTC(&time,&date);
		char datatime[12];
	  sprintf(datatime,"%08ld%04ld",date,time / 100);
    strncpy(rPacket->meterNumer,CONFIG_Meter.MeterNo,10);
	  
    if(stringIsNull(REAL_DATA_PARAM.CUSTOMER_ID,7) == FAILED)
		{
			strncpy(rPacket->CUSTOMER_ID,"XXXXXXX",7);
		}
		else
		{
			strncpy(rPacket->CUSTOMER_ID,REAL_DATA_PARAM.CUSTOMER_ID,7);
		}
	  sumCnt += identCnt;
    //sprintf(rPacket->identifier,"%04d",identCnt++);
	  sprintf(rPacket->FIRMWARE_V,CONFIG_Meter.FIRMWARE_V,4);
	  sprintf(rPacket->PCB_V,CONFIG_Meter.PCB_V,14);
	  strncpy(rPacket->CURRENCY,CONFIG_Meter.CURRENCY,3);
	  sprintf(rPacket->GAS_REMAINING,"%.3f",REAL_DATA_PARAM.TankQuality);
		printf("CYLINDER_No : %s\r\n", CONFIG_Meter.CYLINDER_No);
//	  if(stringIsNull(CONFIG_Meter.CYLINDER_No,11) == FAILED)
//		{
//			strncpy(rPacket->CYLINDER_No,"XXXXXXXXXXX",11);
//		}
//		else
		{
			strncpy(rPacket->CYLINDER_No,CONFIG_Meter.CYLINDER_No,11);
		}
	  sprintf(rPacket->UpdatePeriod,"%u",CONFIG_Meter.UpDuty);
		sprintf(rPacket->StartPeriod,"%u",CONFIG_Meter.StartDuty);
	  strncpy(rPacket->BATTERY_MODEL,CONFIG_Meter.BATTERY_MODEL,5);
	  
		sumCnt += atoi(datatime);
    strncpy(rPacket->datetime,datatime,12);
}

void encodeInformationPacket(char *sendMeagess,InformationPacket_t *rPacket)
{
    strncat(sendMeagess,"##",2);
	  strncat(sendMeagess,"INFS",4);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,rPacket->meterNumer,10);
    strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->CUSTOMER_ID,10);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,rPacket->identifier,4);
    strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,FIRMWARE_Version,11);
	  strncat(sendMeagess,rPacket->FIRMWARE_V,4);
	  strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->PCB_V,14);
	  strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->CURRENCY,3);
	  strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->GAS_REMAINING,6);
	  strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,"XXX",3);
	  strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->CYLINDER_No,11);
	  strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->UpdatePeriod,5);
	  strncat(sendMeagess,"/",1);
		strncat(sendMeagess,rPacket->StartPeriod,5);
	  strncat(sendMeagess,"/",1);
		strncat(sendMeagess,rPacket->BATTERY_MODEL,5);
	  strncat(sendMeagess,"/",1);
    strncat(sendMeagess,rPacket->datetime,12);
    strncat(sendMeagess,"/",1);
    sprintf(rPacket->verification,"%04X",crc16(sendMeagess + 2,strlen(sendMeagess) - 2));
    strncat(sendMeagess,rPacket->verification,4);
    strncat(sendMeagess,"**",2);
}

//####################################################################################################
//
void SendInformationPacket(void)
{
	char sendBuffer[200]={0};
	refreshInformationPacket(&InformationPacket);
	encodeInformationPacket(sendBuffer,&InformationPacket);
  //char sendBuffer[120]={"##TZ00000015/DATA/0000/6253.00/4.00/100/0.00/4000.00/2.484/15.000/0005/1/0/1/0/1/1/16/181211171026/234**"};
	if(GPRS_TcpSendMessage(sendBuffer))
	{
			//printf("GPRS_TcpSendMessage ---------> OK\r\n");
			//printf("SendInformationPacket: %s \r\n",sendBuffer);
		  LogWrite(Send,"INFS",sendBuffer);
	}
	else
	{
			//printf("GPRS_TcpSendMessage ---------> ERROR");
			//Sim80x.GPRS.Connection[0] = GPRSConnection_ConnectFail;
		  LogWrite(Send,"INFSError",sendBuffer);
	}
	//memset(sendBuffer,0,sizeof(sendBuffer));
}

