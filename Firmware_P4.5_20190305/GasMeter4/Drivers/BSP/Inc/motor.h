/**
  ******************************************************************************
  // Header:
  // File Name: motor.h
  // Author:     xudong
  // Date:
  ******************************************************************************
**/
  

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f0xx_hal.h"

typedef enum 
{
	motor_null =0,
	motor_open =1,
	motor_close =2,
	motor_stop =3
}MotorStatus_t;


#define MOTOR_PWR(X)		X>0? HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port,MOTOR_EN_Pin,GPIO_PIN_SET)\
														:HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port,MOTOR_EN_Pin,GPIO_PIN_RESET)

#define LOCK_HANDLE(X)\
{\
	if(X == motor_open)\
	{\
		HAL_GPIO_WritePin(CLOSE_Lock_GPIO_Port,CLOSE_Lock_Pin,GPIO_PIN_RESET);\
		HAL_GPIO_WritePin(OPEN_Lock_GPIO_Port,OPEN_Lock_Pin,GPIO_PIN_SET);\
	}\
	else 	if(X == motor_close)\
	{\
		HAL_GPIO_WritePin(CLOSE_Lock_GPIO_Port,CLOSE_Lock_Pin,GPIO_PIN_SET);\
		HAL_GPIO_WritePin(OPEN_Lock_GPIO_Port,OPEN_Lock_Pin,GPIO_PIN_RESET);\
	}\
	else 	if(X == motor_stop)\
	{\
		HAL_GPIO_WritePin(CLOSE_Lock_GPIO_Port,CLOSE_Lock_Pin,GPIO_PIN_SET);\
		HAL_GPIO_WritePin(OPEN_Lock_GPIO_Port,OPEN_Lock_Pin,GPIO_PIN_SET);\
	}\
}\

#define MOTOR2_HANDLE(X)\
{\
	if(X == motor_open)\
	{\
		HAL_GPIO_WritePin(Moto_Open_GPIO_Port,Moto_Open_Pin,GPIO_PIN_SET);\
		HAL_GPIO_WritePin(Moto_Close_GPIO_Port,Moto_Close_Pin,GPIO_PIN_RESET);\
	}\
	else 	if(X == motor_close)\
	{\
		HAL_GPIO_WritePin(Moto_Open_GPIO_Port,Moto_Open_Pin,GPIO_PIN_RESET);\
		HAL_GPIO_WritePin(Moto_Close_GPIO_Port,Moto_Close_Pin,GPIO_PIN_SET);\
	}\
	else 	if(X == motor_stop)\
	{\
		HAL_GPIO_WritePin(Moto_Open_GPIO_Port,Moto_Open_Pin,GPIO_PIN_SET);\
		HAL_GPIO_WritePin(Moto_Close_GPIO_Port,Moto_Close_Pin,GPIO_PIN_SET);\
	}\
}\

#define Assemble_HANDLE(X)\
{\
	if(X == motor_open)\
	{\
		HAL_GPIO_WritePin(Moto3_Open_GPIO_Port,Moto3_Open_Pin,GPIO_PIN_SET);\
		HAL_GPIO_WritePin(Moto3_Close_GPIO_Port,Moto3_Close_Pin,GPIO_PIN_RESET);\
	}\
	else 	if(X == motor_close)\
	{\
		HAL_GPIO_WritePin(Moto3_Open_GPIO_Port,Moto3_Open_Pin,GPIO_PIN_RESET);\
		HAL_GPIO_WritePin(Moto3_Close_GPIO_Port,Moto3_Close_Pin,GPIO_PIN_SET);\
	}\
	else 	if(X == motor_stop)\
	{\
		HAL_GPIO_WritePin(Moto3_Open_GPIO_Port,Moto3_Open_Pin,GPIO_PIN_SET);\
		HAL_GPIO_WritePin(Moto3_Close_GPIO_Port,Moto3_Close_Pin,GPIO_PIN_SET);\
	}\
}\

extern MotorStatus_t glockSet;	

extern MotorStatus_t glockStatus;	
extern MotorStatus_t gmotorStatus;	 
extern MotorStatus_t gassembleStatus;	

void lock_test(void);	
void motor_test(void);
void assemble_test(void);		

//void motor_test(volatile MotorStatus_t motorStatus,uint16_t runtimeout);
//void motor_control(uint8_t NUM,MotorStatus_t motorStatus);

#endif /*_MOTOR_H */



/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/



