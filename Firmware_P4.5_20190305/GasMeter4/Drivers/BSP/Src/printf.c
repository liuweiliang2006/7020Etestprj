/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include "cmsis_os.h"
#include "string.h"
#include "printf.h"

extern UART_HandleTypeDef huart3;
extern uint8_t	Usart3RxTemp;

uint8_t ch;
//uint8_t ch_r;
/*fputc*/
int fputc(int c, FILE * f)
{
#if 1
	//taskENTER_CRITICAL();
	ch=c;
  HAL_UART_Transmit(&huart3,&ch,1,1000);	
	//HAL_UART_Transmit(&huart3,(uint8_t*)str,strlen(str),100);
	//taskEXIT_CRITICAL();
	return c;
#endif
}

//uint8_t ch_r;
/*myPrintf*/
void myPrintf(char *str)
{
	//HAL_UART_Receive_IT(&huart3,&Usart3RxTemp,1);
	while(huart3.hdmatx->State != HAL_DMA_STATE_READY)
	{
	  osDelay(10);
	}
	
  HAL_UART_Transmit_DMA(&huart3,(uint8_t*)str,strlen(str));
	
  while(huart3.hdmatx->State != HAL_DMA_STATE_READY)
  {
	   osDelay(10);
	}
}

