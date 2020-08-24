#include "HeartbeatPacket.h"
#include "package.h"
#include "string.h"
#include "rtc.h"
#include "Sim80x.h"
#include "LogUtils.h"
#include "StringUtils.h"

heartbeatPackage_t heartbeatPackage;
extern CONFIG_Meter_t CONFIG_Meter;
extern uint16_t identCnt;
extern uint32_t sumCnt;
//给心跳包赋值
void refreshHeartbeatPacket(heartbeatPackage_t *htPackage)
{
	  sumCnt = 0;
	  long date,time;//UTC
	  GetRTC(&time,&date);
		char datatime[12];
	  sprintf(datatime,"%08ld%04ld",date,time / 100);
//	stmerncpy(waringPackage->meterNumer,Sim80x.IMEI,15);
    strncpy(htPackage->meterNumer,CONFIG_Meter.MeterNo,10);
	  sumCnt += identCnt;
    sprintf(htPackage->identifier,"%04d",identCnt++);
	  sumCnt += atoi(datatime);
    strncpy(htPackage->datetime,datatime,12);
    //strncpy(htPackage->verification,"234",3);
}
//编码心跳包
void encodeHeartbeatPacket(char *sendMeagess,heartbeatPackage_t *htPackage)
{
	  strncat(sendMeagess,"##",2);
	  strncat(sendMeagess,"BEAT",4);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,htPackage->meterNumer,10);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,htPackage->identifier,4);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,htPackage->datetime,12);
    strncat(sendMeagess,"/",1);
	  //printf("sumCnt  %d\r\n", sumCnt);
	  sprintf(htPackage->verification,"%04X",crc16(sendMeagess + 2,strlen(sendMeagess) - 2));
    strncat(sendMeagess,htPackage->verification,4);
    strncat(sendMeagess,"**",2);
	
}

//####################################################################################################
//发送心跳包
void SendHeartbeatPacket(void)
{
	char sendBuffer[200]={0};
	refreshHeartbeatPacket(&heartbeatPackage);
	encodeHeartbeatPacket(sendBuffer,&heartbeatPackage);

	if(GPRS_TcpSendMessage(sendBuffer))
	{
			//printf("GPRS_TcpSendMessage ---------> OK\r\n");
			//printf("SendHeartbeatPacket: %s \r\n",sendBuffer);
		  LogWrite(Send,"BEAT",sendBuffer);
	}
	else
	{
			//printf("GPRS_TcpSendMessage ---------> ERROR");
			//Sim80x.GPRS.Connection[0] = GPRSConnection_ConnectFail;
		  LogWrite(Send,"BEATError",sendBuffer);
	}
	//memset(sendBuffer,0,sizeof(sendBuffer));
}
