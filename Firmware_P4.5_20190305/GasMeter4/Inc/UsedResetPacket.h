/**
  ******************************************************************************
  * File Name          : SetupPacket.h
  * Description        :
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UsedResetPacket_H
#define __UsedResetPacket_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"
	 
typedef struct
{
	char keyWord[5];     					//关键STUP
	char meterNumer[20]; 					//仪表号
	char identifier[5];  					//流水号
	char RealWeight[7];           //实际重量
	char datetime[13];    				//日期            
	char verification[4]; 				//校验值
}UsedResetPacket_t; //设置的结构体

extern UsedResetPacket_t UsedResetPacket;
extern bool IsNeedRepayCYCH;

void refreshUsedResetPacket(UsedResetPacket_t *sPacket);
void encodeUsedResetPacket(char *sendMeagess,UsedResetPacket_t *sPacket);
void ResetToMeter(UsedResetPacket_t *sPacket);
void SendResetPacket(void);


#ifdef __cplusplus
}
#endif
#endif /*__ SetupPacket_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
