/**
  ******************************************************************************
  * File Name          : ReportDataPacket.h
  * Description        :
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ReportDataPacket_H
#define __ReportDataPacket_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"
	 
typedef struct
{
	  char keyWord[5];  
    char meterNumer[20]; 
    char identifier[5];   
	  char CumulativeVolumetricUsage[13]; 
	  char TankQuality[7];
	  char CREDIT[13];
	  char CYLINDER_No[12];   
    char datetime[13];                
    char verification[4]; 
} reportDataPacket_t; 

extern reportDataPacket_t reportDataPacket;

void refreshReportDataPacket(reportDataPacket_t *reportDataPacket);
void encodeReportDataPacket(char *sendMeagess,reportDataPacket_t *reportDataPacket);
bool SendReportDataPacket(void);
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
