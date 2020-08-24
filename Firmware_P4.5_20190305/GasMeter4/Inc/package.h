// Header:
// File Name:
// Author:
// Date:

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PACKAGE_H
#define __PACKAGE_H

/* 包含头文件 ----------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

//typedef struct
//{
//    char rxbuffer[200];
//    char flag;
//} rx_t; //接收数据的结构体

//typedef struct
//{
//    //char header[2];
//    char meterNumer[15];
//    char keyWord[4];
//    char identifier[4];
//    char RemainingCreditIn[10];//
//    char BatteryLevel[1];//
//    char ARP[3];//
//    char CumulativeVolumetricUsage[9];//
//    char UnitPrice[7];//
//    char GasDensityIn[5];//
//    char CylinderNominalCapacity[6];//
//    char UploadPeriod[1];//
//    char Mechanicalstatus[1];//
//    char datetime[12];
//    char verification[3];
//    //char footer[2];
//} commReportPackage_t; //上报数据结构体
////TOLTAL

//typedef struct
//{
////	char header[2];
//    char meterNumer[15];
//    char keyWord[4];
//    char identifier[4];
//    char rechargeAmountIn[13];//
//    char unitPrice[7];//
//    char LPGDensity [5];//
//    char CylinderNominalCapacity[6];//
//    char UploadPeriod[1];//
//    char WarningTagLowBattery[1];//
//    char WarningTagLowPrepaidBalanceAlarm[5];//
//    char LowGasVolumeInTankAlarmLevel[2];//
//    char Currency[1];//
//    char OpenLid[1];//
//    char datetime[12];//
//    char verification[3];//
////	char footer[2];
//} setRequstPackage_t; //参数设置结构体
//TOLTAL 74


//void refreshReportPackage(commReportPackage_t *reportPackage);
//void encodeReportPackage		(char *sendMeagess,const commReportPackage_t *reportPackage);
//unsigned int CheckSum( char *string);
//unsigned int  CheckJPh(uint32_t addData);
void analyticalPackage(char *buffer,uint32_t sum);
bool DecodeTask(char onechar);

//extern setRequstPackage_t gsetRequstPackage;
//extern commReportPackage_t greportPackage;

#endif

