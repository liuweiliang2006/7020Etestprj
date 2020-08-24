
#include "Sim80X.h"
#include "Sim80XConfig.h"
#include "Gsm.h"
#include "package.h"
#include "rtc.h"
#include "LogUtils.h"

uint8_t GSM_ON_FLAG = 0; //模块上电标志
extern osMessageQId myQueueGPRSDataHandle;
extern osThreadId myTaskSim80xBufHandle;

Sim80x_t      Sim80x;
osThreadId 		Sim80xTaskHandle;
osThreadId 		Sim80xBuffTaskHandle;

void 	        StartSim80xTask(void const * argument);
void 	        StartSim80xBuffTask(void const * argument);
//######################################################################################################################
//######################################################################################################################
//######################################################################################################################

#define ENABLE_TIMEOUT 0

gsm_data_record gsm_global_data = { "\0", 0, 0};


void	Sim80x_SendString(char *str)
{
	  uint32_t i = 0;
//    HAL_UART_Receive_IT(&_SIM80X_USART,&Sim80x.UsartRxTemp,1);
		while(HAL_UART_Receive_IT(&_SIM80X_USART,&Sim80x.UsartRxTemp,1) != HAL_OK )   
		{
				i++;
				if( i > 10000 )
				{
						_SIM80X_USART.RxState = HAL_UART_STATE_READY;
						__HAL_UNLOCK(&_SIM80X_USART);
						i = 0;
				}
		}  
	
#if (_SIM80X_DMA_TRANSMIT==1)
    while(_SIM80X_USART.hdmatx->State != HAL_DMA_STATE_READY)
        osDelay(10);
    HAL_UART_Transmit_DMA(&_SIM80X_USART,(uint8_t*)str,strlen(str));
    while(_SIM80X_USART.hdmatx->State != HAL_DMA_STATE_READY)
        osDelay(10);
#else
    HAL_UART_Transmit(&_SIM80X_USART,(uint8_t*)str,strlen(str),100);
    osDelay(10);
#endif
}
//######################################################################################################################
void  Sim80x_SendRaw(uint8_t *Data,uint16_t len)
{
	  uint32_t i = 0;
//    HAL_UART_Receive_IT(&_SIM80X_USART,&Sim80x.UsartRxTemp,1); 
		while(HAL_UART_Receive_IT(&_SIM80X_USART,&Sim80x.UsartRxTemp,1) != HAL_OK )   
		{
				i++;
				if( i > 10000 )
				{
						_SIM80X_USART.RxState = HAL_UART_STATE_READY;
						__HAL_UNLOCK(&_SIM80X_USART);
						i = 0;
				}
		} 
#if (_SIM80X_DMA_TRANSMIT==1)
    while(_SIM80X_USART.hdmatx->State != HAL_DMA_STATE_READY)
        osDelay(10);
    HAL_UART_Transmit_DMA(&_SIM80X_USART,Data,len);
    while(_SIM80X_USART.hdmatx->State != HAL_DMA_STATE_READY)
        osDelay(10);
#else
    HAL_UART_Transmit(&_SIM80X_USART,Data,len,100);
    osDelay(10);
#endif

}
//######################################################################################################################
void	Sim80x_RxCallBack(void)
{
	  uint32_t i = 0;
    if((Sim80x.Status.DataTransferMode==0)&&(Sim80x.UsartRxTemp!=0))
    {
        Sim80x.UsartRxLastTime = HAL_GetTick();
        Sim80x.UsartRxBuffer[Sim80x.UsartRxIndex] = Sim80x.UsartRxTemp;
        if(Sim80x.UsartRxIndex < (_SIM80X_BUFFER_SIZE-1))
            Sim80x.UsartRxIndex++;
    }
    else if(Sim80x.Status.DataTransferMode==1)
    {
        Sim80x.UsartRxLastTime = HAL_GetTick();
        Sim80x.UsartRxBuffer[Sim80x.UsartRxIndex] = Sim80x.UsartRxTemp;
        if(Sim80x.UsartRxIndex < (_SIM80X_BUFFER_SIZE-1))
            Sim80x.UsartRxIndex++;
    }

    HAL_UART_Receive_IT(&_SIM80X_USART,&Sim80x.UsartRxTemp,1);


}
//######################################################################################################################
//          Sim80x_SendAtCommand("AT\r\n",                   1000,                1,            "AT\r\r\nOK\r\n")
uint8_t     Sim80x_SendAtCommand(char *AtCommand,int32_t  MaxWaiting_ms,uint8_t HowMuchAnswers,...)
{
    while(Sim80x.Status.Busy == 1)
    {
        osDelay(100);
    }
    Sim80x.Status.Busy = 1;
    Sim80x.AtCommand.FindAnswer = 0;
    Sim80x.AtCommand.ReceiveAnswerExeTime=0;
    Sim80x.AtCommand.SendCommandStartTime = HAL_GetTick();
    Sim80x.AtCommand.ReceiveAnswerMaxWaiting = MaxWaiting_ms;
    memset(Sim80x.AtCommand.ReceiveAnswer,0,sizeof(Sim80x.AtCommand.ReceiveAnswer));
    va_list tag;
    va_start (tag,HowMuchAnswers);
    char *arg[HowMuchAnswers];
    for(uint8_t i=0; i<HowMuchAnswers ; i++)
    {
        arg[i] = va_arg (tag, char *);
        strncpy(Sim80x.AtCommand.ReceiveAnswer[i],arg[i],sizeof(Sim80x.AtCommand.ReceiveAnswer[0]));
    }
    va_end (tag);
    strncpy(Sim80x.AtCommand.SendCommand,AtCommand,sizeof(Sim80x.AtCommand.SendCommand));
    Sim80x_SendString(Sim80x.AtCommand.SendCommand);
    while( MaxWaiting_ms > 0)
    {
        osDelay(10);
        if(Sim80x.AtCommand.FindAnswer > 0)
            return Sim80x.AtCommand.FindAnswer;
        MaxWaiting_ms-=10;
    }
    memset(Sim80x.AtCommand.ReceiveAnswer,0,sizeof(Sim80x.AtCommand.ReceiveAnswer));
    Sim80x.Status.Busy=0;
    return Sim80x.AtCommand.FindAnswer;
}

