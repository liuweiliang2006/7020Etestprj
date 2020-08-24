#ifndef __LCD_H 
#define __LCD_H 

#include "ht1621b.h"
#include "stm32f0xx_hal.h"

/*公共口*/
#define COM1 0X00
#define COM2 0X01
#define COM3 0X02
#define COM4 0X03

/*显存地址*/
#define SEG0 0
#define SEG1 1
#define SEG2 2
#define SEG3 3
#define SEG4 4
#define SEG5 5
#define SEG6 6
#define SEG7 7
#define SEG8 8
#define SEG9 9
#define SEG10 10
#define SEG11 11
#define SEG12 12
#define SEG13 13
#define SEG14 14
#define SEG15 15
#define SEG16 16
#define SEG17 17
#define SEG18 18
#define SEG19 19
#define SEG20 20
#define SEG21 21
#define SEG22 22
#define SEG23 23
#define SEG24 24
#define SEG25 25
#define SEG26 26
#define SEG27 27
#define SEG28 28
#define SEG29 29
#define SEG30 30
#define SEG31 31

/*硬件lcd内存映射*/
#define PIN5 SEG0*4//每个地址有4位
#define PIN6 SEG1*4
#define PIN7 SEG2*4
#define PIN8 SEG3*4
#define PIN9 SEG4*4
#define PIN10 SEG5*4
#define PIN11 SEG6*4
#define PIN12 SEG7*4
#define PIN13 SEG8*4
#define PIN14 SEG9*4
#define PIN15 SEG10*4
#define PIN16 SEG11*4
#define PIN17 SEG12*4
#define PIN18 SEG13*4
#define PIN19 SEG14*4
#define PIN20 SEG15*4
#define PIN21 SEG16*4
#define PIN22 SEG17*4
#define PIN23 SEG18*4
#define PIN24 SEG19*4

/*位段对应内存映射*/
#define _S7  (unsigned char)(PIN5+COM1)
#define _7B  (unsigned char)(PIN5+COM2)
#define _7G  (unsigned char)(PIN5+COM3)
#define _7C  (unsigned char)(PIN5+COM4)

#define _7A  (unsigned char)(PIN6+COM1)
#define _7F  (unsigned char)(PIN6+COM2)
#define _7E  (unsigned char)(PIN6+COM3)
#define _7D  (unsigned char)(PIN6+COM4)
	
#define _D6  (unsigned char)(PIN7+COM1)
#define _6B  (unsigned char)(PIN7+COM2)
#define _6G  (unsigned char)(PIN7+COM3)
#define _6C  (unsigned char)(PIN7+COM4)
	
#define _6A  (unsigned char)(PIN8+COM1)
#define _6F  (unsigned char)(PIN8+COM2)
#define _6E  (unsigned char)(PIN8+COM3)
#define _6D  (unsigned char)(PIN8+COM4)
	
#define _D5  (unsigned char)(PIN9+COM1)
#define _5B  (unsigned char)(PIN9+COM2)
#define _5G  (unsigned char)(PIN9+COM3)
#define _5C  (unsigned char)(PIN9+COM4)

#define _5A  (unsigned char)(PIN10+COM1)
#define _5F  (unsigned char)(PIN10+COM2)
#define _5E  (unsigned char)(PIN10+COM3)
#define _5D  (unsigned char)(PIN10+COM4)

#define _D3  (unsigned char)(PIN11+COM1)
#define _4B  (unsigned char)(PIN11+COM2)
#define _4G  (unsigned char)(PIN11+COM3)
#define _4C  (unsigned char)(PIN11+COM4)

#define _4A  (unsigned char)(PIN12+COM1)
#define _4F  (unsigned char)(PIN12+COM2)
#define _4E  (unsigned char)(PIN12+COM3)
#define _4D  (unsigned char)(PIN12+COM4)

#define _D2  (unsigned char)(PIN13+COM1)
#define _3B  (unsigned char)(PIN13+COM2)
#define _3G  (unsigned char)(PIN13+COM3)
#define _3C  (unsigned char)(PIN13+COM4)

