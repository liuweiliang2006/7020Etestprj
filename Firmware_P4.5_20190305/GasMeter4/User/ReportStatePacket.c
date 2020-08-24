#include "ReportStatePacket.h"
#include "package.h"
#include "string.h"
#include "rtc.h"
#include "key.h"
#include "Sim80x.h"
#include "LogUtils.h"
#include "StringUtils.h"

reportStatePacket_t reportStatePacket;
extern CONFIG_Meter_t CONFIG_Meter;
extern REAL_DATA_PARAM_t REAL_DATA_PARAM;
extern REAL_DATA_Credit_t REAL_DATA_Credit; 
extern uint16_t identCnt;
extern uint32_t sumCnt;

void refreshReportStatePacket(reportStatePacket_t *rPacket)
{
	  sumCnt = 0;
		long date,time;//UTC
	  GetRTC(&time,&date);
		char datatime[12];
	  sprintf(datatime,"%08ld%04ld",date,time / 100);
    strncpy(rPacket->meterNumer,CONFIG_Meter.MeterNo,10);
	  sumCnt += identCnt;
    sprintf(rPacket->identifier,"%04d",identCnt++);
	  sumCnt += REAL_DATA_PARAM.BatVoltage;
    sprintf(rPacket->BatteryLevel,"%.2f",REAL_DATA_PARAM.BatVoltage);
	
	  strncpy(rPacket->Lid_Sensor_status,Opening_input == 1 ? "1" : "0",1);
	
	  strncpy(rPacket->Lid_Lock_status,REAL_DATA_PARAM.LidElectricLock,1);
	
		strncpy(rPacket->NEEDLE_Sensor_status,REAL_DATA_PARAM.NEEDLESensorStatus,1);
	  
	  strncpy(rPacket->Electric_Valve_status,REAL_DATA_PARAM.ElectricValveStatus,1);
	  
	  strncpy(rPacket->tank_Sensor_status,REAL_DATA_PARAM.TankSensorStatus,1);
	
	  strncpy(rPacket->tank_Lock_status,REAL_DATA_PARAM.TankLockStatus,1);
		
	  sprintf(rPacket->GSM_signal_intensity,"%2d",Sim80x.Status.Signal);
		
	  sprintf(rPacket->GAS_TEMPERATURE,"%2d",25);
    sumCnt += atoi(datatime);
    strncpy(rPacket->datetime,datatime,12);
    //strncpy(rPacket->verification,"234",3);
}
//TOTAL

void encodeReportStatePacket(char *sendMeagess,reportStatePacket_t *rPacket)
{
    strncat(sendMeagess,"##",2);
	  strncat(sendMeagess,"HWSR",4);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,rPacket->meterNumer,10);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,rPacket->identifier,4);
    strncat(sendMeagess,"/",1);
		strncat(sendMeagess,rPacket->BatteryLevel,4);
	  strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->Lid_Sensor_status,1);
	  strncat(sendMeagess,"/",1);
    strncat(sendMeagess,rPacket->Lid_Lock_status,1);
	  strncat(sendMeagess,"/",1);
    strncat(sendMeagess,rPacket->NEEDLE_Sensor_status,1);
	  strncat(sendMeagess,"/",1);
    strncat(sendMeagess,rPacket->Electric_Valve_status,1);
		strncat(sendMeagess,"/",1);
    strncat(sendMeagess,rPacket->tank_Sensor_status,1);
    strncat(sendMeagess,"/",1);
		strncat(sendMeagess,rPacket->tank_Lock_status,1);
    strncat(sendMeagess,"/",1);
		strncat(sendMeagess,rPacket->GSM_signal_intensity,2);
    strncat(sendMeagess,"/",1);
		strncat(sendMeagess,rPacket->GAS_TEMPERATURE,2);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,rPacket->datetime,12);
    strncat(sendMeagess,"/",1);
    sprintf(rPacket->verification,"%04X",crc16(sendMeagess + 2,strlen(sendMeagess) - 2));
    strncat(sendMeagess,rPacket->verification,4);
    strncat(sendMeagess,"**",2);
}

//####################################################################################################
bool SendReportStatePacket(void)
{
	char sendBuffer[200]={0};
	refreshReportStatePacket(&reportStatePacket);
	encodeReportStatePacket(sendBuffer,&reportStatePacket);
	//intf("sendBuffer1: %s \r\n",sendBuffer);
	if(GPRS_TcpSendMessage(sendBuffer))
	{
			//printf("GPRS_TcpSendMessage ---------> OK\r\n");
			//printf("SendReportStatePacket: %s \r\n",sendBuffer);
		  LogWrite(Send,"HWSR",sendBuffer);
		return true;
	}
	else
	{
			//printf("GPRS_TcpSendMessage ---------> ERROR");
			Sim80x.GPRS.Connection[0] = GPRSConnection_ConnectFail;
		  LogWrite(Send,"HWSRError",sendBuffer);
		return false;
	}
	//memset(sendBuffer,0,sizeof(sendBuffer));
}

