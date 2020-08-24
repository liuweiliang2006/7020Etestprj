/**
  ******************************************************************************
  * File Name          : HT1621B.h
  * Description        : HT1621B芯片的驱动接口
  ******************************************************************************
**/
//定义防止递归包含
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HT1621_H
#define __HT1621_H

#ifdef __cplusplus
	extern "C" {
#endif

#include "stm32f0xx_hal.h" //导入操作hal库的接口

#define CS_W(A)  		  A>0 ?	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET)//GPIOB,GPIO_PIN_12 片选
#define RD_W(A)  			A>0 ?	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_RESET)//GPIOB,GPIO_PIN_13 读数据控制用脚
#define WR_W(A)  			A>0 ?	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET)//GPIOB,GPIO_PIN_10 写数据控制引脚
#define DAT_W(A)  		A>0 ?	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_RESET)//GPIOB,GPIO_PIN_15 写数据引脚
#define DAT_R     		HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_9)  //读数据的引脚		
		
typedef enum
{
	FourCOMS = 0x28,	//时钟为片内
	LCDON    = 0x03,	//开启Lcd偏置发生器
	LCDOFF   = 0x02,	//关闭Lcd偏置发生器
	SYSEN    = 0X01,	//开启系统振荡器
	SYSDIS   = 0X00		//关振系统荡器和LCD偏压发生器
}HT1621ArgInitTypeDef;

typedef struct
{
		unsigned char SysState;//
		unsigned char LcdState;//
		unsigned char BiasComNum;//
} HT1621_InitTypeDef;


void SendCmd(unsigned char command);

void Write_1621(unsigned char addr,unsigned char Data);

//void WriteAll_1621(unsigned char addr,unsigned char *p,unsigned char cnt);

void Read_Modify_Write_1621(unsigned char addr,unsigned char Mask, unsigned char Value);

#ifdef __cplusplus
}
#endif

#endif /*__ HT1621B_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
