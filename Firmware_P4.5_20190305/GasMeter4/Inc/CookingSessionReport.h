/**
  ******************************************************************************
  * File Name          : CookingSessionReport.h
  * Description        :
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CookingSessionReport_H
#define __CookingSessionReport_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "gas.h"
	 
typedef struct
{
	char meterNumer[20]; 					   //仪表号
	char CUSTOMER_ID[8];             //包租婆编号
	char identifier[5];  					   //流水号
	char CARD_ID[19];							   //卡片ID
	char SESSION_END_TYPE[3];			   //结束的方式
	char START_CUMULATIVE_VOLUME[13];//开始累积体积 
	char END_CUMULATIVE_VOLUME[13];	 //结束累积体积
	char SESSION_START_TIME[15];     //开始使用时间
	char SESSION_END_TIME[15];       //结束使用时间
	char CREDIT_SESSION_START[13];	 //开始的金钱
	char CREDIT_SESSION_END[13];     //结束的金钱
	char UnitPrice[8];               //当前单价 Unit price.unit(T/KG).
	char GasDensity[6];              //当前液化气密度  Density.unit(KG/M3). 2.072  (g/L) 
	char datetime[13];    				   //日期            
	char verification[4]; 				   //校验值
	char GAS_REMAINING[7];           //剩余重量
}CookingSessionReport_t; //结束烹饪的结构体

extern CookingSessionReport_t CookingSessionReport;

void refreshCookingSessionReport(CookingSessionReport_t *cPacket);
void encodeCookingSessionReport(char *sendMeagess,CookingSessionReport_t *cPacket);
bool SendCookingSessionPacket();

#ifdef __cplusplus
}
#endif
#endif /*__ CookingSessionReport_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