//######################################################################################################################
//######################################################################################################################
//######################################################################################################################
void  Sim80x_InitValue(void)
{
	  uint8_t MaxLoop = 10;
	  uint8_t Ishave = 0; 
    Sim80x_SendAtCommand("ATE1\r\n",200,1,"ATE1\r\r\nOK\r\n");              //ATE1 命令回复的时候带发送的,ATE0不带
	  //Sim80x_SendAtCommand("AT+CGMI\r\n",200,1,""); // SIMCOM_Ltd
	  
	  do{
			Ishave = Sim80x_SendAtCommand("AT+CGMM\r\n",500,1,"AT+CGMM");
			MaxLoop--;
		}while(MaxLoop > 0 && Ishave == 0);
		
		if(Sim80x.Modem_Type == SIM7020E)
		{
	    Sim80x_SendAtCommand("AT+CBAND=1,3,5,8\r\n",200,1,"AT+CBAND=1,3,5,8\r\r\nOK\r\n"); //china
	 //   Sim80x_SendAtCommand("AT+CBAND=20\r\n",200,1,"AT+CBAND=20\r\r\nOK\r\n"); //kenya
		}
//		if(Sim80x_SendAtCommand("AT+CGMM\r\n",200,1,"AT+CGMM\r\r\nSIM7020E\r\n\r\nOK\r\n") == 1)
//		{
//	    //Sim80x_SendAtCommand("AT+CBAND=1,3,5,8\r\n",200,1,"AT+CBAND=1,3,5,8\r\r\nOK\r\n"); //china
//	    Sim80x_SendAtCommand("AT+CBAND=20\r\n",200,1,"AT+CBAND=20\r\r\nOK\r\n"); //kenya
//		}
	  osDelay(2000);
	  Sim80x_SendAtCommand("AT+CPIN?\r\n",200,1,"\r\n+CPIN:");                 //查看SIM卡的状态
	  osDelay(200);
//    Sim80x_SendAtCommand("AT+COLP=1\r\n",200,1,"AT+COLP=1\r\r\nOK\r\n");  // 联络线确认陈述,貌似是确认打电话有没有接通 设置被叫号码显示
//		osDelay(200);
//    Sim80x_SendAtCommand("AT+CLIP=1\r\n",200,1,"AT+CLIP=1\r\r\nOK\r\n");  // 呼叫线确认陈述 懵逼啊 设置指示来电号码
//		osDelay(200);
//    Sim80x_SendAtCommand("AT+FSHEX=0\r\n",200,1,"AT+FSHEX=0\r\r\nOK\r\n");//
//		osDelay(200);
    Sim80x_SendAtCommand("AT+CGREG=1\r\n",200,1,"AT+CGREG=1\r\r\nOK\r\n");    //网络注册。获得手机的注册状态。
		osDelay(200);
//    Sim80x_SendAtCommand("AT+ECHO?\r\n",200,1,"\r\nOK\r\n");								//回音取消

//    Gsm_MsgSetMemoryLocation(GsmMsgMemory_OnModule);///获取设备上支持存储短信的位置,如果有一个就认为是手机卡,没有手机卡会返回error
//    Gsm_MsgSetFormat(GsmMsgFormat_Text);//CMGF设置短信的格式
//    Gsm_MsgSetTextModeParameter(17,167,0,0);//CSMP设置短信的存储时间等
//    Gsm_MsgGetCharacterFormat();//CSCS获取短信的字符集
//    Gsm_MsgGetFormat();//CMGF获取短信的格式
//    if(Sim80x.Gsm.MsgFormat != GsmMsgFormat_Text)
//        Gsm_MsgSetFormat(GsmMsgFormat_Text);//CMGF设置短信的格式
//    Gsm_MsgGetServiceNumber(); //短信服务中心地址
//    Gsm_MsgGetTextModeParameter();//CSMP获取短信的存储时间等
    Sim80x_GetIMEI(NULL);//获得 GSM  模块的 IMEI（国际移动设备标识）序列号
		osDelay(800);
//    Sim80x_GetLoadVol();//通话音量
//    Sim80x_GetRingVol();//响铃音量
//    Sim80x_GetMicGain();//麦克音量
//    Sim80x_GetToneVol();//声调音量
		#if (_SIM80X_USE_BLUETOOTH==1)
    Bluetooth_SetAutoPair(true);
		osDelay(200);
		#endif
    //Sim80x_SendAtCommand("AT+CREG?\r\n",200,1,"\r\n+CREG:");//查看号码有没有注册上网络
		//osDelay(200);
    Sim80x_UserInit();
}
//######################################################################################################################
void   Sim80x_SaveParameters(void)
{
    Sim80x_SendAtCommand("AT&W\r\n",1000,1,"AT&W\r\r\nOK\r\n");
		#if (_SIM80X_DEBUG==1)
    printf("\r\nSim80x_SaveParameters() ---> OK\r\n");
		#endif
}
//######################################################################################################################
//长按按键初始化
void  Sim80x_SetPower(bool TurnOn)
{
    if(TurnOn==true)
    {
			//printf("Sim80x.Status.Power %d\r\n",Sim80x.Status.Power);
			
			memset(&Sim80x,0,sizeof(Sim80x));
			memset(Sim80x.UsartRxBuffer,0,_SIM80X_BUFFER_SIZE);
			
			if(Sim80x.Status.Power==0)
			{
				//printf("GPRS Power 0\r\n");
				HAL_GPIO_WritePin(RUN_LED_GPIO_Port,RUN_LED_Pin,GPIO_PIN_SET);//运行灯亮
				osDelay(100);
				HAL_GPIO_WritePin(RUN_LED_GPIO_Port,RUN_LED_Pin,GPIO_PIN_RESET);//运行灯灭
			
				//由于绿色板子的传感器与模块同时供电,为避免关闭GPRS也把那个关闭了,所以关闭只有在低功耗的时候
				//这里开启,另外屏幕亮的时候开启
				//HAL_GPIO_WritePin(EN_GPRS_VCC_GPIO_Port,EN_GPRS_VCC_Pin,GPIO_PIN_SET);//GPRS模块供电
				GPRS_PWR(1);
			
				#if (_SIM80X_USE_POWER_KEY==1)
//				if(HAL_GPIO_ReadPin(_SIM80X_POWER_KEY_GPIO,_SIM80X_POWER_KEY_PIN) == GPIO_PIN_SET)
//				{
//					printf("GPRS Power GPIO_PIN_SET\r\n");
				  HAL_GPIO_WritePin(_SIM80X_POWER_KEY_GPIO,_SIM80X_POWER_KEY_PIN,GPIO_PIN_RESET);
				  osDelay(3500);
//				}
//				else
//				{
//					//printf("GPRS Power GPIO_PIN_RESET\r\n");
//					osDelay(1000);
//				}
				HAL_GPIO_WritePin(_SIM80X_POWER_KEY_GPIO,_SIM80X_POWER_KEY_PIN,GPIO_PIN_SET);
				#else
				osDelay(1000);
				#endif
				
				osDelay(200);

				for(uint8_t i=0 ; i<10 ; i++)
				{
						if(Sim80x_SendAtCommand("AT\r\n",1000,1,"AT\r\r\nOK\r\n") == 1)
						{
								break;
						}
						osDelay(200);
				}
				
				if(Sim80x_SendAtCommand("AT\r\n",200,1,"AT\r\r\nOK\r\n") == 1)
				{
						osDelay(100);
						#if (_SIM80X_DEBUG==1)
						printf("\r\nSim80x_SetPower(ON) ---> OK\r\n");
						#endif
						Sim80x.Status.Power=1;
						Sim80x_InitValue();
						//Sim80x.Status.Power=1;
				}
				else
				{
						#if (_SIM80X_USE_POWER_KEY==1)
						HAL_GPIO_WritePin(_SIM80X_POWER_KEY_GPIO,_SIM80X_POWER_KEY_PIN,GPIO_PIN_RESET);
						osDelay(3500);
						HAL_GPIO_WritePin(_SIM80X_POWER_KEY_GPIO,_SIM80X_POWER_KEY_PIN,GPIO_PIN_SET);
						#endif
						if(Sim80x_SendAtCommand("AT\r\n",200,1,"AT\r\nOK\r\n") == 1)
						{
								osDelay(200);
								#if (_SIM80X_DEBUG==1)
								printf("\r\nSim80x_SetPower(ON)2 ---> OK\r\n");
								#endif
								Sim80x.Status.Power=1;
								Sim80x_InitValue();
								//Sim80x.Status.Power=1;
						}
						else
						{
								Sim80x.Status.Power=0;
						}
				}
			}
			
			LogWrite(Connect,"Connection",NULL);
    }
    else
    {
			  //if(Sim80x.Status.Power==1)
				{
					//if(Sim80x_SendAtCommand("AT\r\n",200,1,"AT\r\r\nOK\r\n") == 1)
					{
						//#if (_SIM80X_USE_POWER_KEY==0)
								if(Sim80x.Modem_Type == Quectel_M26)
								{
	 								Sim80x_SendAtCommand("AT+QPOWD=1\r\n",2000,1,"\r\nOK\r\n");
								}
								else
								{
									Sim80x_SendAtCommand("AT+CPOWD=1\r\n",2000,1,"\r\nOK\r\n");
								}
						//#endif
					}
				}
				
//				#if (_SIM80X_USE_POWER_KEY==1)
//				HAL_GPIO_WritePin(_SIM80X_POWER_KEY_GPIO,_SIM80X_POWER_KEY_PIN,GPIO_PIN_RESET);
//				osDelay(3500);//最少1.5秒 超过33秒模块会重新开机
//				HAL_GPIO_WritePin(_SIM80X_POWER_KEY_GPIO,_SIM80X_POWER_KEY_PIN,GPIO_PIN_SET);
//				#endif
				
				#if (_SIM80X_DEBUG==1)
				printf("\r\nSim80x_SetPower(OFF) ---> OK\r\n");
				#endif
				
				Sim80x.Status.Power=0;
			  Sim80x.Status.Signal=0;

				HAL_GPIO_WritePin(RUN_LED_GPIO_Port,RUN_LED_Pin,GPIO_PIN_RESET);//灯灭意识着电源关闭了
				HAL_GPIO_WritePin(_SIM80X_POWER_KEY_GPIO,_SIM80X_POWER_KEY_PIN,GPIO_PIN_RESET);
				GPRS_PWR(0);//GPRS模块断电
				
				LogWrite(Connect,"disConnection",NULL);
				
				osDelay(1000);
    }
}
//######################################################################################################################
void Sim80x_SetFactoryDefault(void)
{
    Sim80x_SendAtCommand("AT&F0\r\n",1000,1,"AT&F0\r\r\nOK\r\n");
#if (_SIM80X_DEBUG==1)
    printf("\r\nSim80x_SetFactoryDefault() ---> OK\r\n");
#endif
}
//######################################################################################################################
//获得 GSM  模块的 IMEI（国际移动设备标识）序列号
void  Sim80x_GetIMEI(char *IMEI)
{
    Sim80x_SendAtCommand("AT+GSN\r\n",1000,1,"\r\nOK\r\n");
#if (_SIM80X_DEBUG==1)
    printf("\r\nSim80x_GetIMEI(%s) ---> OK\r\n",Sim80x.IMEI);
#endif
}
//######################################################################################################################
uint8_t  Sim80x_GetRingVol(void)
{
    uint8_t answer;
    answer=Sim80x_SendAtCommand("AT+CRSL?\r\n",1000,2,"\r\nOK\r\n","\r\n+CME ERROR:");
    if(answer==1)
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_GetRingVol(%d) <--- OK\r\n",Sim80x.RingVol);
#endif
        return Sim80x.RingVol;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_GetRingVol() <--- ERROR\r\n");
#endif
        return 0;
    }
}
//######################################################################################################################
bool  Sim80x_SetRingVol(uint8_t Vol_0_to_100)
{
    uint8_t answer;
    char str[16];
    snprintf(str,sizeof(str),"AT+CRSL=%d\r\n",Vol_0_to_100);
    answer=Sim80x_SendAtCommand(str,1000,2,"\r\nOK\r\n","\r\n+CME ERROR:");
    if(answer==1)
    {
        Sim80x.RingVol=Vol_0_to_100;
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_SetRingVol(%d) ---> OK\r\n",Sim80x.RingVol);
#endif
        return true;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_SetRingVol(%d) ---> ERROR\r\n",Sim80x.RingVol);
#endif
        return false;
    }
}
//######################################################################################################################
uint8_t  Sim80x_GetLoadVol(void)
{
    uint8_t answer;
    answer=Sim80x_SendAtCommand("AT+CLVL?\r\n",1000,2,"\r\nOK\r\n","\r\n+CME ERROR:");
    if(answer==1)
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_GetLoadVol(%d) <--- OK\r\n",Sim80x.LoadVol);
#endif
        return Sim80x.LoadVol;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_GetLoadVol() <--- ERROR\r\n");
#endif
        return 0;
    }
}
//######################################################################################################################
bool  Sim80x_SetLoadVol(uint8_t Vol_0_to_100)
{
    uint8_t answer;
    char str[16];
    snprintf(str,sizeof(str),"AT+CLVL=%d\r\n",Vol_0_to_100);
    answer=Sim80x_SendAtCommand(str,1000,2,"\r\nOK\r\n","\r\n+CME ERROR:");
    if(answer==1)
    {
        Sim80x.LoadVol=Vol_0_to_100;
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_SetLoadVol(%d) ---> OK\r\n",Sim80x.LoadVol);
#endif
        return true;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_SetLoadVol(%d) ---> ERROR\r\n",Sim80x.LoadVol);
#endif
        return false;
    }
}
//######################################################################################################################
Sim80xWave_t   Sim80x_WaveGetState(void)
{
    Sim80x_SendAtCommand("AT+CREC?\r\n",1000,1,"\r\n+CREC:");
#if (_SIM80X_DEBUG==1)
    printf("\r\nSim80x_WaveGetState(%d) ---> OK\r\n",Sim80x.WaveState);
#endif
    return Sim80x.WaveState;
}
//######################################################################################################################
bool  Sim80x_WaveRecord(uint8_t ID,uint8_t TimeLimitInSecond)
{
    uint8_t answer;
    char str[32];
    snprintf(str,sizeof(str),"AT+CREC=1,\"C:\\User\\%d.amr\",0,%d\r\n",ID,TimeLimitInSecond);
    answer = Sim80x_SendAtCommand(str,3000,1,"\r\nOK\r\n");
    if(answer == 1)
    {
        Sim80x.WaveState = Sim80xWave_Recording;
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_WaveRecord(Recording) ---> OK\r\n");
#endif
        return true;
    }
    else
    {
        Sim80x.WaveState = Sim80xWave_Idle;
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_WaveRecord(Recording) ---> ERROR\r\n");
#endif
        return false;
    }
}
//######################################################################################################################
bool  Sim80x_WavePlay(uint8_t ID)
{
    uint8_t answer;
    char str[64];
    snprintf(str,sizeof(str),"AT+CREC=4,\"C:\\User\\%d.amr\",0,%d\r\n",ID,Sim80x.LoadVol);
    answer = Sim80x_SendAtCommand(str,3000,1,"\r\nOK\r\n");
    if(answer == 1)
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_WavePlay() ---> OK\r\n");
#endif
        Sim80x.WaveState = Sim80xWave_Playing;
        return true;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_WavePlay() ---> ERROR\r\n");
#endif
        Sim80x.WaveState = Sim80xWave_Idle;
        return false;
    }
}
//######################################################################################################################
bool  Sim80x_WaveStop(void)
{
    uint8_t answer;
    answer = Sim80x_SendAtCommand("AT+CREC=5\r\n",1000,2,"\r\nOK\r\n","\r\nERROR\r\n");
    if(answer == 1)
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_WaveStop() ---> OK\r\n");
#endif
        Sim80x.WaveState = Sim80xWave_Idle;
        return true;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_WaveStop() ---> ERROR\r\n");
#endif
        return false;
    }
}
//######################################################################################################################
bool  Sim80x_WaveDelete(uint8_t ID)
{
    uint8_t answer;
    char str[32];
    snprintf(str,sizeof(str),"AT+CREC=3,\"C:\\User\\%d.amr\"\r\n",ID);
    answer = Sim80x_SendAtCommand(str,1000,1,"\r\nOK\r\n");
    if(answer == 1)
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_WaveDelete(%d.amr) ---> OK\r\n",ID);
#endif
        return true;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_WaveDelete(%d.amr) ---> ERROR\r\n",ID);
#endif
        return false;
    }
}
//######################################################################################################################
bool  Sim80x_SetMicGain(uint8_t Channel_0_to_4,uint8_t Gain_0_to_15)
{
    uint8_t answer;
    char str[32];
    snprintf(str,sizeof(str),"AT+CMIC=%d,%d\r\n",Channel_0_to_4,Gain_0_to_15);
    answer = Sim80x_SendAtCommand(str,1000,1,"\r\nOK\r\n");
    if(answer == 1)
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_SetMicGain(%d,%d) ---> OK\r\n",Channel_0_to_4,Gain_0_to_15);
#endif
        Sim80x_GetMicGain();
        return true;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_SetMicGain(%d,%d) ---> ERROR\r\n",Channel_0_to_4,Gain_0_to_15);
#endif
        return false;
    }
}
//######################################################################################################################
bool  Sim80x_GetMicGain(void)
{
    uint8_t answer;
    answer=Sim80x_SendAtCommand("AT+CMIC?\r\n",1000,2,"\r\nOK\r\n","\r\n+CME ERROR:");
    if(answer==1)
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_GetMicGain(%d,%d,%d,%d) <--- OK\r\n",Sim80x.MicGainMain,Sim80x.MicGainAux,Sim80x.MicGainMainHandsfree,Sim80x.MicGainAuxHandsfree);
#endif
        return Sim80x.LoadVol;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_GetMicGain() <--- ERROR\r\n");
#endif
        return 0;
    }
}
//######################################################################################################################
bool  Sim80x_TonePlay(Sim80xTone_t Sim80xTone,uint32_t  Time_ms)
{
    uint8_t answer;
    char str[32];
    snprintf(str,sizeof(str),"AT+STTONE=1,%d,%d\r\n",Sim80xTone,Time_ms);
    answer = Sim80x_SendAtCommand(str,1000,2,"\r\nOK\r\n","\r\n+CME ERROR\r\n");
    if(answer == 1)
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_TonePlay(%d,%d) ---> OK\r\n",Sim80xTone,Time_ms);
#endif
        return true;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_TonePlay() ---> ERROR\r\n");
#endif
        return false;
    }
}
//######################################################################################################################
bool  Sim80x_ToneStop(void)
{
    uint8_t answer;
    answer=Sim80x_SendAtCommand("AT+STTONE=0\r\n",1000,2,"\r\nOK\r\n","\r\n+CME ERROR:");
    if(answer==1)
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_ToneStop() <--- OK\r\n");
#endif
        return true;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_ToneStop() <--- ERROR\r\n");
#endif
        return false;
    }
}
//######################################################################################################################
uint8_t Sim80x_GetToneVol(void)
{
    uint8_t answer;
    answer=Sim80x_SendAtCommand("AT+SNDLEVEL?\r\n",1000,2,"\r\nOK\r\n","\r\n+CME ERROR:");
    if(answer==1)
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_GetToneVol(%d) <--- OK\r\n",Sim80x.ToneVol);
#endif
        return Sim80x.ToneVol;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_GetToneVol() <--- ERROR\r\n");
#endif
        return 0;
    }

}
//######################################################################################################################
bool  Sim80x_SetToneVol(uint8_t Vol_0_to_100)
{
    uint8_t answer;
    char str[32];
    snprintf(str,sizeof(str),"AT+SNDLEVEL=0,%d\r\n",Vol_0_to_100);
    answer = Sim80x_SendAtCommand(str,1000,2,"\r\nOK\r\n","\r\n+CME ERROR\r\n");
    if(answer == 1)
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_SetToneVol(%d) ---> OK\r\n",Vol_0_to_100);
#endif
        Sim80x_GetToneVol();
        return true;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_SetToneVol(%d) ---> ERROR\r\n",Vol_0_to_100);
#endif
        return false;
    }

}
//######################################################################################################################
bool  Sim80x_SetRingTone(uint8_t Tone_0_to_19,bool Save)
{
    uint8_t answer;
    char str[32];
    snprintf(str,sizeof(str),"AT+CALS=%d\r\n",Tone_0_to_19);
    answer = Sim80x_SendAtCommand(str,1000,2,"\r\nOK\r\n","\r\n+CME ERROR\r\n");
    if(answer == 1)
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_SetRingTone(%d) ---> OK\r\n",Tone_0_to_19);
#endif
        if(Save==true)
            Sim80x_SaveParameters();
        return true;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_SetRingTone(%d) ---> ERROR\r\n",Tone_0_to_19);
#endif
        return false;
    }
}
//######################################################################################################################
bool  Sim80x_SetEchoParameters(uint8_t  SelectMic_0_or_1,uint16_t NonlinearProcessingRemove,uint16_t AcousticEchoCancellation,uint16_t NoiseReduction,uint16_t NoiseSuppression)
{
    uint8_t answer;
    char str[64];
    snprintf(str,sizeof(str),"AT+ECHO=%d,%d,%d,%d,%d,1\r\n",SelectMic_0_or_1,NonlinearProcessingRemove,AcousticEchoCancellation,NoiseReduction,NoiseSuppression);
    answer = Sim80x_SendAtCommand(str,1000,2,"\r\nOK\r\n","\r\n+CME ERROR\r\n");
    if(answer == 1)
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_SetEchoParameters() ---> OK\r\n");
#endif
        Sim80x_SendAtCommand("AT+ECHO?\r\n",200,1,"\r\nOK\r\n");
        return true;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nSim80x_SetEchoParameters() ---> ERROR\r\n");
#endif
        Sim80x_SendAtCommand("AT+ECHO?\r\n",200,1,"\r\nOK\r\n");
        return false;
    }
}
//######################################################################################################################
//######################################################################################################################
//######################################################################################################################
//系统启动初始化任务
void	Sim80x_Init(osPriority Priority)
{
	#if (_SIM80X_USE_POWER_KEY==1)  
  HAL_GPIO_WritePin(_SIM80X_POWER_KEY_GPIO,_SIM80X_POWER_KEY_PIN,GPIO_PIN_SET);
  #else
  osDelay(1000);
  #endif
	memset(&Sim80x,0,sizeof(Sim80x));
	memset(Sim80x.UsartRxBuffer,0,_SIM80X_BUFFER_SIZE);
	HAL_UART_Receive_IT(&_SIM80X_USART,&Sim80x.UsartRxTemp,1);
	//处理GPRS链接过程中需要干的事情
  osThreadDef(Sim80xTask, StartSim80xTask, Priority, 0, 256);
  Sim80xTaskHandle = osThreadCreate(osThread(Sim80xTask), NULL);
	//接收到模块数据后处理各种AT指令
  osThreadDef(Sim80xBuffTask, StartSim80xBuffTask, Priority, 0, 256);
  Sim80xBuffTaskHandle = osThreadCreate(osThread(Sim80xBuffTask), NULL);
  for(uint8_t i=0 ;i<50 ;i++)  
  {
    if(Sim80x_SendAtCommand("AT\r\n",200,1,"AT\r\r\nOK\r\n") == 1)
      break;
    osDelay(200);
  }  
  Sim80x_SetPower(true); 
}


