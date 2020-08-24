/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */
#include "gas.h"
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define Moto_Open_Pin GPIO_PIN_0
#define Moto_Open_GPIO_Port GPIOC
#define Moto_Close_Pin GPIO_PIN_1
#define Moto_Close_GPIO_Port GPIOC
#define Moto3_Open_Pin GPIO_PIN_2
#define Moto3_Open_GPIO_Port GPIOC
#define Moto3_Close_Pin GPIO_PIN_3
#define Moto3_Close_GPIO_Port GPIOC
#define LCD_key_Pin GPIO_PIN_0
#define LCD_key_GPIO_Port GPIOA
#define LCD_key_EXTI_IRQn EXTI0_1_IRQn
#define OPENING_Pin GPIO_PIN_1
#define OPENING_GPIO_Port GPIOA
#define OPENING_EXTI_IRQn EXTI0_1_IRQn
#define USART_TX_SIM80x_Pin GPIO_PIN_2
#define USART_TX_SIM80x_GPIO_Port GPIOA
#define USART_RX_SIM80x_Pin GPIO_PIN_3
#define USART_RX_SIM80x_GPIO_Port GPIOA
#define CS_FLASH_Pin GPIO_PIN_4
#define CS_FLASH_GPIO_Port GPIOA
#define WSensor_Pin GPIO_PIN_5
#define WSensor_GPIO_Port GPIOA
#define LCD_LED_EN_Pin GPIO_PIN_0
#define LCD_LED_EN_GPIO_Port GPIOB
#define EN_BOOST_Pin GPIO_PIN_1
#define EN_BOOST_GPIO_Port GPIOB
#define KEY2_Pin GPIO_PIN_2
#define KEY2_GPIO_Port GPIOB
#define KEY2_EXTI_IRQn EXTI2_3_IRQn
#define AD_EN_Pin GPIO_PIN_10
#define AD_EN_GPIO_Port GPIOB
#define RC522_Reset_Pin GPIO_PIN_11
#define RC522_Reset_GPIO_Port GPIOB
#define SPI_CS2_Pin GPIO_PIN_12
#define SPI_CS2_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_6
#define LCD_CS_GPIO_Port GPIOC
#define LCD_RD_Control_Pin GPIO_PIN_7
#define LCD_RD_Control_GPIO_Port GPIOC
#define LCD_WR_Control_Pin GPIO_PIN_8
#define LCD_WR_Control_GPIO_Port GPIOC
#define GPRS_PWR_Pin GPIO_PIN_8
#define GPRS_PWR_GPIO_Port GPIOA
#define LL_VCC_EN_Pin GPIO_PIN_11
#define LL_VCC_EN_GPIO_Port GPIOA
#define BEEP_Pin GPIO_PIN_15
#define BEEP_GPIO_Port GPIOA
#define EN_GPRS_VCC_Pin GPIO_PIN_10
#define EN_GPRS_VCC_GPIO_Port GPIOC
#define MOTOR_EN_Pin GPIO_PIN_11
#define MOTOR_EN_GPIO_Port GPIOC
#define KEY3_Pin GPIO_PIN_12
#define KEY3_GPIO_Port GPIOC
#define KEY3_EXTI_IRQn EXTI4_15_IRQn
#define RUN_LED_Pin GPIO_PIN_2
#define RUN_LED_GPIO_Port GPIOD
#define LCDVCC_EN_Pin GPIO_PIN_3
#define LCDVCC_EN_GPIO_Port GPIOB
#define CLOSE_Lock_Pin GPIO_PIN_4
#define CLOSE_Lock_GPIO_Port GPIOB
#define OPEN_Lock_Pin GPIO_PIN_5
#define OPEN_Lock_GPIO_Port GPIOB
#define RC522_EN_Pin GPIO_PIN_7
#define RC522_EN_GPIO_Port GPIOB
#define SPI_CS3_Pin GPIO_PIN_8
#define SPI_CS3_GPIO_Port GPIOB

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */
#define LL_VCC(X)		X>0? HAL_GPIO_WritePin(LL_VCC_EN_GPIO_Port,LL_VCC_EN_Pin,GPIO_PIN_SET)\
														:HAL_GPIO_WritePin(LL_VCC_EN_GPIO_Port,LL_VCC_EN_Pin,GPIO_PIN_RESET)

#define ENBOOST_PWR(X)		X>0? HAL_GPIO_WritePin(EN_BOOST_GPIO_Port,EN_BOOST_Pin,GPIO_PIN_SET)\
														:HAL_GPIO_WritePin(EN_BOOST_GPIO_Port,EN_BOOST_Pin,GPIO_PIN_RESET)

#define LCD_POWER(X)	X>0 ?	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET)\
														:HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET)//GPIOA,GPIO_PIN_12  LCD 电源 背光 蜂鸣器 芯片
#define LCD_LIGHT(X)	X>0 ?	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET)\
														:HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET)	//black light low enable 背光 低使能
														
#define GPRS_PWR(X)		X>0? HAL_GPIO_WritePin(EN_GPRS_VCC_GPIO_Port,EN_GPRS_VCC_Pin,GPIO_PIN_SET)\
														:HAL_GPIO_WritePin(EN_GPRS_VCC_GPIO_Port,EN_GPRS_VCC_Pin,GPIO_PIN_RESET)

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
