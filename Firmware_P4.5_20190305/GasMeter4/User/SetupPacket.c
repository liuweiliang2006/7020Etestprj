#include "SetupPacket.h"
#include "package.h"
#include "string.h"
#include "rtc.h"
#include "Sim80x.h"
#include "LogUtils.h"
#include "StringUtils.h"

SetupPacket_t SetupPacket;
bool IsNeedRepaySetup = false;

extern CONFIG_Meter_t CONFIG_Meter;
extern REAL_DATA_PARAM_t REAL_DATA_PARAM;  
extern uint16_t identCnt;
extern uint32_t sumCnt;
//给设置需要回复的包赋值
void refreshSetupPacket(SetupPacket_t *sPacket)
{
	  sumCnt = 0;
	  long date,time;//UTC
	  GetRTC(&time,&date);
		char datatime[12];
	  sprintf(datatime,"%08ld%04ld",date,time / 100);
//	stmerncpy(waringPackage->meterNumer,Sim80x.IMEI,15);
    strncpy(sPacket->meterNumer,CONFIG_Meter.MeterNo,10);
	  sumCnt += identCnt;
    //sprintf(sPacket->identifier,"%04d",identCnt++);
	  sumCnt += atoi(datatime);
    strncpy(sPacket->datetime,datatime,12);
    //strncpy(sPacket->verification,"234",3);
}
//编码设置回复包
void encodeSetupPacketReply(char *sendMeagess,SetupPacket_t *sPacket)
{
	  strncat(sendMeagess,"##",2);
	  strncat(sendMeagess,"STUP",4);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,sPacket->meterNumer,10);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,sPacket->identifier,4);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,sPacket->datetime,12);
    strncat(sendMeagess,"/",1);
    sprintf(sPacket->verification,"%04X",crc16(sendMeagess + 2,strlen(sendMeagess) - 2));
    strncat(sendMeagess,sPacket->verification,4);
    strncat(sendMeagess,"**",2);
}

//编码设置
void encodeSetupPacket(char *sendMeagess,SetupPacket_t *sPacket)
{
	  strncat(sendMeagess,"##",2);
	  strncat(sendMeagess,"STUP",4);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,sPacket->meterNumer,10);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,sPacket->identifier,4);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,sPacket->datetime,12);
    strncat(sendMeagess,"/",1);
    sprintf(sPacket->verification,"%04X",crc16(sendMeagess + 2,strlen(sendMeagess) - 2));
    strncat(sendMeagess,sPacket->verification,4);
    strncat(sendMeagess,"**",2);
}

//####################################################################################################
void SendSetupPacket(void)
{
	char sendBuffer[200]={0};
	
	//encodeSetupPacket(sendBuffer,&SetupPacket);
	//LogWrite(Receive,"STUP",sendBuffer);
  //memset(sendBuffer,0,sizeof(sendBuffer));
	
	refreshSetupPacket(&SetupPacket);
	encodeSetupPacketReply(sendBuffer,&SetupPacket);

	if(GPRS_TcpSendMessage(sendBuffer))
	{
			//printf("GPRS_TcpSendMessage ---------> OK\r\n");
			//printf("SendSetupPacket: %s \r\n",sendBuffer);
		  LogWrite(Send,"ReplySTUP",sendBuffer);
	}
	else
	{
			//printf("GPRS_TcpSendMessage ---------> ERROR");
			//Sim80x.GPRS.Connection[0] = GPRSConnection_ConnectFail;
		  LogWrite(Send,"ReplySTUPError",sendBuffer);
	}
	//memset(sendBuffer,0,sizeof(sendBuffer));
}


//设置到仪表
void SetToMeter(SetupPacket_t *sPacket)
{
	 sscanf(sPacket->UpdatePeriod,"%u",&CONFIG_Meter.UpDuty);
	 sscanf(sPacket->StartPeriod,"%u",&CONFIG_Meter.StartDuty);
	 sscanf(sPacket->LowBattery,"%f",&CONFIG_Meter.LowBattery);
	 sscanf(sPacket->LowCredit,"%f",&CONFIG_Meter.LowCredit);
	 sscanf(sPacket->LowGasVolume,"%f",&CONFIG_Meter.LowGasVolume);
	 sscanf(sPacket->GAS_REMAINING,"%f",&REAL_DATA_PARAM.TankQuality);
	 strncpy(CONFIG_Meter.CURRENCY,sPacket->Currency,3);
	
	 IsNeedRepaySetup = true;
}

