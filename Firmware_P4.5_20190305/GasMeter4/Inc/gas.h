/**
  ******************************************************************************
  * File Name          : bsp.h
  * Description        :
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __gas_H
#define __gas_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include <stdbool.h>
	 
#define FIRMWARE_Version "P4.7.0_2020"
#define PCB_Version "HV5.0.191111"
	 
//小内存	 一共 2048 字节 如果超过这个范围将要从头开始写入
#define	CONFIG_GPRS_Address         			2

#define	CONFIG_Meter_Address        			200	
#define	REAL_DATA_PARAM_Address     			400	 
#define	REAL_DATA_Credit_Address    			600
#define	REAL_Current_Credit_Address    		800	
#define	REAL_Current_Log_Address    		  1000
#define	REAL_Current_Voltage_Address    	1200		
#define BOOTFLAG_ADDR	                    1300
	 
	 
//#define	CONFIG_Meter_Address        			CONFIG_GPRS_Address + sizeof(CONFIG_GPRS_t)	
//#define	REAL_DATA_PARAM_Address     			CONFIG_Meter_Address + sizeof(CONFIG_Meter_t)	 
//#define	REAL_DATA_Credit_Address    			REAL_DATA_PARAM_Address + sizeof(REAL_DATA_PARAM_t)
//#define	REAL_Current_Credit_Address    		REAL_DATA_Credit_Address + sizeof(REAL_DATA_Credit_t)
	 
#define CONFIG_GPRS_READ()        				MB85RS16A_READ(CONFIG_GPRS_Address,(uint8_t*)(&CONFIG_GPRS),sizeof(CONFIG_GPRS_t))
#define CONFIG_GPRS_Write()       				MB85RS16A_WRITE(CONFIG_GPRS_Address,(uint8_t*)(&CONFIG_GPRS),sizeof(CONFIG_GPRS_t))
	
#define CONFIG_Meter_READ()       				MB85RS16A_READ(CONFIG_Meter_Address,(uint8_t*)(&CONFIG_Meter),sizeof(CONFIG_Meter_t))
#define CONFIG_Meter_Write()      				MB85RS16A_WRITE(CONFIG_Meter_Address,(uint8_t*)(&CONFIG_Meter),sizeof(CONFIG_Meter_t))
	
#define REAL_DATA_PARAM_READ()    				MB85RS16A_READ(REAL_DATA_PARAM_Address,(uint8_t*)(&REAL_DATA_PARAM),sizeof(REAL_DATA_PARAM_t)) 
#define REAL_DATA_PARAM_Write()   				MB85RS16A_WRITE(REAL_DATA_PARAM_Address,(uint8_t*)(&REAL_DATA_PARAM),sizeof(REAL_DATA_PARAM_t))  
	
#define REAL_DATA_Credit_READ()						MB85RS16A_READ(REAL_DATA_Credit_Address,(uint8_t*)(&REAL_DATA_Credit),sizeof(REAL_DATA_Credit_t))
#define REAL_DATA_Credit_Write()  				MB85RS16A_WRITE(REAL_DATA_Credit_Address,(uint8_t*)(&REAL_DATA_Credit),sizeof(REAL_DATA_Credit_t))
	
#define REAL_Current_Credit_READ()   			MB85RS16A_READ(REAL_Current_Credit_Address,(uint8_t*)(&Current_Credit),sizeof(Current_Credit_t))
#define REAL_Current_Credit_Write()  			MB85RS16A_WRITE(REAL_Current_Credit_Address,(uint8_t*)(&Current_Credit),sizeof(Current_Credit_t))
	
#define REAL_Current_Log_READ()   			  MB85RS16A_READ(REAL_Current_Log_Address,(uint8_t*)(&Current_Log),sizeof(Current_Log_t))
#define REAL_Current_Log_Write()  			  MB85RS16A_WRITE(REAL_Current_Log_Address,(uint8_t*)(&Current_Log),sizeof(Current_Log_t))
	
#define REAL_Current_Voltage_READ()   		MB85RS16A_READ(REAL_Current_Voltage_Address,(uint8_t*)(&Current_Voltage),sizeof(Current_Voltage_t))
#define REAL_Current_Voltage_Write()  		MB85RS16A_WRITE(REAL_Current_Voltage_Address,(uint8_t*)(&Current_Voltage),sizeof(Current_Voltage_t))
	
#define BOOTFLAG_ADDR_READ()   		        MB85RS16A_READ(BOOTFLAG_ADDR,(uint8_t*)(&update),sizeof(update_t))
#define BOOTFLAG_ADDR_Write()  		        MB85RS16A_WRITE(BOOTFLAG_ADDR,(uint8_t*)(&update),sizeof(update_t))

//big flash Voltage calibration data
#define CALIBRATION_Voltage_READ()        SPI_FLASH_BufferRead((uint8_t*)(&REAL_DATA_CALIBRATION),0,sizeof(REAL_DATA_CALIBRATION))
#define CALIBRATION_Voltage_Write()       W25Q64_Write((uint8_t*)(&REAL_DATA_CALIBRATION),0,sizeof(REAL_DATA_CALIBRATION))
//big flash flow calibration data
#define CALIBRATION_Flow_READ()           SPI_FLASH_BufferRead((uint8_t*)(&REAL_Flow_CALIBRATION),4096,sizeof(REAL_Flow_CALIBRATION))	
#define CALIBRATION_Flow_Write()          W25Q64_Write((uint8_t*)(&REAL_Flow_CALIBRATION),4096,sizeof(REAL_Flow_CALIBRATION))
//客户卡片的存储信息	
#define Current_Credit_READ(X)     SPI_FLASH_BufferRead((uint8_t*)(&(Tmp_Credit)), FLASH_RechargeBase + (X) * sizeof(Current_Credit_t), sizeof(Current_Credit_t))
#define Current_Credit_Write(X)    W25Q64_Write((uint8_t*)(&(X)), FLASH_RechargeBase + (X.CurrIndex) * sizeof(Current_Credit_t), sizeof(Current_Credit_t))
//客户做饭的信息
#define Cooking_Session_READ(X)    SPI_FLASH_BufferRead((uint8_t*)(&(CookingSessionReport)), FLASH_CookingSessionBase + (X) * sizeof(CookingSessionReport_t), sizeof(CookingSessionReport_t))
#define Cooking_Session_Write()    W25Q64_Write((uint8_t*)(&(CookingSessionReport)), FLASH_CookingSessionBase + (REAL_DATA_Credit.CookingSessionEnd) * sizeof(CookingSessionReport_t), sizeof(CookingSessionReport_t))

//远程升级信息
typedef struct
{
 uint8_t BOOTFLAG;       //升级标志位
 uint32_t FILESIZE;      //文件大小
 char MD5CODE[16];      //MD5校验码
 char URL_ADDR[128];      //URL地址
}update_t;

//一些联网信息,需要通过工具配置	 
typedef struct
{
 char  Server_IP[20]; 					//GPRS IP地址
 char  Socket_Port[6];   				//GPRS 端口
 char  USE_GPRS_APN[2];					//是否启用APN   1 启用,其他的不启用
 char  APN[17];									//GPRS APN
 char  APN_UserName[17];				//GPRS APN名字
 char  APN_Password[17]; 				//APN密码
 char  CenterSimNumber[20];			//短信中心站号码
 char  ServerSimNumber[20];			//服务器短信接收号码
 char  Communications_Type[2];	//通讯选择 0-通讯关闭 1-GPRS 2-短信 3-都有
 char  Pin[17];	                //肯尼亚有	
}CONFIG_GPRS_t;	 	 

//现在使用的卡数据,
typedef struct
{
	uint32_t CurrIndex;				 //对应当前使用的卡存放的位置
	char CUSTOMER_ID[32];      //包租婆编号
	uint8_t CUSTOMERNoLength;  //客户号长度    7
	char CARD_ID[32];          //卡片编号
	uint8_t CARDNoLength;      //卡片号长度    18
	
	float CurrCredit;					    //当前的可使用的金额
}Current_Credit_t;

//现在使用的日志数据,
typedef struct
{
	uint32_t indexSendNumber;				     //对应当前已经下载过的日志
	uint32_t indexCurrent;                //当前需要存储的编号,存完加一
	uint32_t addrBigData;                   //在内存 0x500000-0x799999之间的数据地址
}Current_Log_t;

//现在使用的电压数据的索引
typedef struct
{
	uint32_t indexVCurrent;                //当前需要存储的编号,存完加一
}Current_Voltage_t;

//电压数据的结构体
typedef struct
{
 char            datatime[18];   		//电压的时间
 float           voltageNumber;		  //电压的值
}Data_Voltage_t;

//仪表的一些实时数据,剩余金钱,气量,单价,密度等,定时或者实时存入FRAM
typedef struct
{
	float CumulationCredit;		//累计使用的金钱,工具可清零
	float CumulationGasL;			//累计使用的气量,工具可清零
	
  float UsedKg;             //使用的千克数,由每个脉冲所代表的体积乘以密度然后累计到这个值
		
	float	Magnification;	    //倍率,由威盛的表与煤气的比热容决定
	float LPGDensity;				 	//当前液化气密度  Density.unit(KG/M3). 2.484  (g/L)
	float UnitPrice;			    //当前单价 Unit price.unit(T/KG).
	
	uint32_t CardTotal;				//保存最大的卡序号,初始值是0,表明从未刷卡,随着刷卡数量的增多,一直增长
	
	uint32_t CookingSessionSendNumber;		//保存发送完成的序号
	uint32_t CookingSessionEnd;		      //保存最大的做饭信息一直增长
}REAL_DATA_Credit_t;


//仪表的一些信息,需要通过工具配置	
typedef struct
{
	char  MeterNo[20];             //仪表编号
	
	uint32_t  MeterNoLength;       //仪表编号长度
	char  FIRMWARE_V[5];          //固件的版本号 
	char  PCB_V[15];              //PCB的版本号 	
	char	CURRENCY[4];						 //货币	TZS or GHC
	char	CYLINDER_No[12];         //钢瓶的编号  
	float CYLINDER_CAPACITY;   	 	 //钢瓶的容积 15 KG
	char  BATTERY_MODEL[6];				 //电池的模式 TX123

//	char  UIType[2];							 //屏幕的类型  1为LCD1602  2为断码屏
//	char  Lid_Type[2];				     //锁的类型 1 电磁锁 2 新型电磁锁
	
	uint32_t UpDuty;               //上传周期 由软件或者服务器设置
  uint32_t StartDuty;            //周期开始时间 由软件或者服务器设置
	
	float LowBattery;              //低于这个电压报警
	float LowCredit;               //低于这个金钱报警
  float LowGasVolume;            //气罐内的体积低于这个报警 单位Kg
	
	bool IsHaveRFID;

	char Longitude [15];	//GPS 经度
	char Latitude [15];	  //GPS 纬度
	
	char CompileDate[12];	  //编译日期
	char CompileTime[10];	  //编译时间
	
  bool NotHaveDog;  //是否没有看门狗 没有看门狗对应极低功耗 ,置1 true
	
	uint32_t ResetSum;  //重启次数

	bool IsNormalReset;   //是正常重启
	uint32_t NormalResetSum;  //正常重启次数
	
	char ResetTime[20];	  //重启时间
	
	char  USE_SENSOR[2];	//传感器类型 1为0-100,2为0-20
	
}CONFIG_Meter_t;

////仪表的执行部件状态
//typedef struct
//{
//  char  LidSensorStatus[2];      //表盖状态 0:Lid closed, 1: Lid open
//	char  LidElectricLock[2];      //表盖锁的状态  0: UnLock, 1: Lock
//	char  NEEDLESensorStatus[2];   //针阀的状态	0:needle up(close), 1: needle down(open)
//	char	ElectricValveStatus[2];	 //阀门的状态  0:Valve closed, 1: Valve open
//	char  TankSensorStatus[2];     //连接传感器的状态 0: Disassemble, 1: Assemble  现在用不着
//	char  TankLockStatus[2];			 //连接锁的状态 0: UnLock, 1: Lock 
//	
//}Electric_Meter_t;

//仪表总成的一些信息
typedef struct
{
	float BatVoltage;      //剩余电量
	float Tank_ARP;		     //气量剩余百分比 000-100 
	float TankQuality;     //本次气罐容量 KG ,由软件或者服务器发送本次的实际重量  Tank quality.unit(kg).

	//Electric_Meter_t Electric_Meter; //仪表的机械状态
	
	char  LidSensorStatus[2];      //表盖状态 0:Lid closed, 1: Lid open
	char  LidElectricLock[2];      //表盖锁的状态  0: UnLock, 1: Lock
	char  NEEDLESensorStatus[2];   //针阀的状态	0:needle up(close), 1: needle down(open)
	char	ElectricValveStatus[2];	 //阀门的状态  0:Valve closed, 1: Valve open
	char  TankSensorStatus[2];     //连接传感器的状态 0: Disassemble, 1: Assemble  现在用不着
	char  TankLockStatus[2];			 //连接锁的状态 0: UnLock, 1: Lock 
	
	uint8_t GprsCsq;           //GPRS信号强度
	
	char CUSTOMER_ID[32];
}REAL_DATA_PARAM_t;	 

//校准数据
typedef struct
{
	float	slope; //校准出来的斜率 K值
  float	zero;	 //零点值   B值
}REAL_DATA_CALIBRATION_t;	 

//传感器校准数据
typedef struct
{
	float	ABCDEF[10]; 			//多项式系数
  int8_t	ParamNumber;	  //多项式的个数
}REAL_Flow_CALIBRATION_t;	 

typedef struct
{
	unsigned char tm_seconds;             // seconds 0-59
	unsigned char tm_minute;              // 0-59
	unsigned char tm_hour;                // 0-59
	unsigned char tm_day;                // 1-31
	unsigned char tm_month;                 // 1-12
	unsigned int  tm_year;                // 
//	unsigned char tm_wday;                // 0-6 0==sunday
//  unsigned char status;                 //
}tim_t;


#ifdef __cplusplus
}
#endif
#endif /*__ usart_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
