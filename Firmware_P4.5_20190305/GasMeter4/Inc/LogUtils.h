/**
  ******************************************************************************
  * File Name          : LogUtils.h
  * Description        : log system
  ******************************************************************************
**/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LogUtils_H
#define __LogUtils_H

#ifdef __cplusplus
	extern "C" {
#endif

#include "stm32f0xx_hal.h"

//日志的类型枚举		
typedef enum
{
	Connect 			= 1,	  //类型为连接 成功或者失败,到哪一步,连接类型为主动连接还是定时连接,心跳还是定时上报
	Send    			= 2,    //数据类型为发送
	Receive			  = 3,    //数据类型为接收
	Cooking 			= 4,    //数据为做饭
	RestartNormal = 5,    //正常重启,在操作的时候写入
	RestartUnNorm = 6,    //不正常重启,在重新启动的时候写入
	ClickButton   = 7,    //单击按钮
	OpenNeedle    = 8,    //打开针法
	CloseNeedle   = 9,    //关闭针阀
	OpenAssemble  = 10,   //打开链接锁
	CloseAssemble = 11,   //关闭连接锁
	OpenValve     = 12,   //打开阀门
	CloseValve    = 13    //关闭阀门
}LogTypeDef_t;		
		
//log的索引 
typedef struct
{
 LogTypeDef_t    LogTypeDef; 			  //日志的类型
 char            datatime[18];   		//日志的时间
 uint32_t        LogAddr;					  //日志对用内容区的开始位置,结尾用\r\n表示
 uint32_t        LogLength;					//日志对用内容区的长度
 char            Description[21];		//日志描述
}Log_index_t;

void LogWrite(LogTypeDef_t LogTypeDef,char * dBuffer,char * pBuffer);
void LogRead(uint32_t startIndex,uint32_t endIndex);

#ifdef __cplusplus
}
#endif

#endif /*__ LogUtils_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