#define _3A  (unsigned char)(PIN14+COM1)
#define _3F  (unsigned char)(PIN14+COM2)
#define _3E  (unsigned char)(PIN14+COM3)
#define _3D  (unsigned char)(PIN14+COM4)

#define _D1  (unsigned char)(PIN15+COM1)
#define _2B  (unsigned char)(PIN15+COM2)
#define _2G  (unsigned char)(PIN15+COM3)
#define _2C  (unsigned char)(PIN15+COM4)

#define _2A  (unsigned char)(PIN16+COM1)
#define _2F  (unsigned char)(PIN16+COM2)
#define _2E  (unsigned char)(PIN16+COM3)
#define _2D  (unsigned char)(PIN16+COM4)

#define _D4  (unsigned char)(PIN17+COM1)
#define _1B  (unsigned char)(PIN17+COM2)
#define _1G  (unsigned char)(PIN17+COM3)
#define _1C  (unsigned char)(PIN17+COM4)

#define _1A  (unsigned char)(PIN18+COM1)
#define _1F  (unsigned char)(PIN18+COM2)
#define _1E  (unsigned char)(PIN18+COM3)
#define _1D  (unsigned char)(PIN18+COM4)

#define _S6  (unsigned char)(PIN19+COM1)
#define _L1  (unsigned char)(PIN19+COM2)
#define _L2  (unsigned char)(PIN19+COM3)
#define _L3  (unsigned char)(PIN19+COM4)

#define _S5  (unsigned char)(PIN20+COM1)
#define _S1  (unsigned char)(PIN20+COM2)
#define _L5  (unsigned char)(PIN20+COM3)
#define _L4  (unsigned char)(PIN20+COM4)

#define _S4  (unsigned char)(PIN21+COM1)
#define _S3  (unsigned char)(PIN21+COM2)
#define _S2  (unsigned char)(PIN21+COM3)
//#define S (unsigned char)(PIN21+COM4)

#define _S8  (unsigned char)(PIN22+COM1)
#define _S9  (unsigned char)(PIN22+COM2)
#define _S10 (unsigned char)(PIN22+COM3)
#define _S11 (unsigned char)(PIN22+COM4)

//#define C1  (unsigned char)(PIN23+COM1)
#define _B1  (unsigned char)(PIN23+COM2)
#define _S13 (unsigned char)(PIN23+COM3)
#define _S12 (unsigned char)(PIN23+COM4)

#define _B4  (unsigned char)(PIN24+COM1)
#define _B2  (unsigned char)(PIN24+COM2)
#define _B3  (unsigned char)(PIN24+COM3)
#define _B5  (unsigned char)(PIN24+COM4)

#define XH1  	   _L1//1格信号
#define XH2    	 _L2//2格信号
#define XH3      _L3//3格信号
#define XH4      _L4//4格信号
#define XH5      _L5//5格信号

#define Bluetooth     _S1//蓝牙
#define RFID        	_S2//RFID
#define WRAN  	      _S3//警告

#define ValveW  		  _S4//阀门外壳
#define ValveL        _S5//左
#define ValveM        _S6//中
#define ValveR        _S7//右

#define syncBig       _S10//大圈
#define syncMid       _S9 //中圈
#define syncSma       _S8 //小圈

#define cylinder      _S11//钢瓶
#define KgSign        _S12//kg
#define MONEY         _S13//钱

#define BATS0         _B5//电量 标志
#define BATS1         _B1//电量 满电标志
#define BATS2         _B2//电量 中电标志
#define BATS3         _B3//电量 低电标志
#define BATS4         _B4//电量 低电标志

#define POINT1        _D1//小数点 1
#define POINT2        _D2//小数点 2
#define POINT3        _D3//小数点 3
#define POINT4        _D4//小数点 4
#define POINT5        _D5//小数点 5
#define POINT6        _D6//小数点 6

//typedef enum
//{
//	DisplayON=0,
//	DisplayOFF
//}LCDState_Type;