//void AppObjCreate (void)
//{
//	/* 创建二值信号量，首次创建信号量计数值是0 */
//	Semaphore_Uart_Rec = xSemaphoreCreateBinary();
//	
//	if(Semaphore_Uart_Rec == NULL)
//    {
//			printf("Semaphore creat failed!\r\n");
//        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
//    }
//}

////######################################################################################################################

//uint8_t Analysis_AT_Cmd(char *pdata)
//{
//	return 0;
//}
//uint8_t Analysis_CSQ_Cmd(char *pdata)
//{
//	return 0;
//}
//uint8_t Analysis_QIREGAPP_Cmd(char *pdata)
//{
//	return 0;
//}
//uint8_t Analysis_QIACT_Cmd(char *pdata)
//{
//	return 0;
//}
//uint8_t Analysis_QILOCIP_Cmd(char *pdata)
//{
//	return 0;
//}
//uint8_t Analysis_QSSLCFG_Cmd(char *pdata)
//{
//	return 0;
//}
//uint8_t Analysis_QHTTPURL_Cmd(char *pdata)
//{
//	return 0;
//}
//uint8_t Analysis_QHTTPGET_Cmd(char *pdata)
//{
//	return 0;
//}
//uint8_t Analysis_QHTTPREAD_Cmd(char *pdata)
//{
//	return 0;
//}
//uint8_t Analysis_QIDEACT_Cmd(char *pdata)
//{
//	return 0;
//}
//void  Sim80x_BufferProcess(void)
//{
//	char      *strStart,*str1,*str2;
//	int32_t   tmp_int32_t;
//	BaseType_t xResult;
//	uint8_t u8ATNum=0;

