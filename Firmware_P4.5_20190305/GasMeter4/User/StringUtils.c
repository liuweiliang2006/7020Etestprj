#include "bsp.h"

//float argsABC[5] = {
//	375.69019364181929,
//	-2238.4887148681482,
//	4863.5364846878228,
//	-4577.7160699287142,
//	1599.7478468166855};
//double argsABC[6] = {
//	-601.52689687745885,
//	5529.3209677073228,
//	-19663.357540827579,
//	33875.154050778627,
//	-28334.985848889908,
//	9257.8544840103259};

extern REAL_Flow_CALIBRATION_t REAL_Flow_CALIBRATION;

double getYvalue(double x)
{
		double y = REAL_Flow_CALIBRATION.ABCDEF[0];
		double xPower = 1;
		for (int i = 1; i < REAL_Flow_CALIBRATION.ParamNumber; i++)
		{
				xPower *= x;
				y += REAL_Flow_CALIBRATION.ABCDEF[i] * xPower;
		}
		return y;
}
//正在使用的炜盛
//9H17V455  1.22176797*x^4-6.30476651*x^3+13.63397451*x^2-6.32829777*x+0.4373903
//double getY10value(double x)
//{
//	  double	ABCDEF[10]; 			
//	  int8_t ParamNumber = 5;
//		ABCDEF[0] = 0.4373903;
//		ABCDEF[1] = -6.32829777;
//		ABCDEF[2] = 13.63397451;
//		ABCDEF[3] = -6.30476651;
//	  ABCDEF[4] = 1.22176797;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//AA0005  2.96608908*x^2+7.52702739*x-4.50637473
double getY10value(double x)
{
	  double	ABCDEF[10]; 			
	  int8_t ParamNumber = 3;
		ABCDEF[0] = -4.50637473;
		ABCDEF[1] = 7.52702739;
		ABCDEF[2] = 2.96608908;
		double y = ABCDEF[0];
		double xPower = 1;
		for (int i = 1; i < ParamNumber; i++)
		{
				xPower *= x;
				y += ABCDEF[i] * xPower;
		}
		return y;
}
//微纳的AA005测试 第一次不装壳体测试
//-4.75097693*x^3+6.57648942*x^2+10.88792317*x-6.50363717
//double getY005value(double x)
//{
//	  double	ABCDEF[10]; 			
//	  int8_t ParamNumber = 4;
//		ABCDEF[0] = -6.50363717;
//		ABCDEF[1] = 10.88792317;
//		ABCDEF[2] = 6.57648942;
//		ABCDEF[3] = -4.75097693;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//微纳的AA005测试 安装上减压器后alcat控制在1.4L/min,重新标定
//-1.57024733*x^2+13.19924953*x-6.22721931
//double getY005value(double x)
//{
//	  double	ABCDEF[10]; 			
//	  int8_t ParamNumber = 3;
//		ABCDEF[0] = -6.22721931;
//		ABCDEF[1] = 13.19924953;
//		ABCDEF[2] = -1.57024733;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//微纳的AA005测试 只使用称在原有基础上增加标定双灶最大火
//7.99652696*x^3-16.78322887*x^2+22.65383937*x-8.14768437
//double getY005value(double x)
//{
//	  double	ABCDEF[10]; 			
//	  int8_t ParamNumber = 4;
//		ABCDEF[0] = -8.14768437;
//		ABCDEF[1] = 22.65383937;
//		ABCDEF[2] = -16.78322887;
//	  ABCDEF[3] = 7.99652696;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//微纳的AA005测试 只使用称在双灶最大火基础上增加标定单灶最大火
//-40.08221697*x^4+132.00294512*x^3-154.07034065*x^2+87.62952908*x-19.30280744
double getY005value(double x)
{
	  double	ABCDEF[10]; 			
	  int8_t ParamNumber = 5;
		ABCDEF[0] = -19.30280744;
		ABCDEF[1] = 87.62952908;
		ABCDEF[2] = -154.07034065;
	  ABCDEF[3] = 132.00294512;
	  ABCDEF[4] = -40.08221697;
		double y = ABCDEF[0];
		double xPower = 1;
		for (int i = 1; i < ParamNumber; i++)
		{
				xPower *= x;
				y += ABCDEF[i] * xPower;
		}
		return y;
}

