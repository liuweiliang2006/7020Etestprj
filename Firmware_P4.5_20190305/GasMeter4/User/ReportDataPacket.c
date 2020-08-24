#include "ReportDataPacket.h"
#include "package.h"
#include "string.h"
#include "rtc.h"
#include "key.h"
#include "Sim80x.h"
#include "StringUtils.h"
#include "LogUtils.h"

reportDataPacket_t reportDataPacket;
extern CONFIG_Meter_t CONFIG_Meter;
extern REAL_DATA_PARAM_t REAL_DATA_PARAM;
extern REAL_DATA_Credit_t REAL_DATA_Credit;//仪表充值减值信息 
extern Current_Credit_t Current_Credit;
extern uint16_t identCnt;
//extern uint32_t sumCnt;
//
void refreshReportDataPacket(reportDataPacket_t *rPacket)
{
	  //sumCnt = 0;
		long date,time;//UTC
	  GetRTC(&time,&date);
		char datatime[12];
	  sprintf(datatime,"%08ld%04ld",date,time / 100);
    strncpy(rPacket->meterNumer,CONFIG_Meter.MeterNo,10);
	  //sumCnt += identCnt;
    sprintf(rPacket->identifier,"%04d",identCnt++);
	  //sumCnt += REAL_DATA_Credit.CumulationGasL;
	  sprintf(rPacket->CumulativeVolumetricUsage,"%.2f",REAL_DATA_Credit.CumulationGasL);
	  //sumCnt += REAL_DATA_PARAM.TankQuality;
	  sprintf(rPacket->TankQuality,"%.3f",REAL_DATA_PARAM.TankQuality);
	  sprintf(rPacket->CREDIT,"%.2f",Current_Credit.CurrCredit);
//	  if(stringIsNull(CONFIG_Meter.CYLINDER_No,11) == FAILED)
//		{
//			strncpy(rPacket->CYLINDER_No,"XXXXXXXXXXX",11);
//			printf("1 : %s",CONFIG_Meter.CYLINDER_No);
//		}
//		else
		{
			strncpy(rPacket->CYLINDER_No,CONFIG_Meter.CYLINDER_No,11);
			//printf("2 : %s",CONFIG_Meter.CYLINDER_No);
		}
		//sumCnt += atoi(datatime);
    strncpy(rPacket->datetime,datatime,12);
}

void encodeReportDataPacket(char *sendMeagess,reportDataPacket_t *rPacket)
{
    strncat(sendMeagess,"##",2);
	  strncat(sendMeagess,"DATA",4);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,rPacket->meterNumer,10);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,rPacket->identifier,4);
    strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->CumulativeVolumetricUsage,12);
	  strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->TankQuality,6);
	  strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->CREDIT,12);
	  strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->CYLINDER_No,11);
	  strncat(sendMeagess,"/",1);
    strncat(sendMeagess,rPacket->datetime,12);
    strncat(sendMeagess,"/",1);
    sprintf(rPacket->verification,"%04X",crc16(sendMeagess + 2,strlen(sendMeagess) - 2));
    strncat(sendMeagess,rPacket->verification,4);
    strncat(sendMeagess,"**",2);
}

//####################################################################################################
//
bool SendReportDataPacket(void)
{
	char sendBuffer[200]={0};
	refreshReportDataPacket(&reportDataPacket);
	encodeReportDataPacket(sendBuffer,&reportDataPacket);
  //char sendBuffer[120]={"##TZ00000015/DATA/0000/6253.00/4.00/100/0.00/4000.00/2.484/15.000/0005/1/0/1/0/1/1/16/181211171026/234**"};
	if(GPRS_TcpSendMessage(sendBuffer))
	{
			//printf("GPRS_TcpSendMessage ---------> OK\r\n");
			//printf("SendReportDataPacket: %s \r\n",sendBuffer);
		  LogWrite(Send,"DATA",sendBuffer);
		return true;
	}
	else
	{
			//printf("GPRS_TcpSendMessage ---------> ERROR");
			Sim80x.GPRS.Connection[0] = GPRSConnection_ConnectFail;
		  LogWrite(Send,"DATAError",sendBuffer);
		return false;
	}
	//memset(sendBuffer,0,sizeof(sendBuffer));
}