////    strStart = (char*)&Sim80x.UsartRxBuffer[0];
//	while(1)
//	{
//		xResult = xSemaphoreTake(Semaphore_Uart_Rec, (TickType_t)portMAX_DELAY);
//		if(xResult == pdTRUE)
//		{
//			xQueueReceive(SendATQueue, (void *)&u8ATNum, (TickType_t)0);
//			printf("ATNO.=%d,rec:%s\r\n",u8ATNum,&Sim80x.UsartRxBuffer[0]);
//			memset(Sim80x.UsartRxBuffer,0,_SIM80X_BUFFER_SIZE);
//			Sim80x.UsartRxIndex = 0;
//		}
//		
//	}
//    //##################################################
//    //+++       Buffer Process
//    //##################################################
////	str1 = strstr(strStart,"OK");
////	if(str1 != NULL)
////	{
////		str1 = strstr(strStart,"CSQ");
////		if(str1 != NULL)
////		{
////			str1 = strstr(strStart,"99");
////			if(str1 != NULL)
////			{
////				Sim80x.AtCommand.FindAnswer = 0;
////			}
////		}
////		else{
////			Sim80x.AtCommand.FindAnswer = 1;
////		}
////		
////		printf("rec:%s\r\n",strStart);
////		memset(Sim80x.UsartRxBuffer,0,_SIM80X_BUFFER_SIZE);
////	}
////	str1 = strstr(strStart,"CONNECT");
////	if(str1 != NULL)
////	{
////		Sim80x.AtCommand.FindAnswer = 1;
////		printf("rec:%s\r\n",strStart);
////		memset(Sim80x.UsartRxBuffer,0,_SIM80X_BUFFER_SIZE);
////	}
////	
//////	str1 = strstr(strStart,"QHTTPGET");
//////	if(str1 != NULL)
//////	{
//////		Sim80x.AtCommand.FindAnswer = 1;
//////		printf("rec:%s\r\n",strStart);
//////		memset(Sim80x.UsartRxBuffer,0,_SIM80X_BUFFER_SIZE);
//////	}
////	
////	str1 = strstr(strStart,"ERROR");
////	if(str1 != NULL)
////	{
////		Sim80x.AtCommand.FindAnswer = 1;
////		printf("rec:%s\r\n",strStart);
////		memset(Sim80x.UsartRxBuffer,0,_SIM80X_BUFFER_SIZE);
////	}
////	
////	
////	str1 = strstr(strStart,"QILOCIP");
////	if(str1 != NULL)
////	{
////		Sim80x.AtCommand.FindAnswer = 1;
////		printf("rec:%s\r\n",strStart);
////		memset(Sim80x.UsartRxBuffer,0,_SIM80X_BUFFER_SIZE);
////	}
//	
//	
//    //##################################################
//    //---       Buffer Process
//    //##################################################
////#if (_SIM80X_DEBUG==2)
////    printf("%s",strStart);
////#endif
//    Sim80x.UsartRxIndex=0;
////    memset(Sim80x.UsartRxBuffer,0,_SIM80X_BUFFER_SIZE);
//    Sim80x.Status.Busy=0;
//}

