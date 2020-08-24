#include "UsedResetPacket.h"
#include "package.h"
#include "string.h"
#include "rtc.h"
#include "Sim80x.h"
#include "LogUtils.h"
#include "StringUtils.h"

UsedResetPacket_t UsedResetPacket;
bool IsNeedRepayCYCH = false;

extern CONFIG_Meter_t CONFIG_Meter;
extern REAL_DATA_PARAM_t REAL_DATA_PARAM;
extern REAL_DATA_Credit_t REAL_DATA_Credit; 

extern uint16_t identCnt;
//extern uint32_t sumCnt;
//给设置需要回复的包赋值
void refreshUsedResetPacket(UsedResetPacket_t *sPacket)
{
	  //sumCnt = 0;
	  long date,time;//UTC
	  GetRTC(&time,&date);
		char datatime[12];
	  sprintf(datatime,"%08ld%04ld",date,time / 100);
//	stmerncpy(waringPackage->meterNumer,Sim80x.IMEI,15);
    strncpy(sPacket->meterNumer,CONFIG_Meter.MeterNo,10);
	  //sumCnt += identCnt;
    sprintf(sPacket->identifier,"%04d",identCnt++);
    strncpy(sPacket->datetime,datatime,12);
    //strncpy(sPacket->verification,"234",3);
}
//编码设置回复包
void encodeUsedResetPacket(char *sendMeagess,UsedResetPacket_t *sPacket)
{
	  strncat(sendMeagess,"##",2);
	  strncat(sendMeagess,"CYCH",4);
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

//设置到仪表
void ResetToMeter(UsedResetPacket_t *sPacket)
{
	 float realWeight = 0;//实际的的重量
	 sscanf(sPacket->RealWeight,"%f",&realWeight);
	 if(realWeight<0.1)
	 {
		 REAL_DATA_PARAM.TankQuality = CONFIG_Meter.CYLINDER_CAPACITY;
	 }
	 else
	 {
		 sscanf(sPacket->RealWeight,"%f",&REAL_DATA_PARAM.TankQuality);
	 }
	 REAL_DATA_Credit.CumulationCredit =0;
	 REAL_DATA_Credit.CumulationGasL =0;
}

//####################################################################################################
void SendResetPacket(void)
{
	char sendBuffer[200]={0};
	refreshUsedResetPacket(&UsedResetPacket);
	encodeUsedResetPacket(sendBuffer,&UsedResetPacket);

	if(GPRS_TcpSendMessage(sendBuffer))
	{
			//printf("GPRS_TcpSendMessage ---------> OK\r\n");
			//printf("SendResetPacket: %s \r\n",sendBuffer);
		  LogWrite(Send,"ReplyCYCH",sendBuffer);
	}
	else
	{
			//printf("GPRS_TcpSendMessage ---------> ERROR");
			//Sim80x.GPRS.Connection[0] = GPRSConnection_ConnectFail;
		  LogWrite(Send,"ReplyCYCHError",sendBuffer);
	}
	//memset(sendBuffer,0,sizeof(sendBuffer));
}