//微纳传感器  46.37508624*x^3-162.18963643*x^2+198.89989504*x-82.97057826
//double getY10value(double x)
//{
//	  double	ABCDEF[10]; 			
//	  int8_t ParamNumber = 4;
//		ABCDEF[0] = -82.97057826;
//		ABCDEF[1] = 198.89989504;
//		ABCDEF[2] = -162.18963643;
//		ABCDEF[3] = 46.37508624;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//9H17V455  1.22176797*x^4-6.30476651*x^3+13.63397451*x^2-6.32829777*x+0.4373903
//double getY10value(double x)
//{
//	  double	ABCDEF[10]; 			//多项式系数
//	  int8_t ParamNumber = 5;
//		ABCDEF[0] = 0.4373903;
//		ABCDEF[1] = -6.32829777;
//		ABCDEF[2] = 13.63397451;
//		ABCDEF[3] = -6.30476651;
//	  ABCDEF[4] = 1.22176797;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//9H17V359  70.98038978*x^5-332.52203502*x^4+593.59800584*x^3-499.23340521*x^2+201.68638187*x-31.68655232
//double getY10value(double x)
//{
//	  double	ABCDEF[10]; 			//多项式系数
//	  int8_t ParamNumber = 6;
//		ABCDEF[0] = -31.68655232;
//		ABCDEF[1] = 201.68638187;
//		ABCDEF[2] = -499.23340521;
//		ABCDEF[3] = 593.59800584;
//	  ABCDEF[4] = -332.52203502;
//	  ABCDEF[5] = 70.98038978;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//9H17V359  -2.06667146*x^3+9.84113884*x^2-5.90296993*x+0.75641168
//double getY10value(double x)
//{
//	  double	ABCDEF[10]; 			//多项式系数
//	  int8_t ParamNumber = 4;
//		ABCDEF[0] = 0.75641168;
//		ABCDEF[1] = -5.90296993;
//		ABCDEF[2] = 9.84113884;
//		ABCDEF[3] = -2.06667146;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//9H17V359_avg  1.39703859*x^3+0.79078806*x^2+1.92062901*x-1.33278972
//double getY10value(double x)
//{
//	  double	ABCDEF[10]; 			//多项式系数
//	  int8_t ParamNumber = 4;
//		ABCDEF[0] = -1.33278972;
//		ABCDEF[1] = 1.92062901;
//		ABCDEF[2] = 0.79078806;
//		ABCDEF[3] = 1.39703859;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//9H17V359_avg  -2.48982104*x^3+10.29207331*x^2-5.46802533*x+0.47308513

//double getY10value(double x)
//{
//	  double	ABCDEF[10]; 			//多项式系数
//	  int8_t ParamNumber = 4;
//		ABCDEF[0] = 0.47308513;
//		ABCDEF[1] = -5.46802533;
//		ABCDEF[2] = 10.29207331;
//		ABCDEF[3] = -2.48982104;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//通用
//double getY10value(double x)
//{
//	  double y = REAL_Flow_CALIBRATION.ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < REAL_Flow_CALIBRATION.ParamNumber; i++)
//		{
//				xPower *= x;
//				y += REAL_Flow_CALIBRATION.ABCDEF[i] * xPower;
//		}
//		return y;
//}