//######################################################################################################################
//######################################################################################################################
//######################################################################################################################
//处理从模块接收到的数据
void StartSim80xBuffTask(void const * argument)
{
    while(1)
    {
        if( ((Sim80x.UsartRxIndex>4) && (HAL_GetTick()-Sim80x.UsartRxLastTime > 50)))
        {
            Sim80x.BufferStartTime = HAL_GetTick();
            Sim80x_BufferProcess();
            Sim80x.BufferExeTime = HAL_GetTick()-Sim80x.BufferStartTime;
        }
        osDelay(10);
				
//				if(GSM_ON_FLAG3 == 0)
//				{
//					break;//循环中不能有跳出语句
//				}
    }
		
//		vTaskDelete(NULL);//删除了还要重建
}

//######################################################################################################################
//处理上电需要干什么,比如发送心跳包,或者时间到了,定时上传,或者要报警了
void StartSim80xTask(void const * argument)
{
	uint32_t TimeForSlowRun=0;
  #if( _SIM80X_USE_GPRS==1)
  uint32_t TimeForSlowRunGPRS=0;
  #endif
  uint8_t UnreadMsgCounter=1;
  while(1)
  {    
    //###########################################
    #if( _SIM80X_USE_BLUETOOTH==1)
    //###########################################
    if(Sim80x.Bluetooth.SPPLen >0 )
    {      
      Bluetooth_UserNewSppData(Sim80x.Bluetooth.SPPBuffer,Sim80x.Bluetooth.SPPLen);
      Sim80x.Bluetooth.SPPLen=0;
    }
    //###########################################
    if(Sim80x.Bluetooth.NeedGetStatus==1)
    {
      Sim80x.Bluetooth.NeedGetStatus=0;
      Bluetooth_GetStatus();
    }    
    //###########################################
    if(Sim80x.Bluetooth.ConnectingRequestProfile != BluetoothProfile_NotSet)
    {
      Bluetooth_UserConnectingSpp();
      Sim80x.Bluetooth.ConnectingRequestProfile = BluetoothProfile_NotSet;          
    }
    //###########################################
    if(Sim80x.Bluetooth.ConnectedID==255)
    {
      Sim80x.Bluetooth.ConnectedID=0;
      Bluetooth_UserNewPairingRequest(Sim80x.Bluetooth.ConnectedName,Sim80x.Bluetooth.ConnectedAddress,Sim80x.Bluetooth.PairingPassword);      
    }
    //###########################################
    #endif
    //###########################################
    //###########################################
    #if( _SIM80X_USE_GPRS==1)
    //###########################################
    if(HAL_GetTick()-TimeForSlowRunGPRS > 5000)
    {
      
      
      TimeForSlowRunGPRS=HAL_GetTick();
    }
    
    
    

    //###########################################
    #endif
    //###########################################
    for(uint8_t i=0 ;i<sizeof(Sim80x.Gsm.HaveNewMsg) ; i++)
    {
      if(Sim80x.Gsm.HaveNewMsg[i] > 0)
      {
        //Gsm_MsgGetMemoryStatus();        
        if(Gsm_MsgRead(Sim80x.Gsm.HaveNewMsg[i])==true)
        {
          Gsm_UserNewMsg(Sim80x.Gsm.MsgNumber,Sim80x.Gsm.MsgDate,Sim80x.Gsm.MsgTime,Sim80x.Gsm.Msg);
          Gsm_MsgDelete(Sim80x.Gsm.HaveNewMsg[i]);
        }
        Gsm_MsgGetMemoryStatus();  
        Sim80x.Gsm.HaveNewMsg[i]=0;
      }        
    }    
    //###########################################
    if(Sim80x.Gsm.MsgUsed > 0)
    {   
      if(Gsm_MsgRead(UnreadMsgCounter)==true)
      {
        Gsm_UserNewMsg(Sim80x.Gsm.MsgNumber,Sim80x.Gsm.MsgDate,Sim80x.Gsm.MsgTime,Sim80x.Gsm.Msg);
        Gsm_MsgDelete(UnreadMsgCounter);
        Gsm_MsgGetMemoryStatus();
      }
      UnreadMsgCounter++;
      if(UnreadMsgCounter==150)
        UnreadMsgCounter=0;      
    }
    //###########################################
    if(Sim80x.Gsm.HaveNewCall == 1)
    {
      Sim80x.Gsm.GsmVoiceStatus = GsmVoiceStatus_Ringing;
      Sim80x.Gsm.HaveNewCall = 0;
      Gsm_UserNewCall(Sim80x.Gsm.CallerNumber);     
    }    
    //###########################################
    if(HAL_GetTick() - TimeForSlowRun > 20000)
    {
      Sim80x_SendAtCommand("AT+CSQ\r\n",200,1,"\r\n+CSQ:");  
      Sim80x_SendAtCommand("AT+CBC\r\n",200,1,"\r\n+CBC:"); 
      Sim80x_SendAtCommand("AT+CGREG?\r\n",200,1,"\r\n+CGREG:");  
      Gsm_MsgGetMemoryStatus();      
      TimeForSlowRun=HAL_GetTick();
    }
    //###########################################
    Gsm_User(HAL_GetTick());
    //###########################################
    osDelay(100);
    
  } 
}


