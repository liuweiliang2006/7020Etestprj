/**
  ******************************************************************************
  * File Name          : KEY.h
  * Description        : 
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef _KEY_H_
#define _KEY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f0xx_hal.h"

#define PORT_KEY 		GPIOA
#define PORT_PIN 		GPIO_PIN_0

#define key_input     		HAL_GPIO_ReadPin(LCD_key_GPIO_Port,LCD_key_Pin)      // 按键输入口  上升沿触发,正常为高,松开按键触发
#define Opening_input     HAL_GPIO_ReadPin(OPENING_GPIO_Port,OPENING_Pin)      // 开盖检测的光敏 开盖为高,上升沿触发
#define Key2_input        HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin)  			     // 开盖检测的微动  正常为高,从倾斜到正常水平为由低到高
#define NEEDLE_input      HAL_GPIO_ReadPin(KEY3_GPIO_Port,KEY3_Pin)  			     // 针阀的微动

#define N_key    0             //无键
#define S_key    1             //单键
#define D_key    2             //双键
#define L_key    3             //长键

#define key_state_0 0 			// 按键初始状态
#define key_state_1 1       // 按键被按下,状态转换到按键消抖和确认状态
#define key_state_2 2       // 按键任然处于按下,消抖完成,状态转换到按下键时间的计时状态,但返回的还是无键事件
#define key_state_3 3 	    // 转换到等待按键释放状态

typedef struct
{
  uint8_t index_input;        //选择哪一个输入引脚
	unsigned char key_m;   			//外层变量
	unsigned char key_time_1;   //外层变量
	
	unsigned char key_state;    //内层变量
	unsigned char key_time;     //内层变量
}key_Status_t;

unsigned char key_read(key_Status_t * key_Status);	

#ifdef __cplusplus
}
#endif

#endif /* _KEY_H_ */
