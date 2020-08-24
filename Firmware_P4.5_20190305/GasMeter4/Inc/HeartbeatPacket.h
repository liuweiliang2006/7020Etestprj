/**
  ******************************************************************************
  * File Name          : HeartbeatPacket.h
  * Description        :
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HeartbeatPacket_H
#define __HeartbeatPacket_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
	 
typedef struct
{
    char meterNumer[20]; //仪表号
    char keyWord[5];     //关键字
    char identifier[5];  //流水号
    char datetime[13];   //日期          
    char verification[4];//校验值
} heartbeatPackage_t; //心跳结构体

extern heartbeatPackage_t heartbeatPackage;

void refreshHeartbeatPacket(heartbeatPackage_t *heartbeatPackage);
void encodeHeartbeatPacket(char *sendMeagess,heartbeatPackage_t *heartbeatPackage);
void SendHeartbeatPacket(void);
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
