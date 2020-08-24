#include "ReportPacket.h"
#include "string.h"
#include "rtc.h"
#include "key.h"
#include "Sim80x.h"

reportPacket_t reportPacket;
extern CONFIG_Meter_t CONFIG_Meter;
extern REAL_DATA_PARAM_t REAL_DATA_PARAM;
extern REAL_DATA_Credit_t REAL_DATA_Credit; //仪表充值减值信息
extern uint16_t identCnt;
//给主动上报结构体赋值
void refreshReportPacket(reportPacket_t *rPacket)
{	
		long date,time;//UTC
	  GetRTC(&time,&date);
		char datatime[12];
	  sprintf(datatime,"%08ld%04ld",date,time);
    strncpy(rPacket->meterNumer,CONFIG_Meter.MeterNo,10);
    sprintf(rPacket->identifier,"%04d",identCnt++);

    //sprintf(rPacket->RemainingCredit,"%.2f",REAL_DATA_Credit.RemainedCredit);
    sprintf(rPacket->BatteryLevel,"%.2f",REAL_DATA_PARAM.BatVoltage);
	  sprintf(rPacket->ApproximateRemainingPercentage,"%.1f",REAL_DATA_PARAM.Tank_ARP);
	  sprintf(rPacket->CumulativeVolumetricUsage,"%.2f",REAL_DATA_Credit.CumulationGasL);
	  sprintf(rPacket->UnitPrice,"%.2f",REAL_DATA_Credit.UnitPrice);
	  sprintf(rPacket->GasDensity,"%.3f",REAL_DATA_Credit.LPGDensity);
	  sprintf(rPacket->CylinderNominalCapacity,"%.3f",CONFIG_Meter.CYLINDER_CAPACITY);
	  sprintf(rPacket->UpdatePeriod,"%04d",CONFIG_Meter.UpDuty);
	  strncpy(rPacket->Lid_Sensor_status,Key2_input == GPIO_PIN_SET ? "1" : "0",1);
	  strncpy(rPacket->Lid_Lock_status,REAL_DATA_PARAM.LidElectricLock,1);
		strncpy(rPacket->NEEDLE_Sensor_status,REAL_DATA_PARAM.NEEDLESensorStatus,1);
	  strncpy(rPacket->Electric_Valve_status,REAL_DATA_PARAM.ElectricValveStatus,1);
	  strncpy(rPacket->tank_Sensor_status,REAL_DATA_PARAM.TankSensorStatus,1);
	  strncpy(rPacket->tank_Lock_status,REAL_DATA_PARAM.TankLockStatus,1);
	  sprintf(rPacket->GSM_signal_intensity,"%2d",Sim80x.Status.Signal);
	  sprintf(rPacket->GAS_TEMPERATURE,"%2d",25);

    strncpy(rPacket->datetime,datatime,12);
    strncpy(rPacket->verification,"234",3);
}
//TOTAL

//主动上报
void encodeReportPacket(char *sendMeagess,const reportPacket_t *rPacket)
{
    strncat(sendMeagess,"##",2);
    strncat(sendMeagess,rPacket->meterNumer,10);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,"DATA",4);
    strncat(sendMeagess,"/",1);
    strncat(sendMeagess,rPacket->identifier,4);
    strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->RemainingCredit,12);
	  strncat(sendMeagess,"/",1);
		strncat(sendMeagess,rPacket->BatteryLevel,4);
	  strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->ApproximateRemainingPercentage,3);
	  strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->CumulativeVolumetricUsage,12);
	  strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->UnitPrice,8);
	  strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->GasDensity,5);
	  strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->CylinderNominalCapacity,6);
	  strncat(sendMeagess,"/",1);
	  strncat(sendMeagess,rPacket->UpdatePeriod,5);
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
    strncat(sendMeagess,rPacket->verification,3);
    strncat(sendMeagess,"**",2);
}

//####################################################################################################
//发送定时报
void SendReportPacket(void)
{
	//char sendBuffer[200]={0};
	//refreshReportPacket(&reportPacket);
	//encodeReportPacket(sendBuffer,&reportPacket);
  char sendBuffer[120]={"##TZ00000015/DATA/0000/6253.00/4.00/100/0.00/4000.00/2.484/15.000/0005/1/0/1/0/1/1/16/181211171026/234**"};
	if(GPRS_TcpSendMessage(sendBuffer))
	{
			printf("GPRS_TcpSendMessage ---------> OK\r\n");
			printf("sendBuffer: %s \r\n",sendBuffer);
	}
	else
	{
			//printf("GPRS_TcpSendMessage ---------> ERROR");
			Sim80x.GPRS.Connection[0] = GPRSConnection_ConnectFail;
	}
	//memset(sendBuffer,0,sizeof(sendBuffer));
}

