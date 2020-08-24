// Header:
// File Name:
// Author:
// Date:

#include <stdio.h>
#include <stdlib.h>
#include "package.h"
#include "string.h"
#include "Sim80X.h"
#include "gas.h"
#include "rtc.h"
#include "bsp.h"
#include "motor.h"
#include "RechargePacket.h"
#include "ControlPacket.h"
#include "SetupPacket.h"
#include "UsedResetPacket.h"
#include "CustomerCredit.h"
#include "InformationPacket.h"
#include "CookingSessionReport.h"
#include <ctype.h>

extern update_t update;

char rxbufferGPRS[200] = {0};//不停接收缓存的长短
extern CONFIG_Meter_t CONFIG_Meter;	
extern REAL_DATA_Credit_t REAL_DATA_Credit;

extern uint32_t OpenLockTime;

extern bool IsSaveCONFIG_GPRS;
extern bool IsSaveCONFIG_Meter;
extern bool IsSaveREAL_DATA_PARAM;
extern bool IsSaveREAL_DATA_Credit;
extern bool IsSaveUpdate;
extern bool IsNeedTimeing;

extern bool IsNeedInformationResponse;

extern bool IsReceivedCSRPReply;

uint16_t identCnt =0;  //发送接收的唯一识别码
uint32_t sumCnt = 0;    //累加的和

extern uint8_t commandType;

//+IPD,95:##123456789012345,STUP,1234,1234567.67,4000.99,2.472,15.123,0,7,12345,10,1,1,201808272134,345**
//void analyticalPackage(char *buffer,setRequstPackage_t * setRequstPackage)
//{
//    char StrSetbuf[15][15];
//    uint8_t i,j;
////    uint32_t buftemp[20] = {0};
//    if(strstr(buffer,"##")!=NULL)
//    {
//        if(strstr(buffer,"STUP")!=NULL)//Jude the head.
//        {
//            //strncpy(setRequstPackage->meterNumer,buffer+2,15);
//            memset(StrSetbuf, 0 , sizeof(StrSetbuf));
//            sscanf(
//                buffer,"##%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^*]",
//                StrSetbuf[0],//meterNo
//                StrSetbuf[1],//STUP
//                StrSetbuf[2],//Identifier;1234
//                StrSetbuf[3],//Credit in. 1234567.00 10
//                StrSetbuf[4],//unit price. 4000.99  7
//                StrSetbuf[5],//density. 2.472 5
//                StrSetbuf[6],//Tank capacity. 15.123 6
//                StrSetbuf[7],//Up duty.
//                StrSetbuf[8],//Battery low.
//                StrSetbuf[9],//Credit is not enough.
//                StrSetbuf[10],//lowgas volume in tank.
//                StrSetbuf[11],//Currency.
//                StrSetbuf[12],//Open lid.
//                StrSetbuf[13],//yyyy mm dd hh mm
//                StrSetbuf[14]//Verification
//            );

//            strncpy(setRequstPackage->identifier,StrSetbuf[2],4);
//            strncpy(setRequstPackage->rechargeAmountIn,StrSetbuf[3],10);
//            strncpy(setRequstPackage->unitPrice,StrSetbuf[4],7);
//            strncpy(setRequstPackage->LPGDensity,StrSetbuf[5],5);
//            strncpy(setRequstPackage->CylinderNominalCapacity,StrSetbuf[6],6);
//            strncpy(setRequstPackage->UploadPeriod,StrSetbuf[7],1);
//            strncpy(setRequstPackage->WarningTagLowBattery,StrSetbuf[8],1);
//            strncpy(setRequstPackage->WarningTagLowPrepaidBalanceAlarm,StrSetbuf[9],1);
//            strncpy(setRequstPackage->LowGasVolumeInTankAlarmLevel,StrSetbuf[10],2);
//            strncpy(setRequstPackage->Currency,StrSetbuf[11],1);
//            strncpy(setRequstPackage->OpenLid,StrSetbuf[12],1);
//            strncpy(setRequstPackage->datetime,StrSetbuf[13],12);
//            strncpy(setRequstPackage->verification,StrSetbuf[14],3);
//        }
//        else
//        {

//        }
//    }
//}

extern REAL_DATA_PARAM_t REAL_DATA_PARAM;