//typedef enum
//{
//	SW_None = 0,//全部不显示
//	SW_XH1 = 1, //信号标志1
//	SW_XH2 = 1<<1,//信号2格
//	SW_XH3 = 1<<2,//信号3格
//	SW_XH4 = 1<<3,//信号4格
//	SW_XH5 = 1<<4,//信号5格
//	
//	SW_Bluetooth = 1<<5,//蓝牙
//	SW_RFID = 1<<6,//RFID
//	SW_WRAN =1 <<7,//WRAN
//	
//	SW_Valve = 1<<8,//Valve
//	SW_ValveL = 1<<9,//ValveLeft
//	SW_ValveM  = 1<<10,//ValveM 
//	SW_ValveR = 1<<11,//ValveRight
//	
//	SW_syncBig = 1<<12,
//	SW_syncMid = 1<<13,
//	SW_syncSma = 1<<14,
//	
//	SW_cylinder = 1<<15,//cylinder
//	SW_KgSign = 1<<16,//KgSign
//	SW_MONEY = 1<<17,//MONEY
//	
//	SW_BATS0 = 1<<18, //0
//	SW_BATS1 = 1<<19, //1
//	SW_BATS2 = 1<<20, //2
//	SW_BATS3 = 1<<21, //3
//	SW_BATS4 = 1<<22, //4
//	
//	SW_POINT1 = 1<<23,
//	SW_POINT2 = 1<<24,
//	SW_POINT3 = 1<<25,
//	SW_POINT4 = 1<<26,
//	SW_POINT5 = 1<<27,
//	SW_POINT6 = 1<<28
////	SW_All = SW_Signal | SW_Signal_1 | SW_Signal_2 | SW_Signal_3 | SW_Tariff | SW_Months
////									 | SW_Cumulation | SW_Balance| SW_Consumption | SW_Top_Up | SW_Frozen
////									 | SW_UP | SW_WARN | SW_Point_1 
////									 | SW_Point_2 | SW_Money | SW_Slash | SW_M3 | SW_Valve_Opens | SW_Valve_Closes
////									 | SW_Bat_S | SW_Bat_S1 | SW_Bat_S2 | SW_Bat_S3 
////									 |SW_BackLight|SW_UsedCredit
////									 |SW_RemainedCredit|SW_GasVolume|SW_VLeftGas|SW_VUGasM3
//	 
//								
//}LCDDisSw;
//typedef enum
//{
//	WaterRecv=0,
//	WaterDischarge,
//	Sleep
//}LCDWateBoxState_Type;

typedef enum
{
	OFF=0,
	ON=!OFF
}LCDSymbolState_Type;

typedef enum
{
	Update_None = 0,//没有可以更新的数据
	Update_Signal_1 = 1, //信号1标志
	Update_Signal_2 = 1<<1,//信号2格
	Update_Signal_3 = 1<<2,//信号3格
	Update_Signal_4 = 1<<3,//信号4格
	Update_Signal_5 = 1<<4,//信号5格

	Update_Bluetooth = 1<<5,//月份
	Update_RFID = 1<<6,//RFID
	Update_WRAN =1 <<7,//WRAN
	Update_ValveW = 1<<8,//Valve
	Update_ValveL = 1<<9,//ValveL
	Update_ValveM = 1<<10,//ValveM
	Update_ValveR = 1<<11,//ValveR
	Update_syncBig = 1<<12,
	Update_syncMid = 1<<13,
	Update_syncSma = 1<<14,
	Update_cylinder = 1<<15,//cylinder
	Update_KgSign = 1<<16,//KgSign
	Update_MONEY = 1<<17,//MONEY
	
	Update_BATS0 = 1<<18, //BATS0
	Update_BATS1 = 1<<19, //BATS1
	Update_BATS2 = 1<<20, //BATS2
	Update_BATS3 = 1<<21, //BATS3
	Update_BATS4 = 1<<22, //BATS4
	
	Update_POINT1 = 1<<23, 
	Update_POINT2 = 1<<24, 
	Update_POINT3 = 1<<25,
	Update_POINT4 = 1<<26,
	Update_POINT5 = 1<<27,
	Update_POINT6 = 1<<28,
	Update_num = 1<<29,//升级数字
	
	Update_All = Update_Signal_1 | Update_Signal_1 | Update_Signal_2 | Update_Signal_3 | Update_Signal_4 | Update_Signal_5
									 | Update_Bluetooth | Update_RFID| Update_WRAN | Update_ValveW | Update_ValveL | Update_ValveM | Update_ValveR
									 | Update_syncBig | Update_syncMid |  Update_syncSma 
									 | Update_cylinder | Update_KgSign | Update_MONEY 
									 | Update_BATS0 | Update_BATS1 | Update_BATS2| Update_BATS3 | Update_BATS4
									 | Update_POINT1 | Update_POINT2 | Update_POINT3 | Update_POINT4 | Update_POINT5 | Update_POINT6
									 | Update_num
}LCDUpdate_Type;

