/**
  ******************************************************************************
  * File Name          : RechargePacket.h
  * Description        :
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RechargePacket_H
#define __RechargePacket_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"
	 
typedef struct
{
	char meterNumer[20]; 					//仪表号
	char CUSTOMER_ID[8];             //包租婆编号
	char identifier[5];  					   //流水号
	char CARD_ID[19];							   //卡片ID
	char rechargeAmountIn[13];		//充值金额
	char unitPrice[9];						//单价
	char LPGDensity [6];					//密度
	char ADDORSUB [4];						//加还是减
	char datetime[13];    				//日期            
	char verification[4]; 				//校验值
}RechargePacket_t; //充值的结构体

extern RechargePacket_t RechargePacket;
volatile extern bool IsNeedRepayReCharge;
//extern bool IsRepayReChargeSuccess;//是否回复报警信息成功

void refreshRechargePacket(RechargePacket_t *rPacket);
void encodeRechargePacket(char *sendMeagess,RechargePacket_t *rPacket);
void RechargeToMeter(RechargePacket_t *setRequstPackage);
uint8_t SubToMeter(float);
bool SendRechargePacket(void);

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