//unsigned int CheckSum( char *string)				//累加和校验
//{
//	unsigned int uiCheakSum = 0;
//	int i = 0;
//	for (;;)
//	{
//		if(string[i] == '\0')
//		{
//		   break;
//		}
//		uiCheakSum += string[i];
//		i++;
//	}
//	return (uiCheakSum & 0xfff);
//}

//unsigned int CheckJPh(uint32_t addData)				//累加和校验
//{
//	char uiChar[30] = {0};
//	char tmpChar[3] = {0};
//	sprintf(uiChar,"%d",addData);
//	//printf("uiChar %s\r\n", uiChar);
//	uint8_t i = 0;
//	while(uiChar[i] != '\0')
//	{
//	   i++;
//	}
//	
//	//printf("uiChar i %d\r\n", i);
//	
//	if(i > 1)
//	{
//		tmpChar[0] = uiChar[i - 2];
//	}
//	if(i > 0)
//	{
//		tmpChar[1] = uiChar[i - 1];
//	}
//	
//	return atoi(tmpChar);
//}

//承担解码任务,这里的数据是IPD后面的数据
//如果不存在半包粘包的问题,完全不需要一个线程
//存在的价值是为了处理数据中间没来得及发送,需要等,缓存超出了还没有完需要抛弃
//一包半需要先处理一包在等到另外的半包
bool DecodeTask(char onechar)
{
	  static uint8_t index_i = 0;//需要每次插入拷贝的地方  最多256奥
		char *begin,*find,*end;

		if(index_i==199)//如果i=200,说明接收了200个之后还没有触发处理函数,说明有问题
		{
			printf("index_i over\r\n");
			index_i=0;
		  memset(rxbufferGPRS,0,sizeof(rxbufferGPRS));
		}
		if(index_i == 0 && onechar != '#')//如果需要接收的第一个字符不是#,抛弃
		{
			return true;
		}
		
		if(onechar == '#' && index_i > 2)//如果接收到#但是缓存不是1或者2,那么从零开始填充
		{
			index_i = 0;
			memset(rxbufferGPRS,0,sizeof(rxbufferGPRS));
		}
		
		rxbufferGPRS[index_i] = onechar;
		index_i++;
		//if(*rxbufferGPRS != '\0')//说明有接收到数据
		if(index_i > 1)
		{
			find = strstr(rxbufferGPRS,"##");
			if(find!=NULL)//说明接收到了指令
			{
				find = strstr(rxbufferGPRS,"AT+");
				if(find!=NULL)//说明接收到了AT指令,需要抛弃
				{
					index_i = 0;
			    memset(rxbufferGPRS,0,sizeof(rxbufferGPRS));
					return false;
				}
				//else
				{
				
						begin = rxbufferGPRS;
		//				if(strstr(rxbufferGPRS,"##")!=begin)//说明出现的位置不是开始,需要去除开始的无用字符
		//				{
		//					printf("rxbufferGPRS %s begin %s ok\r\n",strstr(rxbufferGPRS,"##"),begin);
		//					printf("move start ## ok\r\n");
		//					for(int m = 0;m < (200- (find - begin));m++)
		//					{
		//						printf("move ing %d ## ok\r\n",m);
		//						*(begin+m) = *(find+m);
		//						*(find+m)='\0';//移动完的空位置补零
		//					}
		//					index_i = index_i - (find - begin);
		//					printf("2 index_i %d\r\n",index_i);
		//					printf("move end ## ok\r\n");
		//				}
						end = strstr(rxbufferGPRS,"**");
						if(end!=NULL)
						{
							//printf("rxbufferGPRS %s ok\r\n",rxbufferGPRS);
							analyticalPackage(rxbufferGPRS,end+2-begin);//需要处理的字节数据,需要处理的字节长度
							memset(rxbufferGPRS,0,sizeof(rxbufferGPRS));
		//						for(int m = 0;m < (200- (end + 2 - begin));m++)
		//						{
		//							*(begin + m) = *(end + 2 + m);
		//							printf("rxbufferGPRS %d %c \r\n",m,*(begin + m));
		//							*(end+m)='\0';
		//						}
							index_i = 0;
		//						printf("3 index_i %d\r\n",index_i);
		//						index_i = index_i - (end + 2 - begin);
		//						printf("4 index_i %d\r\n",index_i);
						}
		//				else
		//				{
		//				  printf("no have end ** ok\r\n");
		//				}
					}
			}
//			else
//			{
//				memset(rxbufferGPRS,0,sizeof(rxbufferGPRS));
//				index_i=0;
//			}	
		}
		
		return true;
}

