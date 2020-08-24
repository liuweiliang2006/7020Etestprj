/**
  ******************************************************************************
  * File Name          : SetupPacket.h
  * Description        :
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SetupPacket_H
#define __SetupPacket_H
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
	char UpdatePeriod[6];         //上传周期
	char StartPeriod[6];          //周期开始时间
	char LowBattery[5];						//低电压报警
	char LowCredit[6];		        //低金钱报警
	char LowGasVolume[6];					//低气量报警
	char GAS_REMAINING[7];        //钢瓶的现在实际重量
	char Currency[4];             //货币单位
	char datetime[13];    				//日期            
	char verification[4]; 				//校验值
}SetupPacket_t; //设置的结构体

extern SetupPacket_t SetupPacket;
extern bool IsNeedRepaySetup;

void refreshSetupPacket(SetupPacket_t *sPacket);
void encodeSetupPacket(char *sendMeagess,SetupPacket_t *sPacket);
void SetToMeter(SetupPacket_t *sPacket);
void SendSetupPacket(void);

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