bool sim900a_cmd_with_reply (const char *cmd, const char * reply1, const char * reply2, uint32_t waittime)
{
    memset(&gsm_global_data, 0, sizeof(gsm_data_record));
    gsm_global_data.frame_len = 0;

    if((uint32_t)cmd < 0xFF) {
        sim900a_send_byte((uint32_t)cmd);
    } else {
        sim900a_usart("%s\r\n", cmd );
    }

    printf("--->%s\n", cmd);

    if ((reply1 == NULL) && (reply2 == NULL))
        return true;

#if ENABLE_TIMEOUT
    bool ret = 0;
    gsm_global_data.frame_buf[GSM_DATA_RECORD_MAX_LEN - 1] = '\0';
    TIME_LOOP_MS(waittime) {
        if ((reply1 != NULL) && (reply2 != NULL)) {
            ret = (( bool ) strstr(gsm_global_data.frame_buf, reply1)
                   || ( bool ) strstr(gsm_global_data.frame_buf, reply2));
        } else if (reply1 != 0) {
            ret = (( bool ) strstr(gsm_global_data.frame_buf, reply1));
        } else {
            ret = (( bool ) strstr(gsm_global_data.frame_buf, reply2));
        }

        if(ret) {
            break;
        }
    }

    return ret;
#else
   // delay_ms(waittime);
		osDelay(waittime);

    if ((reply1 != 0) && (reply2 != 0)) {
        return (( bool ) strstr(gsm_global_data.frame_buf, reply1)
                || ( bool ) strstr(gsm_global_data.frame_buf, reply2));
    } else if (reply1 != 0) {
        return (( bool ) strstr(gsm_global_data.frame_buf, reply1));

    } else {
        return (( bool ) strstr(gsm_global_data.frame_buf, reply2));

    }
#endif
}