//*pbDest 输出缓冲区 
//*pszSrc 输入字符串
// nLen 字符串长度/2
void StrToHex(byte *pbDest, char *pszSrc, int nLen)
{
 char h1, h2;
 byte s1, s2;
 for (int i = 0; i < nLen; i++)
 {
  h1 = pszSrc[2 * i];
  h2 = pszSrc[2 * i + 1];
 
  s1 = toupper(h1) - 0x30;
  if (s1 > 9)
   s1 -= 7;
 
  s2 = toupper(h2) - 0x30;
  if (s2 > 9)
   s2 -= 7;
 
  pbDest[i] = s1 * 16 + s2;
 }
}

void analyticalPackage(char *buffer,uint32_t sum)
{
		long yymmdd;
	  long hhmmss;
	
	  char *currPosition;
		char StrSetbuf[100] = {0};
		
		uint8_t elementIndex = 0;
	  bool NextIsEnd = false;
    //printf("analyticalPackage start %d %s\r\n",sum,buffer);
    if(strstr(buffer,"##")!=NULL)
    {
        if(strstr(buffer,"STUP")!=NULL)//Jude the head.
        {
          currPosition = buffer + 2;
					memset(&SetupPacket,0,sizeof(SetupPacket));
					while(currPosition<buffer+sum)
					{
						memset(StrSetbuf,0,sizeof(StrSetbuf));
						if(NextIsEnd == false)
						{
							sscanf(currPosition,"%[^/]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 1;
						}
						else
						{
						  sscanf(currPosition,"%[^*]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 2;
						}	
		
						switch(elementIndex)
						{
							case 0:
							  strncpy(SetupPacket.keyWord,StrSetbuf,4);
								break;
							case 1:
								strncpy(SetupPacket.meterNumer,StrSetbuf,10);
								break;
							case 2:
								strncpy(SetupPacket.identifier,StrSetbuf,4);
								break;
							case 3:
								strncpy(SetupPacket.UpdatePeriod,StrSetbuf,5);
								break;
							case 4:
								strncpy(SetupPacket.StartPeriod,StrSetbuf,5);
								break;
							case 5:
								strncpy(SetupPacket.LowBattery,StrSetbuf,4);
								break;
							case 6:
								strncpy(SetupPacket.LowCredit,StrSetbuf,5);
								break;
							case 7:
								strncpy(SetupPacket.LowGasVolume,StrSetbuf,4);
								break;
							case 8:
								strncpy(SetupPacket.GAS_REMAINING,StrSetbuf,6);
								break;
							case 9:
								strncpy(SetupPacket.Currency,StrSetbuf,3);
								break;
							case 10:
								strncpy(SetupPacket.datetime,StrSetbuf,12);
								sscanf(SetupPacket.datetime+2,"%6ld",&yymmdd);
								sscanf(SetupPacket.datetime+8,"%4ld",&hhmmss);
								//SetRTC(hhmmss * 100,yymmdd);
								NextIsEnd = true;
								break;
							case 11:
								strncpy(SetupPacket.verification,StrSetbuf,strlen(StrSetbuf));
								break;
						}
						elementIndex++;
					}
          if(NextIsEnd == true)
					{
						SetToMeter(&SetupPacket);
						
						HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
						osDelay(500);
						HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
						
						IsSaveCONFIG_Meter = true;
						IsSaveREAL_DATA_PARAM = true;
					}
        }
				else if(strstr(buffer,"ADMO")!=NULL)
				{
					currPosition = buffer + 2;
					memset(&RechargePacket,0,sizeof(RechargePacket));
					while(currPosition<buffer+sum)
					{
						memset(StrSetbuf,0,sizeof(StrSetbuf));
						if(NextIsEnd == false)
						{
							sscanf(currPosition,"%[^/]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 1;
						}
						else
						{
						  sscanf(currPosition,"%[^*]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 2;
						}	
						//printf("StrSetbuf %s\r\n",StrSetbuf);
		
						switch(elementIndex)
						{
							case 0:
								//strncpy(RechargePacket.meterNumer,StrSetbuf,10);
							  //strncpy(RechargePacket.keyWord,StrSetbuf,4);
								break;
							case 1:
								strncpy(RechargePacket.meterNumer,StrSetbuf,10);
								break;
							case 2:
								strncpy(RechargePacket.CUSTOMER_ID,StrSetbuf,7);
								break;
							case 3:
								strncpy(RechargePacket.identifier,StrSetbuf,4);
								break;
							case 4:
								strncpy(RechargePacket.CARD_ID,StrSetbuf,18);
								break;
							case 5:
								strncpy(RechargePacket.rechargeAmountIn,StrSetbuf,12);
								break;
							case 6:
								strncpy(RechargePacket.unitPrice,StrSetbuf,8);
								break;
							case 7:
								strncpy(RechargePacket.LPGDensity,StrSetbuf,5);
								break;
							case 8:
								strncpy(RechargePacket.ADDORSUB,StrSetbuf,3);
								break;
							case 9:
								strncpy(RechargePacket.datetime,StrSetbuf,12);
								sscanf(RechargePacket.datetime+2,"%6ld",&yymmdd);
								sscanf(RechargePacket.datetime+8,"%4ld",&hhmmss);
								//SetRTC(hhmmss * 100,yymmdd);
								NextIsEnd = true;
								break;
							case 10:
								strncpy(RechargePacket.verification,StrSetbuf,strlen(StrSetbuf));
							  //printf("StrSetbuf %s\r\n",StrSetbuf);
							  //printf("StrSetbuf %d\r\n",strlen(StrSetbuf));
								break;
						}
						elementIndex++;
					}
					
          if(NextIsEnd == true)
					{
							while((IsNeedRepayReCharge == true 
								|| commandType != 0) 
							  && GSM_ON_FLAG == 1)
							{
								osDelay(500);
								//printf("stop here\r\n");
							}
							
							IsNeedRepayReCharge = true;
//							RechargeToMeter(&RechargePacket);
//						
//							HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
//							osDelay(200);
//							HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);

//							IsSaveREAL_DATA_Credit = true;//充值
					}
				}
				else if(strstr(buffer,"CONT")!=NULL)
        {	 
					currPosition = buffer + 2;
					memset(&controlPacket,0,sizeof(controlPacket));
					while(currPosition<buffer+sum)
					{
						memset(StrSetbuf,0,sizeof(StrSetbuf));
						if(NextIsEnd == false)
						{
							sscanf(currPosition,"%[^/]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 1;
							//printf("1:%s\r\n",StrSetbuf);
						}
						else
						{
						  sscanf(currPosition,"%[^*]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 2;
							//printf("2:%s\r\n",StrSetbuf);
						}	
		
						switch(elementIndex)
						{
							case 0:
								strncpy(controlPacket.keyWord,StrSetbuf,4);
								break;
							case 1:
								strncpy(controlPacket.meterNumer,StrSetbuf,10);
								break;
							case 2:
								strncpy(controlPacket.identifier,StrSetbuf,4);
								break;
							case 3:
								strncpy(controlPacket.Openlid,StrSetbuf,1);
								break;
							case 4:
								strncpy(controlPacket.Valve,StrSetbuf,1);
								break;
							case 5:
								strncpy(controlPacket.NEEDLE,StrSetbuf,1);
								break;
							case 6:
								strncpy(controlPacket.tankLock,StrSetbuf,1);
								break;
							case 7:
								strncpy(controlPacket.datetime,StrSetbuf,12);
								sscanf(controlPacket.datetime+2,"%6ld",&yymmdd);
								sscanf(controlPacket.datetime+8,"%4ld",&hhmmss);
								//SetRTC(hhmmss * 100,yymmdd);
								NextIsEnd = true;
								break;
							case 8:
								strncpy(controlPacket.verification,StrSetbuf,strlen(StrSetbuf));
								break;
						}
						elementIndex++;
					}
					
					if(NextIsEnd == true)
					{
							//开盖
							if(controlPacket.Openlid[0] != '2')
							{	
								if(controlPacket.Openlid[0] == '1')
								{
									OpenLockTime = 10;
									glockStatus = motor_open;
									gassembleStatus = motor_open;
									glockSet = motor_open;
								}
								if(controlPacket.Openlid[0] == '0')
								{
									OpenLockTime = 0;
									glockStatus = motor_close;
									gassembleStatus = motor_close;
									glockSet = motor_close;
								}
							}
							//阀门	
							if(controlPacket.Valve[0] != '2')
							{					
								if(controlPacket.Valve[0] == '1')
								{
									//gmotorStatus = motor_open;
								}
								if(controlPacket.Valve[0] == '0')
								{
									gmotorStatus = motor_close;
								}
							}	
							//这个现在没有
							if(controlPacket.NEEDLE[0] != '2')
							{				
								if(controlPacket.NEEDLE[0] == '1')
								{
									//gmotorStatus = motor_open;
								}
								if(controlPacket.NEEDLE[0] == '0')
								{
									//gmotorStatus = motor_close;
								}
							}	
							//控制是否可以把仪表从罐体上摘下来,就算可以了也需要手动操作
							if(controlPacket.tankLock[0] != '2')
							{	
								if(controlPacket.tankLock[0] == '1')
								{
									OpenLockTime = 10;
									gassembleStatus = motor_open;
									glockSet = motor_open;
								}
								if(controlPacket.tankLock[0] == '0')
								{
									OpenLockTime = 0;
									gassembleStatus = motor_close;
									glockSet = motor_close;
								}
							}	
							
							IsNeedRepayControl = true; 
							
							HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
							osDelay(500);
							HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);

							IsSaveREAL_DATA_PARAM = true;	
					}					
        }
				else if(strstr(buffer,"CSRP")!=NULL)
        {	 
					currPosition = buffer + 2;
					//memset(&controlPacket,0,sizeof(controlPacket));
					while(currPosition<buffer+sum)
					{
						memset(StrSetbuf,0,sizeof(StrSetbuf));
						if(NextIsEnd == false)
						{
							sscanf(currPosition,"%[^/]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 1;
							//printf("1:%s\r\n",StrSetbuf);
						}
						else
						{
						  sscanf(currPosition,"%[^*]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 2;
							//printf("2:%s\r\n",StrSetbuf);
						}	
		
						switch(elementIndex)
						{
							case 0:
								//keyWord CSRP
								break;
							case 1:
								//TZ1234567
								break;
							case 2:
								//CUSTOMER_ID
								break;
							case 3:
								//strncpy(controlPacket.identifier,StrSetbuf,4);
							  if(stringCmp(StrSetbuf,CookingSessionReport.identifier,4) == PASSED)
								{
								   REAL_DATA_Credit.CookingSessionSendNumber++;
								}
								break;
							case 4:
								//CARD_ID
								break;
							case 5:
								strncpy(controlPacket.datetime,StrSetbuf,12);
								sscanf(controlPacket.datetime+2,"%6ld",&yymmdd);
								sscanf(controlPacket.datetime+8,"%4ld",&hhmmss);
								//SetRTC(hhmmss * 100,yymmdd);
								NextIsEnd = true;
								break;
							case 6:
								strncpy(controlPacket.verification,StrSetbuf,strlen(StrSetbuf));
								break;
						}
						elementIndex++;
					}
					
					if(NextIsEnd == true)
					{
							IsReceivedCSRPReply = true; 
						  IsSaveREAL_DATA_Credit = true; //收到回复了,保存实时信息
					}					
        }
				else if(strstr(buffer,"VURS")!=NULL)
        {
					currPosition = buffer + 2;
					memset(&UsedResetPacket,0,sizeof(UsedResetPacket));
					while(currPosition<buffer+sum)
					{
						memset(StrSetbuf,0,sizeof(StrSetbuf));
						if(NextIsEnd == false)
						{
							sscanf(currPosition,"%[^/]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 1;
						}
						else
						{
						  sscanf(currPosition,"%[^*]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 2;
						}	
		
						switch(elementIndex)
						{
							case 0:
							  strncpy(UsedResetPacket.keyWord,StrSetbuf,4);
								break;
							case 1:
								strncpy(UsedResetPacket.meterNumer,StrSetbuf,10);
								break;
							case 2:
								strncpy(UsedResetPacket.identifier,StrSetbuf,4);
								break;
							case 3:
								strncpy(UsedResetPacket.RealWeight,StrSetbuf,6);
								break;
							case 4:
								strncpy(UsedResetPacket.datetime,StrSetbuf,12);
								sscanf(UsedResetPacket.datetime+2,"%6ld",&yymmdd);
								sscanf(UsedResetPacket.datetime+8,"%4ld",&hhmmss);
								//SetRTC(hhmmss * 100,yymmdd);
								NextIsEnd = true;
								break;
							case 5:
								strncpy(UsedResetPacket.verification,StrSetbuf,strlen(StrSetbuf));
								break;
						}
						elementIndex++;
					}
          
					if(NextIsEnd == true)
					{	
						ResetToMeter(&UsedResetPacket);
						
						HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
						osDelay(500);
						HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
						
						IsSaveREAL_DATA_PARAM = true;	
					}
				}
				else if(strstr(buffer,"CYCH")!=NULL)
        {
					currPosition = buffer + 2;
					while(currPosition<buffer+sum)
					{
						memset(StrSetbuf,0,sizeof(StrSetbuf));
						if(NextIsEnd == false)
						{
							sscanf(currPosition,"%[^/]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 1;
						}
						else
						{
						  sscanf(currPosition,"%[^*]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 2;
						}	
		
						switch(elementIndex)
						{
							case 0:
								break;
							case 1:
								break;
							case 2:
								break;
							case 3:
								break;
							case 4:
								break;
							case 5:
								strncpy(CONFIG_Meter.CYLINDER_No,StrSetbuf,11);
							  //printf("CONFIG_Meter.CYLINDER_No : %s",CONFIG_Meter.CYLINDER_No);
								break;
							case 6:
								sscanf(StrSetbuf,"%f",&(REAL_DATA_PARAM.TankQuality));
							  //printf("REAL_DATA_PARAM.TankQuality : %f",REAL_DATA_PARAM.TankQuality);
								break;
							case 7:
								sscanf(StrSetbuf,"%f",&REAL_DATA_Credit.LPGDensity);
							  //printf("REAL_DATA_Credit.LPGDensity : %f",REAL_DATA_Credit.LPGDensity);
								break;
							case 8:
								strncpy(UsedResetPacket.datetime,StrSetbuf,12);
								sscanf(UsedResetPacket.datetime+2,"%6ld",&yymmdd);
								sscanf(UsedResetPacket.datetime+8,"%4ld",&hhmmss);
								//SetRTC(hhmmss * 100,yymmdd);
								NextIsEnd = true;
								break;
							case 9:
								strncpy(UsedResetPacket.verification,StrSetbuf,strlen(StrSetbuf));
								break;
						}
						elementIndex++;
					}
					if(NextIsEnd == true)
					{	
							IsNeedRepayCYCH = true; 
							
							HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
							osDelay(500);
							HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
								
							IsSaveCONFIG_Meter = true;	
							IsSaveREAL_DATA_PARAM = true;	
					}						
				}
				else if(strstr(buffer,"DAHR")!=NULL)
        {
					currPosition = buffer + 2;
					while(currPosition<buffer+sum)
					{
						memset(StrSetbuf,0,sizeof(StrSetbuf));
						if(NextIsEnd == false)
						{
							sscanf(currPosition,"%[^/]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 1;
						}
						else
						{
						  sscanf(currPosition,"%[^*]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 2;
						}	
		
						switch(elementIndex)
						{
							case 0:
								break;
							case 1:
								break;
							case 2:
								break;
							case 3:
//								strncpy(UsedResetPacket.datetime,StrSetbuf,12);
//								sscanf(UsedResetPacket.datetime+2,"%6ld",&yymmdd);
//								sscanf(UsedResetPacket.datetime+8,"%4ld",&hhmmss);
								//SetRTC(hhmmss * 100,yymmdd);
								NextIsEnd = true;
								break;
							case 4:
//								strncpy(UsedResetPacket.verification,StrSetbuf,strlen(StrSetbuf));
								break;
						}
						elementIndex++;
					}
					if(NextIsEnd == true)
					{	
							IsNeedTimeing = true; 
							
							HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
							osDelay(500);
							HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
								
							//IsSaveCONFIG_Meter = true;	
							//IsSaveREAL_DATA_PARAM = true;	
					}						
				}
				else if(strstr(buffer,"INFQ")!=NULL)
        {
					currPosition = buffer + 2;
					memset(&InformationPacket,0,sizeof(InformationPacket));
					while(currPosition<buffer+sum)
					{
						memset(StrSetbuf,0,sizeof(StrSetbuf));
						if(NextIsEnd == false)
						{
							sscanf(currPosition,"%[^/]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 1;
						}
						else
						{
						  sscanf(currPosition,"%[^*]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 2;
						}	
		
						switch(elementIndex)
						{
							case 0:
								break;
							case 1:
								break;
							case 2:
								strncpy(InformationPacket.identifier,StrSetbuf,4);
								break;
							case 3:
								strncpy(InformationPacket.datetime,StrSetbuf,12);
								sscanf(InformationPacket.datetime+2,"%6ld",&yymmdd);
								sscanf(InformationPacket.datetime+8,"%4ld",&hhmmss);
								//SetRTC(hhmmss * 100,yymmdd);
								NextIsEnd = true;
								break;
							case 4:
								strncpy(InformationPacket.verification,StrSetbuf,strlen(StrSetbuf));
								break;
						}
						elementIndex++;
					}
					if(NextIsEnd == true)
					{	
							HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
							osDelay(500);
							HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
						
							IsNeedInformationResponse = true;
							commandType = 1;	//查询仪表信息
					}
				}
				else if(strstr(buffer,"CCCH")!=NULL)
        {
					currPosition = buffer + 2;
					memset(&CustomerCredit,0,sizeof(CustomerCredit));
					while(currPosition<buffer+sum)
					{
						memset(StrSetbuf,0,sizeof(StrSetbuf));
						if(NextIsEnd == false)
						{
							sscanf(currPosition,"%[^/]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 1;
						}
						else
						{
						  sscanf(currPosition,"%[^*]",StrSetbuf);
							currPosition = currPosition + strlen(StrSetbuf) + 2;
						}	
		        //printf("StrSetbuf %s\r\n",StrSetbuf);
						switch(elementIndex)
						{
							case 0:
								break;
							case 1:
								strncpy(CustomerCredit.meterNumer,StrSetbuf,10);
								break;
							case 2:
								strncpy(CustomerCredit.CUSTOMER_ID,StrSetbuf,7);
								break;
							case 3:
								strncpy(CustomerCredit.identifier,StrSetbuf,4);
								break;
							case 4:
								strncpy(CustomerCredit.CARD_ID,StrSetbuf,18);
								break;
							case 5:
								strncpy(CustomerCredit.datetime,StrSetbuf,12);
								sscanf(CustomerCredit.datetime+2,"%6ld",&yymmdd);
								sscanf(CustomerCredit.datetime+8,"%4ld",&hhmmss);
								//SetRTC(hhmmss * 100,yymmdd);
								NextIsEnd = true;
								break;
							case 6:
								strncpy(UsedResetPacket.verification,StrSetbuf,strlen(StrSetbuf));
								break;
						}
						elementIndex++;
					}
					
					if(NextIsEnd == true)
					{	
							HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
							osDelay(500);
							HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
							
							commandType = 2;	//查询用户信用
					}
				}
				else if(strstr(buffer,"UPDATE")!=NULL)
				{
					 printf("update\r\n");
					 currPosition = buffer + 2;
					 memset(&update,0,sizeof(update));
					 while(currPosition<buffer+sum)
					 {
						memset(StrSetbuf,0,sizeof(StrSetbuf));
						if(NextIsEnd == false)
						{
							 if(elementIndex != 4)
							 {
								 sscanf(currPosition,"%[^/]",StrSetbuf);
								 currPosition = currPosition + strlen(StrSetbuf) + 1;
							 }
							 else
							 {
							   strncpy(StrSetbuf,currPosition,strstr(buffer,".bin") - currPosition + 4);
								 currPosition = currPosition + strlen(StrSetbuf) + 1;
							 }
						}
						else
						{
							 sscanf(currPosition,"%[^*]",StrSetbuf);
						   currPosition = currPosition + strlen(StrSetbuf) + 2;
						} 
						printf("StrSetbuf %s\r\n",StrSetbuf);
						switch(elementIndex)
						{
						 case 0:
							break;
						 case 1:
							break;
						 case 2:
							break;
						 case 3:
							StrToHex((byte*)update.MD5CODE,StrSetbuf,16);
							break;
						 case 4:
							strncpy(update.URL_ADDR,StrSetbuf,strlen(StrSetbuf));
							break; 
						 case 5:
								strncpy(CustomerCredit.datetime,StrSetbuf,12);
								sscanf(CustomerCredit.datetime+2,"%6ld",&yymmdd);
								sscanf(CustomerCredit.datetime+8,"%4ld",&hhmmss);
								//SetRTC(hhmmss * 100,yymmdd);
								NextIsEnd = true;
								break;
							case 6:
								strncpy(UsedResetPacket.verification,StrSetbuf,strlen(StrSetbuf));
								break;
					}
					elementIndex++;
				 }
				 
				 if(NextIsEnd == true)
				 {  
					 update.BOOTFLAG=0xaa; 
					 HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
					 osDelay(500);
					 HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
					 
					 IsSaveUpdate = true;					 
				 }
			 }
    }
}