//9G18V014
//double getY10value(double x)
//{
//	  double	ABCDEF[10]; 			//多项式系数
//	  int8_t ParamNumber = 4;
//		ABCDEF[0] = -0.41714166;
//		ABCDEF[1] = -2.22022378;
//		ABCDEF[2] = 6.46414643;
//		ABCDEF[3] = -1.43701703;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//9G18V017
//double getY10value(double x)
//{
//	  double	ABCDEF[10]; 			//多项式系数
//	  int8_t ParamNumber = 4;
//		ABCDEF[0] = -1.58154218;
//		ABCDEF[1] = 2.11460879;
//		ABCDEF[2] = 1.73405187;
//		ABCDEF[3] = -0.18211776;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//9G18V018
//double getY10value(double x)
//{
//	  double	ABCDEF[10]; 			//多项式系数
//	  int8_t ParamNumber = 4;
//		ABCDEF[0] = -0.14019546;
//		ABCDEF[1] = -2.91608626;
//		ABCDEF[2] = 7.20131887;
//		ABCDEF[3] = -1.73355871;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//9G18V021
//double getY10value(double x)
//{
//	  double	ABCDEF[10]; 			//多项式系数
//	  int8_t ParamNumber = 4;
//		ABCDEF[0] = -0.16591118;
//		ABCDEF[1] = -2.65659552;
//		ABCDEF[2] = 6.7787148;
//		ABCDEF[3] = -1.55565681;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//9G18V022
//double getY10value(double x)
//{
//	  double	ABCDEF[10]; 			//多项式系数
//	  int8_t ParamNumber = 4;
//		ABCDEF[0] = -0.56173811;
//		ABCDEF[1] = -1.41931046;
//		ABCDEF[2] = 5.47481822;
//		ABCDEF[3] = -1.1570683;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//9G18V023
//double getY10value(double x)
//{
//	  double	ABCDEF[10]; 			//多项式系数
//	  int8_t ParamNumber = 4;
//		ABCDEF[0] = -0.34757895;
//		ABCDEF[1] = -2.0284156;
//		ABCDEF[2] = 6.36216404;
//		ABCDEF[3] = -1.64782049;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//9G18V024
//double getY10value(double x)
//{
//	  double	ABCDEF[10]; 			//多项式系数
//	  int8_t ParamNumber = 4;
//		ABCDEF[0] = -2.11067997;
//		ABCDEF[1] = 3.46794153;
//		ABCDEF[2] = 1.91798856;
//		ABCDEF[3] = -0.45854551;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//9H17V504
//double getY10value(double x)
//{
//	  double	ABCDEF[10]; 			//多项式系数
//	  int8_t ParamNumber = 4;
//		ABCDEF[0] = -0.64423516;
//		ABCDEF[1] = -1.49900688;
//		ABCDEF[2] = 6.44980053;
//		ABCDEF[3] = -1.91926205;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}
//9H17V238
//double getY10value(double x)
//{
//	  double	ABCDEF[10]; 			//多项式系数
//	  int8_t ParamNumber = 4;
//		ABCDEF[0] = -0.64423516;
//		ABCDEF[1] = -1.49900688;
//		ABCDEF[2] = 6.44980053;
//		ABCDEF[3] = -1.91926205;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

//9H17V238
//double getY10value(double x)
//{
//	  double	ABCDEF[10]; 			//多项式系数
//	  int8_t ParamNumber = 4;
//		ABCDEF[0] = -0.32355592;
//		ABCDEF[1] = -3.06185844;
//		ABCDEF[2] = 8.84020471;
//		ABCDEF[3] = -2.9922273;
//		double y = ABCDEF[0];
//		double xPower = 1;
//		for (int i = 1; i < ParamNumber; i++)
//		{
//				xPower *= x;
//				y += ABCDEF[i] * xPower;
//		}
//		return y;
//}

uint32_t crc16(char *string,uint16_t len)
{
   uint32_t uiCRC16;
   int i,j;
   uiCRC16= 0xffff;  //init
   for(i = 0; i<len; i++)
   {
      //uiCRC16=UpdateCRC16(string[i],uiCRC16);   //update for every Byte
      uiCRC16 = (unsigned int)(string[i] ^ uiCRC16);

			for(j =0; j<8;j++)
			{
				if((uiCRC16 & 0x0001) == 0)
				{
					 uiCRC16=(unsigned int)(uiCRC16>>1);
				}
				else
				{
					 uiCRC16=(unsigned int)(uiCRC16>>1);
					 uiCRC16=(unsigned int)(uiCRC16 ^ 0xa001);
				}
			}
   }
   return uiCRC16 ;
}

TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
  while(BufferLength--)
  {
    if(*pBuffer1 != *pBuffer2)
    {
      return FAILED;
    }

    pBuffer1++;
    pBuffer2++;
  }
  return PASSED;
}

TestStatus stringCmp(char* pBuffer1, char* pBuffer2, uint8_t BufferLength)
{
  while(BufferLength--)
  {
    if(*pBuffer1 != *pBuffer2)
    {
      return FAILED;
    }

    pBuffer1++;
    pBuffer2++;
  }
  return PASSED;
}

