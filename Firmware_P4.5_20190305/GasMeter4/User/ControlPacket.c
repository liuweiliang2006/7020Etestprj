#include "ControlPacket.h"
#include "package.h"
#include "string.h"
#include "rtc.h"
#include "Sim80x.h"
#include "LogUtils.h"
#include "StringUtils.h"

ControlPacket_t controlPacket;
bool IsNeedRepayControl = false;

extern CONFIG_Meter_t CONFIG_Meter;
extern uint16_t identCnt;
//extern uint32_t sumCnt;
//给控制需要回复的包赋值
void refreshControlPacket(ControlPacket_t *rPacket)
{
	  //sumCnt = 0;
	  long date,time;//UTC
	  GetRTC(&time,&date);
		char datatime[12];
	  sprintf(datatime,"%08ld%04ld",date,time / 100);
//	stmerncpy(waringPackage->meterNumer,Sim80x.IMEI,15);
    strncpy(rPacket->meterNumer,CONFIG_Meter.MeterNo,10);
    sprintf(rPacket->identifier,"%04d",identCnt++);
    strncpy(rPacket->datetime,datatime,12);

}
//编码控制回复包
void encodeControlPacket(char *sendMeagess,ControlPacket_t *rPacket)
{
	  strncat(sendMeagess,"##",2);
	  strncat(sendMeagess,"CONT",4);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,rPacket->meterNumer,10);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,rPacket->identifier,4);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,rPacket->datetime,12);
    strncat(sendMeagess,"/",1);
	  sprintf(rPacket->verification,"%04X",crc16(sendMeagess + 2,strlen(sendMeagess) - 2));
    strncat(sendMeagess,rPacket->verification,4);
    strncat(sendMeagess,"**",2);
}

//####################################################################################################
void SendControlPacket(void)
{
	char sendBuffer[200]={0};
	refreshControlPacket(&controlPacket);
	encodeControlPacket(sendBuffer,&controlPacket);

	if(GPRS_TcpSendMessage(sendBuffer))
	{
			//printf("GPRS_TcpSendMessage ---------> OK\r\n");
			//printf("SendControlPacket: %s \r\n",sendBuffer);
		
		  LogWrite(Send,"ReplyCONT",sendBuffer);
	}
	else
	{
			//printf("GPRS_TcpSendMessage ---------> ERROR");
			//Sim80x.GPRS.Connection[0] = GPRSConnection_ConnectFail;
		  LogWrite(Send,"ReplyCONTError",sendBuffer);
	}
	//memset(sendBuffer,0,sizeof(sendBuffer));
}

