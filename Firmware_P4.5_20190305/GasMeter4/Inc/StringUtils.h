#ifndef __STRINGUTILS_H
#define __STRINGUTILS_H	

#include "stm32f0xx_hal.h" 

typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

//时间结构体
typedef struct
{
		uint16_t nHour;
		uint16_t nMinute;
		uint16_t nSecond;

		//公历日月年周
		uint16_t  nMonth;
		uint16_t  nDay;
		uint16_t  nWeek;
		uint16_t  nYear;

} RTC_TIME_DEF;

TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);

TestStatus stringCmp(char* pBuffer1, char* pBuffer2, uint8_t BufferLength);

double getYvalue(double x);
double getY10value(double x);
double getY005value(double x);

TestStatus stringIsNull(char* pBuffer1, uint8_t BufferLength);

RTC_TIME_DEF GetSoftWareBuildTargetTime(void);

char * GetSoftWareTime();

uint32_t crc16(char *string,uint16_t len);
#endif