TestStatus stringIsNull(char* pBuffer1, uint8_t BufferLength)
{
  while(BufferLength--)
  {
		//printf("pBuffer1 %c  %d \r\n",BufferLength,*pBuffer1);
    if(*pBuffer1 == '\0')
    {
			//printf("pBuffer1 ok \r\n");
      return FAILED;
    }

    pBuffer1++;
  }
  return PASSED;
}

//字符串截取
char *myStrncpy(char *dest, const char *src, int n)
{
		int size = sizeof(char)*(n + 1);
		char *tmp = (char*)malloc(size); // 开辟大小为n+1的临时内存tmp

		if (tmp)	
		{
				memset(tmp, '\0', size); // 将内存初始化为0
				memcpy(tmp, src, size - 1); // 将src的前n个字节拷贝到tmp
				memcpy(dest, tmp, size); // 将临时空间tmp的内容拷贝到dest

				free(tmp);// 释放内存
				return dest;
		}
		else
		{
				return NULL;
		}
}

//得到软件编译时间 
RTC_TIME_DEF GetSoftWareBuildTargetTime(void)
{
		char arrDate[20]; //Jul 03 2018
		char arrTime[20]; //06:17:05
		char pDest[20];
		RTC_TIME_DEF stTime;

		sprintf(arrDate,"%s",__DATE__);//Jul 03 2018
		sprintf(arrTime,"%s",__TIME__);//06:17:05
	
    //char *strncpy(char *dest, const char *src, int n)
		//(char*)(&(pDest[0])) = myStrncpy(pDest, arrDate, 3);
		sprintf(pDest, "%s", myStrncpy(pDest, arrDate, 3));

		if(strcmp(pDest, "Jan") == 0) 
			stTime.nMonth = 1;
		else if (strcmp(pDest, "Feb") == 0) 
			stTime.nMonth = 2;
    else if (strcmp(pDest, "Mar") == 0) 
			stTime.nMonth = 3;
		else if (strcmp(pDest, "Apr") == 0) 
			stTime.nMonth = 4;
		else if (strcmp(pDest, "May") == 0) 
			stTime.nMonth = 5;
		else if (strcmp(pDest, "Jun") == 0) 
			stTime.nMonth = 6;
		else if (strcmp(pDest, "Jul") == 0) 
			stTime.nMonth = 7;
		else if (strcmp(pDest, "Aug") == 0) 
			stTime.nMonth = 8;
		else if (strcmp(pDest, "Sep") == 0) 
			stTime.nMonth = 9;
		else if (strcmp(pDest, "Oct") == 0) 
			stTime.nMonth = 10;
		else if (strcmp(pDest, "Nov") == 0) 
			stTime.nMonth = 11;
		else if (strcmp(pDest, "Dec") == 0) 
			stTime.nMonth = 12;
		else stTime.nMonth = 1;

		sprintf(pDest, "%s", myStrncpy(pDest, arrDate+4, 2));
		//int atoi(const char *nptr);
		stTime.nDay = atoi(pDest);

		sprintf(pDest, "%s", myStrncpy(pDest, arrDate + 4 + 3, 4));
		//int atoi(const char *nptr);
		stTime.nYear = atoi(pDest);

		//time
		sprintf(pDest, "%s", myStrncpy(pDest, arrTime, 2));
		stTime.nHour = atoi(pDest);
		sprintf(pDest, "%s", myStrncpy(pDest, arrTime+3, 2));
		stTime.nMinute = atoi(pDest);
		sprintf(pDest, "%s", myStrncpy(pDest, arrTime + 3 + 3, 2));
		stTime.nSecond = atoi(pDest);

		return stTime;
}

char * GetSoftWareTime()
{
   char tmp[5] = {0};
	 char *tmpp = tmp;
	 memset(tmp, '\0', 5);
	 sprintf(tmp, "%02d", GetSoftWareBuildTargetTime().nMonth);
	 sprintf(tmp + 2, "%02d", GetSoftWareBuildTargetTime().nDay);
	 
	 return tmpp;
}

