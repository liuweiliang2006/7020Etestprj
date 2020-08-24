#include "CustomerCredit.h"
#include "package.h"
#include "string.h"
#include "rtc.h"
#include "Sim80x.h"
#include "LogUtils.h"
#include "StringUtils.h"

CustomerCredit_t CustomerCredit;
extern CONFIG_Meter_t CONFIG_Meter;
extern REAL_DATA_Credit_t REAL_DATA_Credit;
extern Current_Credit_t Current_Credit;
extern uint16_t identCnt;
extern uint32_t sumCnt;
//赋值
void refreshCustomerCredit(CustomerCredit_t *htPackage)
{
	  long date,time;//UTC
	  GetRTC(&time,&date);
		char datatime[12];
	  sprintf(datatime,"%08ld%04ld",date,time / 100);
//	stmerncpy(waringPackage->meterNumer,Sim80x.IMEI,15);
    strncpy(htPackage->meterNumer,CONFIG_Meter.MeterNo,10);
	  strncpy(htPackage->CUSTOMER_ID,Current_Credit.CUSTOMER_ID,7);
    sprintf(htPackage->identifier,"%04d",identCnt++);
	  strncpy(htPackage->CARD_ID,Current_Credit.CARD_ID,18);
    strncpy(htPackage->datetime,datatime,12);
    //strncpy(htPackage->verification,"234",3);
}
//编码
void encodeCustomerCredit(char *sendMeagess,CustomerCredit_t *htPackage)
{
	  strncat(sendMeagess,"##",2);
	  strncat(sendMeagess,"CCCR",4);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,htPackage->meterNumer,10);
    strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,htPackage->CUSTOMER_ID,7);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,htPackage->identifier,4);
    strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,htPackage->CARD_ID,18);
    strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,htPackage->CREDIT_REMAINING,12);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,htPackage->datetime,12);
    strncat(sendMeagess,"/",1);
	  sprintf(htPackage->verification,"%04X",crc16(sendMeagess + 2,strlen(sendMeagess) - 2));
    strncat(sendMeagess,htPackage->verification,4);
    strncat(sendMeagess,"**",2);
}

//####################################################################################################
void SendCustomerCreditPacket(void)
{
	char sendBuffer[200]={0};
	refreshCustomerCredit(&CustomerCredit);
	encodeCustomerCredit(sendBuffer,&CustomerCredit);

	if(GPRS_TcpSendMessage(sendBuffer))
	{
			//printf("GPRS_TcpSendMessage ---------> OK\r\n");
			//printf("SendCustomerCreditPacket: %s \r\n",sendBuffer);
		  LogWrite(Send,"CCCR",sendBuffer);
	}
	else
	{
			//printf("GPRS_TcpSendMessage ---------> ERROR");
			//Sim80x.GPRS.Connection[0] = GPRSConnection_ConnectFail;
		  LogWrite(Send,"CCCRError",sendBuffer);
	}
	//memset(sendBuffer,0,sizeof(sendBuffer));
}
