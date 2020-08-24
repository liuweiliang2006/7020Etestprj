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
	char keyWord[5];     					//�ؼ���CONT
	char meterNumer[20]; 					//�Ǳ��
	char identifier[5];  					//��ˮ��
	char Openlid[2];							//����
	char Valve[2];								//���ط��� 0��1��2����������
	char NEEDLE[2];								//�����뷧 
	char tankLock[2];						  //���ظ�ƿ��
	char datetime[13];    				//����            
	char verification[4]; 				//У��ֵ
}ControlPacket_t; //��ֵ�Ľṹ��

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
