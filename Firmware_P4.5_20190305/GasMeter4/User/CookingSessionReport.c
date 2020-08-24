#include "CookingSessionReport.h"
#include "package.h"
#include "string.h"
#include "rtc.h"
#include "Sim80x.h"
#include "LogUtils.h"
#include "StringUtils.h"

CookingSessionReport_t CookingSessionReport;
extern CONFIG_Meter_t CONFIG_Meter;
extern REAL_DATA_Credit_t REAL_DATA_Credit;
//uint16_t identCntCSRP;
//extern uint32_t sumCnt;

//赋值
void refreshCookingSessionReport(CookingSessionReport_t *htPackage)
{
	  //sumCnt = 0;
	  long date,time;//UTC
	  GetRTC(&time,&date);
		char datatime[12];
	  sprintf(datatime,"%08ld%04ld",date,time / 100);
//	stmerncpy(waringPackage->meterNumer,Sim80x.IMEI,15);
    strncpy(htPackage->meterNumer,CONFIG_Meter.MeterNo,10);
	  //sumCnt += identCnt;
    sprintf(htPackage->identifier,"%04d",REAL_DATA_Credit.CookingSessionSendNumber);
		//sumCnt += atoi(datatime);
    strncpy(htPackage->datetime,datatime,12);
    //strncpy(htPackage->verification,"234",3);
}
//编码
void encodeCookingSessionReport(char *sendMeagess,CookingSessionReport_t *htPackage)
{
	  strncat(sendMeagess,"##",2);
	  strncat(sendMeagess,"CSRP",4);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,CONFIG_Meter.MeterNo,10);
    strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,htPackage->CUSTOMER_ID,7);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,htPackage->identifier,4);
    strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,htPackage->CARD_ID,18);
    strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,htPackage->SESSION_END_TYPE,2);
    strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,htPackage->GAS_REMAINING,6);
    strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,htPackage->START_CUMULATIVE_VOLUME,12);
    strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,htPackage->END_CUMULATIVE_VOLUME,12);
    strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,htPackage->SESSION_START_TIME,14);
    strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,htPackage->SESSION_END_TIME,14);
    strncat(sendMeagess,"/",1);
		strncat(sendMeagess,htPackage->CREDIT_SESSION_START,12);
    strncat(sendMeagess,"/",1);
		strncat(sendMeagess,htPackage->CREDIT_SESSION_END,12);
    strncat(sendMeagess,"/",1);
		strncat(sendMeagess,htPackage->UnitPrice,7);
    strncat(sendMeagess,"/",1);
		strncat(sendMeagess,htPackage->GasDensity,5);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,htPackage->datetime,12);
    strncat(sendMeagess,"/",1);
	  sprintf(htPackage->verification,"%04X",crc16(sendMeagess + 2,strlen(sendMeagess) - 2));
    strncat(sendMeagess,htPackage->verification,4);
    strncat(sendMeagess,"**",2);
}

//####################################################################################################
//发送做完饭的数据包
bool SendCookingSessionPacket()
{
	char sendBuffer[200]={0};
	refreshCookingSessionReport(&CookingSessionReport);
	encodeCookingSessionReport(sendBuffer,&CookingSessionReport);
	//char sendBuffer[120]={"##CSRP/TZ12345678/1234567/1234/01/9.12/9.99/201809031353/201809031412/9.12/8.88/5.12/2.072/201809031353/345**"};
	
	if(GPRS_TcpSendMessage(sendBuffer))
	{
			//printf("GPRS_TcpSendMessage ---------> OK\r\n");
			//printf("SendCookingSessionPacket: %s \r\n",sendBuffer);
		  //REAL_DATA_Credit.CookingSessionSendNumber++;
		  LogWrite(Send,"CSRP",sendBuffer);
		return true;
	}
	else
	{
			//printf("GPRS_TcpSendMessage ---------> ERROR");
			Sim80x.GPRS.Connection[0] = GPRSConnection_ConnectFail;
		  LogWrite(Send,"CSRPError",sendBuffer);
		return false;
	}
	//memset(sendBuffer,0,sizeof(sendBuffer));
}

