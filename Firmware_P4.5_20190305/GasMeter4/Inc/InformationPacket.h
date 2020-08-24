/**
  ******************************************************************************
  * File Name          : InformationPacket.h
  * Description        :
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __InformationPacket_H
#define __InformationPacket_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
	 
typedef struct
{
	  char keyWord[5];  
    char meterNumer[20]; 
	  char CUSTOMER_ID[8];
    char identifier[5];   
	  char FIRMWARE_V[5]; 
	  char PCB_V[15];
	  char CURRENCY[4];						 //货币TZS or GHC
	  char GAS_REMAINING[7];
	  char CYLINDER_No[12];  
    char UpdatePeriod[6];         //上传周期	
	  char StartPeriod[6];          //周期开始时间
	  char BATTERY_MODEL[6];
    char datetime[13];                
    char verification[4]; 
} InformationPacket_t; 

extern InformationPacket_t InformationPacket;

void refreshInformationPacket(InformationPacket_t *informationPacket);
void encodeInformationPacket(char *sendMeagess,InformationPacket_t *informationPacket);
void SendInformationPacket(void);
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