//封装的发送与读取函数

int httpsendread()
{
		volatile int whiletime_out = 20;
	  char url[]=" https://5yqfk16bvi.execute-api.us-east-1.amazonaws.com/test/helloworld?name=John&city=Seattle";
 
		// 启动任务并设置接入点APN
		while(!sim900a_cmd_with_reply("AT+QIREGAPP", "OK", NULL, GSM_CMD_WAIT_SHORT)) 
		{
				//delay_ms(300);
				osDelay(300);
				whiletime_out--;
				if(whiletime_out == 0)break; 
		}
			whiletime_out = 20;
		//激活移动场景
		while(!sim900a_cmd_with_reply("AT+QIACT", "OK", NULL, GSM_CMD_WAIT_LONG)) 
		{
				//delay_ms(300);
				osDelay(300);
				whiletime_out--;
				if(whiletime_out == 0)break; 
		}
	  whiletime_out = 20;
		while(!sim900a_cmd_with_reply("AT+QSSLCFG=\"https\",1", "OK", NULL, GSM_CMD_WAIT_LONG)) 
		{
				//delay_ms(300);
				osDelay(300);
				whiletime_out--;
				if(whiletime_out == 0)break; 
		}
		 whiletime_out = 20;
		while(!sim900a_cmd_with_reply("AT+QSSLCFG=\"httpsctxi\",0", "OK", NULL, GSM_CMD_WAIT_LONG)) 
		{
				//delay_ms(300);
				osDelay(300);
				whiletime_out--;
				if(whiletime_out == 0)break; 
		}
		whiletime_out = 20;
		while(!sim900a_cmd_with_reply("AT+QHTTPURL=93,60", "CONNECT", NULL, GSM_CMD_WAIT_SHORT)) {
				//delay_ms(300);
				osDelay(300);
				whiletime_out--;
				if(whiletime_out == 0)break; 
		}
		sim900a_cmd_with_reply(url, "OK", NULL, GSM_CMD_WAIT_SHORT);
		whiletime_out = 20;
		while(!sim900a_cmd_with_reply("AT+QHTTPGET=60,120", "OK", NULL, 5000)) {
				//delay_ms(300);
				osDelay(300);
				whiletime_out--;
				if(whiletime_out == 0)break; 
		}
			sim900a_cmd_with_reply("AT+QHTTPREAD", NULL, NULL, GSM_CMD_WAIT_SHORT);
		
		
}






























