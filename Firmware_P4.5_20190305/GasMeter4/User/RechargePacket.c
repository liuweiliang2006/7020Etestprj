#include "RechargePacket.h"
#include "package.h"
#include "string.h"
#include "rtc.h"
#include "Sim80x.h"
#include "LogUtils.h"
#include "StringUtils.h"

RechargePacket_t RechargePacket;
volatile bool IsNeedRepayReCharge = false;
//bool IsRepayReChargeSuccess = false;//是否回复报警信息成功
extern bool IsSaveREAL_DATA_Credit;

extern REAL_DATA_Credit_t REAL_DATA_Credit; //仪表充值减值信息

extern CONFIG_GPRS_t CONFIG_GPRS;           //联网配置信息
extern CONFIG_Meter_t CONFIG_Meter;				 //仪表的一些信息
extern REAL_DATA_PARAM_t REAL_DATA_PARAM;   //信号强度等

extern Current_Credit_t Current_Credit;
extern Current_Credit_t Tmp_Credit;

extern uint16_t identCnt;
extern uint32_t sumCnt;
float rechargeCredit = 0;//充值的金钱

extern uint8_t commandType;

//给充值需要回复的包赋值
void refreshRechargePacket(RechargePacket_t *rPacket)
{
		sumCnt = 0;
	  long date,time;//UTC
	  GetRTC(&time,&date);
		char datatime[12];
	  sprintf(datatime,"%08ld%04ld",date,time / 100);
//	stmerncpy(waringPackage->meterNumer,Sim80x.IMEI,15);
    strncpy(rPacket->meterNumer,CONFIG_Meter.MeterNo,10);
	  sumCnt += identCnt;
    //sprintf(rPacket->identifier,"%04d",identCnt++);
    strncpy(rPacket->datetime,datatime,12);
    //strncpy(rPacket->verification,"234",3);
}
//编码充值回复包
void encodeRechargePacket(char *sendMeagess,RechargePacket_t *rPacket)
{
	  strncat(sendMeagess,"##",2);
	  strncat(sendMeagess,"ADMO",4);
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
bool SendRechargePacket(void)
{
	char sendBuffer[200]={0};
	refreshRechargePacket(&RechargePacket);
	encodeRechargePacket(sendBuffer,&RechargePacket);
	if(GPRS_TcpSendMessage(sendBuffer))
	{
			//printf("GPRS_TcpSendMessage ---------> OK\r\n");
			//printf("SendRechargePacket: %s \r\n",sendBuffer);
		  LogWrite(Send,"ReplyADMO",sendBuffer);
		  return true;
	}
	else
	{
			//printf("GPRS_TcpSendMessage ---------> ERROR");
			//Sim80x.GPRS.Connection[0] = GPRSConnection_ConnectFail;
		  LogWrite(Send,"ReplyADMOError",sendBuffer);
		  return false;
	}
	//memset(sendBuffer,0,sizeof(sendBuffer));
}


//充值到仪表
void RechargeToMeter(RechargePacket_t *tPacket)
{
    float unitPrice = 0;//充值单价
    float LPGDensity = 0;//充值密度
	  
//	  uint32_t timeDelay = 0;
//		IsRepayReChargeSuccess = false;
		
//		timeDelay = 0;
//		while((IsRepayReChargeSuccess == false && GSM_ON_FLAG == 1) || timeDelay > 50)
//		{
//			osDelay(100);
//			timeDelay++;
//		}
		
//		if(IsRepayReChargeSuccess == true)
		{
			sscanf(tPacket->unitPrice,"%f",&unitPrice);//充值单价
			if(unitPrice<0.1)
			{
				unitPrice = 4000;
			}
			REAL_DATA_Credit.UnitPrice = unitPrice;
			
			sscanf(tPacket->LPGDensity,"%f",&LPGDensity);//去充值的密度
			if(LPGDensity<0.1)
			{
				LPGDensity = 2.525;
			}
			REAL_DATA_Credit.LPGDensity = LPGDensity;
			
			rechargeCredit = 0;
			
			sscanf(tPacket->rechargeAmountIn,"%f",&rechargeCredit);
			
			if(strcmp(tPacket->ADDORSUB, "SUB")==0)
			{
				rechargeCredit = rechargeCredit * -1;
			}
		}
}

//每个脉冲调用一次,减少余额
//参数为每个脉冲减少的体积 减少的克数
uint8_t SubToMeter(float subSum)
{
	  //printf("LPGDensity:%f\r\n",Tmp_CreditLPGDensity);
		float subKg = subSum / 1000.0;// * REAL_DATA_Credit.LPGDensity;//减去的千克数
		REAL_DATA_Credit.UsedKg = REAL_DATA_Credit.UsedKg + subKg;
	  //printf("subKg:%f\r\n",subKg);
		REAL_DATA_PARAM.TankQuality = REAL_DATA_PARAM.TankQuality - subKg;//钢瓶真实的剩余质量
	  //printf("TankQuality:%f\r\n",REAL_DATA_PARAM.TankQuality);
		REAL_DATA_PARAM.Tank_ARP = REAL_DATA_PARAM.TankQuality / CONFIG_Meter.CYLINDER_CAPACITY * 100;
	
		REAL_DATA_Credit.CumulationGasL = REAL_DATA_Credit.CumulationGasL + subSum;
		REAL_DATA_Credit.CumulationCredit += subKg * REAL_DATA_Credit.UnitPrice;
	  //printf("CurrCredit:%f\r\n",Tmp_CreditCurrCredit);
	  Current_Credit.CurrCredit -= subKg * REAL_DATA_Credit.UnitPrice;
	  if(Current_Credit.CurrCredit < 0)
		{
		   Current_Credit.CurrCredit = 0;
		}
		
		//不论什么情况,只要调用一次就保存一次
		IsSaveREAL_DATA_Credit = true;
		return 0;
}

