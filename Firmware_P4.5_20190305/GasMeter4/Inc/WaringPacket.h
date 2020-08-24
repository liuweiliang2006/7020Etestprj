/**
  ******************************************************************************
  * File Name          : WaringPacket.h
  * Description        :
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WaringPacket_H
#define __WaringPacket_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
	 
typedef struct
{
	  char keyWord[5];     //关键字
    char meterNumer[20]; //仪表号
		char CUSTOMER_ID[8]; //包租婆编号
    char identifier[5];  //流水号
    char Low_battery[2]; //低电
    char low_amount_of_prepaid_credit[2]; //低钱
	  char Low_gas_amount[2];  //低煤气
	  char Illegal_lid_opening[2];  //非法开盖
	  char Electric_Lock_status[2]; //表盖锁状态
	  char NEEDLE_Sensor_status[2]; //针阀状态
	  char Electric_Valve_status[2];//阀门状态
	  char tank_Sensor_status[2];   //连接还是分离
	  char tank_Lock_status[2];     //锁的状态
	  char GSM_signal_intensity[3]; //GSM信号的值
	  char GAS_TEMPERATURE[3];      //温度的值
    char datetime[13];    //日期            
    char verification[4]; //校验值
} waringPacket_t; //报警的结构体

extern waringPacket_t waringPacket;

void refreshWaringPacket(waringPacket_t *waringPacket);
void encodeWaringPacket(char *sendMeagess,waringPacket_t *waringPacket);
void SendWarnPacket(void);

#ifdef __cplusplus
}
#endif
#endif /*__ HeartbeatPacket_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
