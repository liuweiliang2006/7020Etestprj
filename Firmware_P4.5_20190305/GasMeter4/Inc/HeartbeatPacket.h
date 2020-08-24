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
    char meterNumer[20]; //�Ǳ��
    char keyWord[5];     //�ؼ���
    char identifier[5];  //��ˮ��
    char datetime[13];   //����          
    char verification[4];//У��ֵ
} heartbeatPackage_t; //�����ṹ��

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
