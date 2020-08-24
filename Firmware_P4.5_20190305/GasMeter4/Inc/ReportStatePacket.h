/**
  ******************************************************************************
  * File Name          : ReportStatePacket.h
  * Description        :
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ReportStatePacket_H
#define __ReportStatePacket_H
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
    char BatteryLevel[5]; 
	  char Lid_Sensor_status[2]; 
		char Lid_Lock_status[2];   
	  char NEEDLE_Sensor_status[2]; 
	  char Electric_Valve_status[2];
	  char tank_Sensor_status[2];   
	  char tank_Lock_status[2];     
	  char GSM_signal_intensity[3]; 
	  char GAS_TEMPERATURE[3];      
    char datetime[13];             
    char verification[4]; 
} reportStatePacket_t; 

extern reportStatePacket_t reportStatePacket;

void refreshStateReportPacket(reportStatePacket_t *reportStatePacket);
void encodeReportStatePacket(char *sendMeagess,reportStatePacket_t *reportStatePacket);
bool SendReportStatePacket(void);
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
