/**
  ******************************************************************************
  * File Name          : CustomerCredit.h
  * Description        :
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CustomerCredit_H
#define __CustomerCredit_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
	 
typedef struct
{
	char meterNumer[20]; 					
	char CUSTOMER_ID[8];            
	char identifier[5];  					   
	char CARD_ID[19];							   
	char CREDIT_REMAINING[13];								
	char datetime[13];    				            
	char verification[4]; 				
}CustomerCredit_t; 

extern CustomerCredit_t CustomerCredit;

void refreshCustomerCredit(CustomerCredit_t *rPacket);
void encodeCustomerCredit(char *sendMeagess,CustomerCredit_t *rPacket);
void SendCustomerCreditPacket(void);

#ifdef __cplusplus
}
#endif
#endif /*__ CustomerCredit_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