//LCD显示面板定义结构
typedef struct
{
	uint8_t DisType;
	int DisNumber;//显示的数字
	char DisChar[7];
//	unsigned int CumulationCredit;//累计使用金额
//	unsigned int RemainedCredit;//剩余金额
//	unsigned int GasVolume;//液化气体积（M3) 气罐中可用体积
//	unsigned int CumulationGasM3;//计量累计使用体积
//	unsigned int RemainedGasM3;//用户剩余可用气体体积 
	unsigned char Sys_Time_HH;//系统时钟小时
	unsigned char Sys_Time_MM;//系统时钟分钟
	unsigned char Sys_Time_SS;//系统时钟秒
	
	//LCDDisSw PartDisSw;//某个或者组合模块是否显示的开关 （闪烁）
	
	LCDSymbolState_Type XH1_Sign;//信号1格状态
	LCDSymbolState_Type XH2_Sign;//信号2格状态
	LCDSymbolState_Type XH3_Sign;//信号3格状态
	LCDSymbolState_Type XH4_Sign;//信号4格状态
	LCDSymbolState_Type XH5_Sign;//信号5格状态
	
	LCDSymbolState_Type Bluetooth_Sign;
	LCDSymbolState_Type RFID_Sign;
	LCDSymbolState_Type WRAN_Sign;
	LCDSymbolState_Type ValveW_Sign;
	LCDSymbolState_Type ValveL_Sign;
	LCDSymbolState_Type ValveM_Sign;
	LCDSymbolState_Type ValveR_Sign;
	LCDSymbolState_Type syncBig_Sign;
	LCDSymbolState_Type syncMid_Sign;
	LCDSymbolState_Type syncSma_Sign;
	LCDSymbolState_Type cylinder_Sign;
	LCDSymbolState_Type KgSign_Sign;
	LCDSymbolState_Type MONEY_Sign;//钱标志状态
	
	LCDSymbolState_Type POINT1_Sign;//小数点1状态
	LCDSymbolState_Type POINT2_Sign;//小数点2状态
	LCDSymbolState_Type POINT3_Sign;//小数点3状态
	LCDSymbolState_Type POINT4_Sign;//小数点4状态
	LCDSymbolState_Type POINT5_Sign;//小数点5状态
	LCDSymbolState_Type POINT6_Sign;//小数点6状态

	LCDSymbolState_Type BATS0_Sign;//电池B5状态
	LCDSymbolState_Type BATS1_Sign;//电池B1格状态
	LCDSymbolState_Type BATS2_Sign;//电池B2格状态
	LCDSymbolState_Type BATS3_Sign;//电池B3格状态
	LCDSymbolState_Type BATS4_Sign;//电池B4格状态	
	
	//LCDUpdate_Type Update_Type;//更新类型
	
}LCDPanel_TypeDef;


void LCD_Init(void);
void RefreshSignal(void);
void RefreshBatVoltage(void);
void RefreshValve(void);
void RefreshWarn(void);
void RefreshCylinder(void);
void RefreshMoney(void);
void LCD_Refresh(LCDPanel_TypeDef* LCDPanel);
void LCD_Close(void);
//void LCD_Show(uint8_t * sStep);

#endif
