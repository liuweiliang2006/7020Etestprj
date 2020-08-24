#ifndef __LCD_H 
#define __LCD_H 

#include "ht1621b.h"
#include "stm32f0xx_hal.h"

/*������*/
#define COM1 0X00
#define COM2 0X01
#define COM3 0X02
#define COM4 0X03

/*�Դ��ַ*/
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

/*Ӳ��lcd�ڴ�ӳ��*/
#define PIN5 SEG0*4//ÿ����ַ��4λ
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

/*λ�ζ�Ӧ�ڴ�ӳ��*/
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

#define XH1  	   _L1//1���ź�
#define XH2    	 _L2//2���ź�
#define XH3      _L3//3���ź�
#define XH4      _L4//4���ź�
#define XH5      _L5//5���ź�

#define Bluetooth     _S1//����
#define RFID        	_S2//RFID
#define WRAN  	      _S3//����

#define ValveW  		  _S4//�������
#define ValveL        _S5//��
#define ValveM        _S6//��
#define ValveR        _S7//��

#define syncBig       _S10//��Ȧ
#define syncMid       _S9 //��Ȧ
#define syncSma       _S8 //СȦ

#define cylinder      _S11//��ƿ
#define KgSign        _S12//kg
#define MONEY         _S13//Ǯ

#define BATS0         _B5//���� ��־
#define BATS1         _B1//���� �����־
#define BATS2         _B2//���� �е��־
#define BATS3         _B3//���� �͵��־
#define BATS4         _B4//���� �͵��־

#define POINT1        _D1//С���� 1
#define POINT2        _D2//С���� 2
#define POINT3        _D3//С���� 3
#define POINT4        _D4//С���� 4
#define POINT5        _D5//С���� 5
#define POINT6        _D6//С���� 6

//typedef enum
//{
//	DisplayON=0,
//	DisplayOFF
//}LCDState_Type;

//typedef enum
//{
//	SW_None = 0,//ȫ������ʾ
//	SW_XH1 = 1, //�źű�־1
//	SW_XH2 = 1<<1,//�ź�2��
//	SW_XH3 = 1<<2,//�ź�3��
//	SW_XH4 = 1<<3,//�ź�4��
//	SW_XH5 = 1<<4,//�ź�5��
//	
//	SW_Bluetooth = 1<<5,//����
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
	Update_None = 0,//û�п��Ը��µ�����
	Update_Signal_1 = 1, //�ź�1��־
	Update_Signal_2 = 1<<1,//�ź�2��
	Update_Signal_3 = 1<<2,//�ź�3��
	Update_Signal_4 = 1<<3,//�ź�4��
	Update_Signal_5 = 1<<4,//�ź�5��

	Update_Bluetooth = 1<<5,//�·�
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
	Update_num = 1<<29,//��������
	
	Update_All = Update_Signal_1 | Update_Signal_1 | Update_Signal_2 | Update_Signal_3 | Update_Signal_4 | Update_Signal_5
									 | Update_Bluetooth | Update_RFID| Update_WRAN | Update_ValveW | Update_ValveL | Update_ValveM | Update_ValveR
									 | Update_syncBig | Update_syncMid |  Update_syncSma 
									 | Update_cylinder | Update_KgSign | Update_MONEY 
									 | Update_BATS0 | Update_BATS1 | Update_BATS2| Update_BATS3 | Update_BATS4
									 | Update_POINT1 | Update_POINT2 | Update_POINT3 | Update_POINT4 | Update_POINT5 | Update_POINT6
									 | Update_num
}LCDUpdate_Type;

//LCD��ʾ��嶨��ṹ
typedef struct
{
	uint8_t DisType;
	int DisNumber;//��ʾ������
	char DisChar[7];
//	unsigned int CumulationCredit;//�ۼ�ʹ�ý��
//	unsigned int RemainedCredit;//ʣ����
//	unsigned int GasVolume;//Һ���������M3) �����п������
//	unsigned int CumulationGasM3;//�����ۼ�ʹ�����
//	unsigned int RemainedGasM3;//�û�ʣ������������ 
	unsigned char Sys_Time_HH;//ϵͳʱ��Сʱ
	unsigned char Sys_Time_MM;//ϵͳʱ�ӷ���
	unsigned char Sys_Time_SS;//ϵͳʱ����
	
	//LCDDisSw PartDisSw;//ĳ���������ģ���Ƿ���ʾ�Ŀ��� ����˸��
	
	LCDSymbolState_Type XH1_Sign;//�ź�1��״̬
	LCDSymbolState_Type XH2_Sign;//�ź�2��״̬
	LCDSymbolState_Type XH3_Sign;//�ź�3��״̬
	LCDSymbolState_Type XH4_Sign;//�ź�4��״̬
	LCDSymbolState_Type XH5_Sign;//�ź�5��״̬
	
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
	LCDSymbolState_Type MONEY_Sign;//Ǯ��־״̬
	
	LCDSymbolState_Type POINT1_Sign;//С����1״̬
	LCDSymbolState_Type POINT2_Sign;//С����2״̬
	LCDSymbolState_Type POINT3_Sign;//С����3״̬
	LCDSymbolState_Type POINT4_Sign;//С����4״̬
	LCDSymbolState_Type POINT5_Sign;//С����5״̬
	LCDSymbolState_Type POINT6_Sign;//С����6״̬

	LCDSymbolState_Type BATS0_Sign;//���B5״̬
	LCDSymbolState_Type BATS1_Sign;//���B1��״̬
	LCDSymbolState_Type BATS2_Sign;//���B2��״̬
	LCDSymbolState_Type BATS3_Sign;//���B3��״̬
	LCDSymbolState_Type BATS4_Sign;//���B4��״̬	
	
	//LCDUpdate_Type Update_Type;//��������
	
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
