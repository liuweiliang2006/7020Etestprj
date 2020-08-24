#include "bsp.h"
#include "CookingSessionReport.h"

extern CookingSessionReport_t CookingSessionReport;
extern REAL_DATA_Credit_t REAL_DATA_Credit;

extern Current_Log_t Current_Log;
extern bool IsSaveREAL_Current_Log;

void LogWrite(LogTypeDef_t LogTypeDef,char * dBuffer,char * pBuffer)
{
  Log_index_t Log_index;
	Log_index.LogTypeDef = LogTypeDef;
	long date,time;//UTC
	GetRTC(&time,&date);
	sprintf(Log_index.datatime,"'%08ld-%06ld'",date,time);
	if(pBuffer != NULL)
	{
	  Log_index.LogAddr = Current_Log.addrBigData;
		Log_index.LogLength = strlen(pBuffer);
		Current_Log.addrBigData += strlen(pBuffer);
	}
	else
	{
		if(LogTypeDef == Cooking)
		{
		   Log_index.LogAddr = FLASH_CookingSessionBase + (REAL_DATA_Credit.CookingSessionEnd) * sizeof(CookingSessionReport_t);
		   Log_index.LogLength = sizeof(CookingSessionReport_t);
		}
		else
		{
				Log_index.LogAddr = 100;
				Log_index.LogLength = 100;
		}
	}

	strncpy(Log_index.Description,dBuffer,20);

	//日志索引从0x200000开始
	W25Q64_Write((uint8_t*)(&Log_index),FLASH_LogIndexBase + Current_Log.indexCurrent * sizeof(Log_index_t),sizeof(Log_index_t));
	
	//日志从0x500000开始
	if(Log_index.LogAddr >= FLASH_LogBase)
	{
	  W25Q64_Write((uint8_t *)pBuffer,Log_index.LogAddr,Log_index.LogLength);
	}
	Current_Log.indexCurrent++;
	IsSaveREAL_Current_Log = true;
	
	//printf("Current_Log.indexCurrent %d\r\n",Current_Log.indexCurrent);
}

void LogRead(uint32_t startIndex,uint32_t endIndex)
{
	uint8_t Rx_Buffer_W25Q64[200];
	char name[15];
	Log_index_t Log_index;
	if(endIndex == 0)
	{
		endIndex = Current_Log.indexCurrent;
	}
	printf("startExportLog index,name,datetime,Description,data\r\n");
	
	if(startIndex < endIndex)
	{
		for(uint32_t i = startIndex ; i < endIndex ; i++)
		{
			//printf("1\r\n");
			memset(&Log_index,0,sizeof(Log_index_t));
			//printf("2\r\n");
			SPI_FLASH_BufferRead((uint8_t*)(&Log_index),0x200000 + i * sizeof(Log_index_t),sizeof(Log_index_t));
			//printf("%d\r\n",sizeof(Log_index_t));
			//printf("%d\r\n",sizeof(LogTypeDef_t));
			memset(Rx_Buffer_W25Q64,0,200);
			//printf("4\r\n");
			switch(Log_index.LogTypeDef)
			{
			  case Connect:
					Log_index.LogAddr = 0;
				  strcpy(name,"Connection");
				  break;
				case Send:
				  strcpy(name,"Send");
				  break;
				case Receive:
				  strcpy(name,"Receive");
				  break;
				case Cooking:
				  strcpy(name,"Cooking");
				  break;
				case RestartNormal:
					Log_index.LogAddr = 0;
				  strcpy(name,"RestartNormal");
				  break;
				case RestartUnNorm:
					Log_index.LogAddr = 0;
				  strcpy(name,"RestartUnNorm");
				  break;
				case ClickButton:
					Log_index.LogAddr = 0;
				  strcpy(name,"ClickButton");
				  break;
				case OpenNeedle:
					Log_index.LogAddr = 0;
				  strcpy(name,"OpenNeedle");
				  break;
				case CloseNeedle:
					Log_index.LogAddr = 0;
				  strcpy(name,"CloseNeedle");
				  break;
				case OpenAssemble:
					Log_index.LogAddr = 0;
				  strcpy(name,"OpenAssemble");
				  break;
				case CloseAssemble:
					Log_index.LogAddr = 0;
				  strcpy(name,"CloseAssemble");
				  break;
				case OpenValve:
					Log_index.LogAddr = 0;
				  strcpy(name,"OpenValve");
				  break;
				case CloseValve:
					Log_index.LogAddr = 0;
				  strcpy(name,"CloseValve");
				  break;
			}
			if(Log_index.LogTypeDef == Cooking)
			{
			   SPI_FLASH_BufferRead((uint8_t*)(&(CookingSessionReport)), Log_index.LogAddr,Log_index.LogLength);
				 printf("Line %d,%s,%s,%s,%s|%s|%s|%s|%s|%s|%s|%s|%s\r\n",(i - startIndex + 1),name,Log_index.datatime,Log_index.Description,CookingSessionReport.CARD_ID,CookingSessionReport.SESSION_END_TYPE,CookingSessionReport.GAS_REMAINING,CookingSessionReport.SESSION_START_TIME,CookingSessionReport.SESSION_END_TIME,CookingSessionReport.START_CUMULATIVE_VOLUME,CookingSessionReport.END_CUMULATIVE_VOLUME,CookingSessionReport.CREDIT_SESSION_START,CookingSessionReport.CREDIT_SESSION_END); 
			}
			else
			{
				if(Log_index.LogAddr >= 0x500000 && Log_index.LogAddr != -1 && Log_index.LogAddr != 0 && Log_index.LogAddr != 100)
				{
					//printf("Log_index.LogAddr %d\r\n",Log_index.LogAddr);
					SPI_FLASH_BufferRead(Rx_Buffer_W25Q64, Log_index.LogAddr, Log_index.LogLength);
				}
				//printf("5\r\n");
				printf("Line %d,%s,%s,%s,%s\r\n",(i - startIndex + 1),name,Log_index.datatime,Log_index.Description,Rx_Buffer_W25Q64); 
			}
			
			Current_Log.indexSendNumber++;
		}
		IsSaveREAL_Current_Log = true;
	}
	
	printf("endExportLog\r\n");
}
