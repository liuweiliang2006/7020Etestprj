#include "Sim80x.h"
#include "HeartbeatPacket.h"

extern CONFIG_GPRS_t CONFIG_GPRS;  
extern uint16_t connectStep;

#if (_SIM80X_USE_GPRS==1)

//#########################################################################################################
void     GPRS_UserHttpGetAnswer(char *data,uint32_t StartAddress,uint16_t dataLen)
{

}

//#########################################################################################################
//####################################################################################################
//连接到GPRS网络
bool  GPRS_ConnectToServer()
{
    char str[64];
	if(Sim80x.Modem_Type == Quectel_M26)
	{
	 	sprintf(str,"AT+QIOPEN=\"TCP\",\"%s\",\"%s\"\r\n",CONFIG_GPRS.Server_IP,CONFIG_GPRS.Socket_Port);
	}
	else
	{
		sprintf(str,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"\r\n",CONFIG_GPRS.Server_IP,CONFIG_GPRS.Socket_Port);
	}
	
    for(;;)
    {
			  if(Sim80x_SendAtCommand(str,4000,2,"\r\nCONNECT OK\r\n","ALREADY CONNECT"))
        //if(Sim80x_SendAtCommand(str,10000,1,"\r\nCONNECT OK\r\n"))
        {
						#if (_SIM80X_DEBUG==1)
						printf("\r\nGPRS_ConnectToNetwork() ---> OK\r\n");
						#endif
            return true;
        } else {
            #if (_SIM80X_DEBUG==1)
						printf("\r\nGPRS_ConnectToNetwork() ---> ERROR\r\n");
						#endif
            return false;
        }
    }
}

//发送数据到socket
bool GPRS_TcpSendMessage(char *message)
{
    uint8_t tryCnt = 0;
    uint8_t enter = 0x1a;

    for(;;)
    {
        if((Sim80x.Modem_Type == Quectel_M26 ? Sim80x_SendAtCommand("AT+QISEND\r\n",5000,1,">") : Sim80x_SendAtCommand("AT+CIPSEND\r\n",5000,1,">")) > 0)
        {
            Sim80x_SendString(message);
            Sim80x_SendRaw(&enter,1);
            return true;
        } 
				else
				{

            if(tryCnt < 1)//重试1次,一个发2次
            {
                tryCnt++;
							  osDelay(1000);
            } 
						else
            {
                return false;
            }
        }
        osDelay(100);
    }
    return false;
//	  char tempBuffer[200]={0};
//    uint8_t pos = 0;
//		//printf("message %s\r\n",message);
//		while(*message != '\0')
//		{
//			//printf("message1 %s\r\n",message);
//			while(*message != '\0' && pos < 150)
//			{
//			   tempBuffer[pos] = *message;
//				 pos++;
//				 message++;
//			}
//			//printf("message2 %s\r\n",sendBuffer);
//			pos = 0;
//			for(;;)
//			{
//				  //printf("message3 %s\r\n",sendBuffer);
//					if(Sim80x_SendAtCommand("AT+CIPSEND\r\n",2000,1,">"))
//					{
//							Sim80x_SendString(tempBuffer);
//							Sim80x_SendRaw(&enter,1);
//							break;
//					} 
//					else 
//					{

//							if(tryCnt < 1)//重试1次,一个发2次
//							{
//									tryCnt++;
//									osDelay(1000);
//							} 
//							else
//							{
//									return false;
//							}
//					}
//					osDelay(100);
//			}
//			
//			memset(tempBuffer,0,200);
//		}
}

#endif
