/**
  ******************************************************************************
  * File Name          : ControlPacket.h
  * Description        :
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ControlPacket_H
#define __ControlPacket_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"
	 
typedef struct
{
	char keyWord[5];     					//关键字CONT
	char meterNumer[20]; 					//仪表号
	char identifier[5];  					//流水号
	char Openlid[2];							//开盖
	char Valve[2];								//开关阀门 0关1开2不操作本项
	char NEEDLE[2];								//开关针阀 
	char tankLock[2];						  //开关钢瓶锁
	char datetime[13];    				//日期            
	char verification[4]; 				//校验值
}ControlPacket_t; //充值的结构体

extern ControlPacket_t controlPacket;
extern bool IsNeedRepayControl;

void refreshControlPacket(ControlPacket_t *cPacket);
void encodeControlPacket(char *sendMeagess,ControlPacket_t *cPacket);
void SendControlPacket(void);

#ifdef __cplusplus
}
#endif
#endif /*__ RechargePacket_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
