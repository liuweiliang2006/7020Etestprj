#include "bsp.h"
#include "printf.h"
#include <ctype.h>

cmd_analyze_struct cmd_analyze;

uint8_t	Usart3RxTemp;

extern bool IsNeedRestart;

extern osThreadId myTaskCmdAnalyzHandle;

extern CONFIG_GPRS_t CONFIG_GPRS;           
extern CONFIG_Meter_t CONFIG_Meter;				 
extern REAL_DATA_PARAM_t REAL_DATA_PARAM;  
extern REAL_DATA_Credit_t REAL_DATA_Credit;
extern Current_Credit_t Current_Credit;
extern Current_Log_t Current_Log;

extern bool IsSaveCONFIG_GPRS;
extern bool IsSaveCONFIG_Meter;
extern bool IsSaveREAL_DATA_PARAM;
extern bool IsSaveREAL_DATA_Credit;
extern bool IsSaveREAL_Current_Log;
extern bool LCD_Delay_Time; 

extern float volt1;    
extern float	y1;      

extern float vert2;    
extern float	y2;     

extern REAL_DATA_CALIBRATION_t REAL_DATA_CALIBRATION;
extern bool IsSaveCALIBRATION;

extern uint32_t logStart;
extern uint32_t logEnd;
extern bool IsNeedReportLog;

extern uint32_t VoltageStart;
extern uint32_t VoltageEnd;
extern bool IsNeedReportVoltage;

//extern bool IsReadVoltage;

extern REAL_Flow_CALIBRATION_t REAL_Flow_CALIBRATION;
extern bool IsSaveFlowCALIBRATION;

//接收数据 
uint32_t rec_count = 0;

///*命令表*/
//const cmd_list_struct cmd_list[3]={
///* 命令    					参数数目    	处理函数        帮助信息                         */   
//{"hello",  							 0,      printf_hello,        "HelloWorld!"},
//{"arg",     						 8,      handle_arg,          "arg <arg1> <arg2>"},
//{"ReadSystemPara",       0,      ReadSystemPara,      "return"},
//{"SetAddr",       			 0,      SetAddr,      "SetAddr"}
//};


/*提供给串口中断服务程序，保存串口接收到的单个字符*/
void fill_rec_buf()
{
	  uint32_t i = 0;
    cmd_analyze.rec_buf[rec_count]=Usart3RxTemp;
	  //printf("Usart3RxTemp %c\r\n",Usart3RxTemp);
	  //myPrintf(cmd_analyze.rec_buf);
	
    if(0x0A==cmd_analyze.rec_buf[rec_count] && 0x0D==cmd_analyze.rec_buf[rec_count-1])
    {
			 //printf("rec_buf %s\r\n",cmd_analyze.rec_buf);
			  /* xHigherPriorityTaskWoken在使用之前必须设置为pdFALSE 
       如果调用vTaskNotifyGiveFromISR()会解除vHandlingTask任务的阻塞状态， 
       并且vHandlingTask任务的优先级高于当前处于运行状态的任务， 
       则xHigherPriorityTaskWoken将会自动被设置为pdTRUE */
       BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			
       rec_count=0;
       
       /*收到一帧数据，向命令行解释器任务发送通知*/
			 /* 发送任务通知，并解锁阻塞在该任务通知下的任务 */ 
       vTaskNotifyGiveFromISR(myTaskCmdAnalyzHandle,&xHigherPriorityTaskWoken);
       
       /*是否需要强制上下文切换*/
			 /* 如果被解锁的任务优先级比当前运行的任务的优先级高 
       则在中断退出前执行一次上下文切换，在中断退出后去执行 
       刚刚被唤醒的优先级更高的任务*/ 
       portYIELD_FROM_ISR(xHigherPriorityTaskWoken );
    }
    else
    {
       rec_count++;
       
       /*防御性代码，防止数组越界*/
       if(rec_count>=CMD_BUF_LEN)
       {
				   printf("rec_count2 %d\r\n",rec_count);
           rec_count=0;
       }
    }
//    if(HAL_UART_Receive_IT(&huart3,&Usart3RxTemp,1) != HAL_OK)
//		{    
//			do
//			{
//					if(HAL_UART_GetState(&huart3) == HAL_UART_STATE_BUSY_TX)
//					{
//							continue;
//					}
//			}while(HAL_UART_Receive_IT(&huart3,&Usart3RxTemp,1)!=HAL_BUSY);                        
//		}	

	  while(HAL_UART_Receive_IT(&huart3,&Usart3RxTemp,1) != HAL_OK )   
		{
				i++;
				if( i > 10000 )
				{
						huart3.RxState = HAL_UART_STATE_READY;
						__HAL_UNLOCK(&huart3);
						i = 0;
				}
		}		
}

/**
* 使用SecureCRT串口收发工具,在发送的字符流中可能带有不需要的字符以及控制字符,
* 比如退格键,左右移动键等等,在使用命令行工具解析字符流之前,需要将这些无用字符以
* 及控制字符去除掉.
* 支持的控制字符有:
*   上移:1B 5B 41
*   下移:1B 5B 42
*   右移:1B 5B 43
*   左移:1B 5B 44
*   回车换行:0D 0A
*  Backspace:08
*  Delete:7F
*/
uint32_t get_true_char_stream(char *dest,const char *src)
{
   uint32_t dest_count=0;
   uint32_t src_count=0;
   
    while(src[src_count]!=0x0D && src[src_count+1]!=0x0A)
    {
       if(isprint(src[src_count]))
       {
           dest[dest_count++]=src[src_count++];
       }
       else
       {
           switch(src[src_count])
           {
                case    0x08:                          //退格键键值
									{
											if(dest_count>0)
											{
													dest_count --;
											}
											src_count ++;
									}
									break;
                case    0x1B:
									{
											if(src[src_count+1]==0x5B)
											{
													if(src[src_count+2]==0x41 || src[src_count+2]==0x42)
													{
															src_count +=3;              //上移和下移键键值
													}
													else if(src[src_count+2]==0x43)
													{
															dest_count++;               //右移键键值
															src_count+=3;
													}
													else if(src[src_count+2]==0x44)
													{
															if(dest_count >0)           //左移键键值
															{
																	dest_count --;
															}
														 src_count +=3;
													}
													else
													{
															src_count +=3;
													}
											}
											else
											{
													src_count ++;
											}
									}
									break;
                default:
									{
											src_count++;
									}
									break;
           }
       }
    }
    dest[dest_count++]=src[src_count++];
    dest[dest_count++]=src[src_count++];
    return dest_count;
}

/*字符串转10/16进制数*/
static int32_t string_to_dec(uint8_t *buf,uint32_t len)
{
   uint32_t i=0;
   uint32_t base=10;       //基数
   int32_t  neg=1;         //表示正负,1=正数
   int32_t  result=0;
   
    if((buf[0]=='0')&&(buf[1]=='x'))
    {
       base=16;
       neg=1;
       i=2;
    }
    else if(buf[0]=='-')
    {
       base=10;
       neg=-1;
       i=1;
    }
    for(;i<len;i++)
    {
       if(buf[i]==0x20 || buf[i]==0x0D)    //为空格
       {
           break;
       }
       
       result *= base;
       if(isdigit(buf[i]))                 //是否为0~9
       {
           result += buf[i]-'0';
       }
       else if(isxdigit(buf[i]))           //是否为a~f或者A~F
       {
            result+=tolower(buf[i])-87;
       }
       else
       {
           result += buf[i]-'0';
       }                                        
    }
   result *= neg;
   
   return result ;
}

/*字符串截取,以空格或者结束符为界限*/
char * Mystrcpy(char *dst,const char *src)   
{
    if(src==NULL)
		{ 
       return NULL;
		}			
    char *ret = dst; //[1]
 
    //while ((*dst++=*src++)!=0x20); //[2]
		*dst=*src;
		while((*dst)!=0x20 && (*dst)!=0x0D)
    {
			dst++;
			src++;
			*dst=*src;
		}

		*dst = '\0';
		//printf("%s\r\n",ret);
		//free(dst);
    return ret;//[3]
}

/**
* 命令参数分析函数,以空格作为一个参数结束,支持输入十六进制数(如:0x15),支持输入负数(如-15)
* @param rec_buf   命令参数缓存区
* @param len       命令的最大可能长度
* @return -1:       参数个数过多,其它:参数个数
*/
int32_t cmd_arg_analyze(char *rec_buf,unsigned int len)
{
   uint32_t i;
   uint32_t blank_space_flag=0;    //空格标志
   uint32_t arg_num=0;             //参数数目
   uint32_t index[ARG_NUM];        //有效参数首个数字的数组索引
	
    /*先做一遍分析,找出参数的数目,以及参数段的首个数字所在rec_buf数组中的下标*/
    for(i=0;i<len;i++)
    {
       if(rec_buf[i]==0x20)        //为空格
       {
           blank_space_flag=1;              
           continue;
       }
        else if(rec_buf[i]==0x0D)   //换行
       {
           break;
       }
       else
       {
           if(blank_space_flag==1)
           {
                blank_space_flag=0; 
                if(arg_num < ARG_NUM)
                {
                   index[arg_num]=i;
                   arg_num++;         
                }
                else
                {
                    return -1;      //参数个数太多
                }
           }
       }
    }
   
    for(i=0;i<arg_num;i++)
    {
			  if(cmd_analyze.cmd_arg[i] == NULL)
				{
					cmd_analyze.cmd_arg[i]=(char*)malloc(sizeof(char) * 20); 
				}
				memset(cmd_analyze.cmd_arg[i],0,sizeof(char) * 20);
        //cmd_analyze.cmd_arg[i]=string_to_dec((unsigned char *)(rec_buf+index[i]),len-index[i]);
			  Mystrcpy(cmd_analyze.cmd_arg[i],rec_buf+index[i]);
			  //printf("%d:%s\n",i+1,cmd_analyze.cmd_arg[i]);
    }
    return arg_num;
}

/*打印字符串:Hello world!*/
void printf_hello(int32_t argc,char *hello_arg[])
{
   //MY_DEBUGF(CMD_LINE_DEBUG,("Hello world!\n"));
	printf("world!");
}

/*打印每个参数*/
void handle_arg(int32_t argc,char * arg_arg[])
{
   uint32_t i;
   //char  * arg[] = arg_arg;
   
    if(argc==0)
    {
       //MY_DEBUGF(CMD_LINE_DEBUG,("无参数\n"));
			printf("world!");
    }
    else
    {
       for(i=0;i<argc;i++)
       {
           //MY_DEBUGF(CMD_LINE_DEBUG,("第%d个参数:%d\n",i+1,arg[i]));
				 printf("aaa%d:%s\n",i+1,arg_arg[i]);
       }
    }
}

/*读取系统参数*/
void ReadSystemPara(int32_t argc,char * arg_arg[])
{
	printf("SetAddr %s\r\n",CONFIG_Meter.MeterNo);
	long date,time;//UTC
	GetRTC(&time,&date);
	printf("SETTIME %08ld%06ld\r\n",date,time);
  //printf("HMI_type %s\r\n",CONFIG_Meter.UIType);
	printf("MeterNoLength %d\r\n",CONFIG_Meter.MeterNoLength);
	//printf("Lock_version %s\r\n",CONFIG_Meter.Lid_Type);
	printf("UpDuty %d\r\n",CONFIG_Meter.UpDuty);
	printf("StartDuty %d\r\n",CONFIG_Meter.StartDuty); 
	printf("VeryLowPower %d\r\n",CONFIG_Meter.NotHaveDog); 
	printf("WarnBattery %.1f\r\n",CONFIG_Meter.LowBattery);
	printf("WarnPerpaid %.0f\r\n",CONFIG_Meter.LowCredit);
	printf("WarnVolume %.1f\r\n",CONFIG_Meter.LowGasVolume);
	printf("NominalCapacity %.3f\r\n",CONFIG_Meter.CYLINDER_CAPACITY);
	printf("GasRemain %.3f\r\n",REAL_DATA_PARAM.TankQuality);
	printf("USE_SENSOR %s\r\n",CONFIG_Meter.USE_SENSOR);
	printf("SetIP 1 %s PORT %s\r\n",CONFIG_GPRS.Server_IP,CONFIG_GPRS.Socket_Port);
	printf("USE_GPRS_APN %s\r\n",CONFIG_GPRS.USE_GPRS_APN);
	
	printf("SetAPN %s\r\n",CONFIG_GPRS.APN);
	printf("SetName_APN %s\r\n",CONFIG_GPRS.APN_UserName);
	printf("SetAP_N_secret %s\r\n",CONFIG_GPRS.APN_Password);
	printf("SetPin %s\r\n",CONFIG_GPRS.Pin);
	printf("ResetSum %d\r\n",CONFIG_Meter.ResetSum);
	printf("ResetTime %s\r\n",CONFIG_Meter.ResetTime);
	printf("FIRMWARE_V %s%s\r\n",FIRMWARE_Version,CONFIG_Meter.FIRMWARE_V);
	printf("BatVoltage %.3f\r\n",REAL_DATA_PARAM.BatVoltage);
}

/*设置ID*/
void SetAddr(int32_t argc,char * arg_arg[])
{
	strcpy(CONFIG_Meter.MeterNo,arg_arg[0]);
  IsSaveCONFIG_Meter = true;
	printf("OKOKOK\r\n");  
}

/*设置时间*/
void DTISETTIME(int32_t argc,char * arg_arg[])
{
	long yymmdd;
	long hhmmss;
	
	sscanf(arg_arg[0] + 2,"%6ld",&yymmdd);
	//printf("年月日 : %ld\r\n",yymmdd);
	sscanf(arg_arg[0]+8,"%6ld",&hhmmss);
	//printf("时分秒 : %ld\r\n",hhmmss);
	SetRTC(hhmmss,yymmdd);

	printf("OKOKOK\r\n");  
}

/*设置上传周期*/
void UpDuty(int32_t argc,char * arg_arg[])
{
	uint32_t * Up = &CONFIG_Meter.UpDuty; 
	sscanf(arg_arg[0],"%u",Up);
  IsSaveCONFIG_Meter = true;
	
	printf("OKOKOK\r\n");  
}

/*设置心跳周期*/
void StartDuty(int32_t argc,char * arg_arg[])
{
	uint32_t * Up = &CONFIG_Meter.StartDuty; 
	sscanf(arg_arg[0],"%u",Up);

	IsSaveCONFIG_Meter = true;
	printf("OKOKOK\r\n");  
}

/*设置使用的人机屏幕类型*/
void HMI_type(int32_t argc,char * arg_arg[])
{
	//strcpy(CONFIG_Meter.UIType,arg_arg[0]);
	//strcpy(CONFIG_Meter.Lid_Type,arg_arg[0]);

	printf("OKOKOK\r\n");  
}

///*设置使用的盖锁类型*/
//void Lock_version(int32_t argc,char * arg_arg[])
//{
//	strcpy(CONFIG_Meter.Lid_Type,arg_arg[0]);

//	printf("OKOKOK\r\n");  
//}

/*设置低电量报警*/
void WarnBattery(int32_t argc,char * arg_arg[])
{
	float * Up = &CONFIG_Meter.LowBattery;
	sscanf(arg_arg[0],"%f",Up);
  IsSaveCONFIG_Meter = true;
	printf("OKOKOK\r\n");  
}

/*设置预付费低报警*/
void WarnPerpaid(int32_t argc,char * arg_arg[])
{
	float * Up = &CONFIG_Meter.LowCredit;
	sscanf(arg_arg[0],"%f",Up);
  IsSaveCONFIG_Meter = true;
	printf("OKOKOK\r\n");  
}

/*设置低气量报警*/
void WarnVolume(int32_t argc,char * arg_arg[])
{
	float * Up = &CONFIG_Meter.LowGasVolume;
	
	sscanf(arg_arg[0],"%f",Up);
	IsSaveCONFIG_Meter = true;
	printf("OKOKOK\r\n");  
}

/*设置额定容量*/
void NominalCapacity(int32_t argc,char * arg_arg[])
{
	float * Up = &CONFIG_Meter.CYLINDER_CAPACITY;
	sscanf(arg_arg[0],"%f",Up);
	IsSaveCONFIG_Meter = true;

	printf("OKOKOK\r\n");  
}

/*设置当前剩余*/
void GasRemain(int32_t argc,char * arg_arg[])
{
	float * Up = &REAL_DATA_PARAM.TankQuality;
	sscanf(arg_arg[0],"%f",Up);
	IsSaveREAL_DATA_PARAM = true;

	printf("OKOKOK\r\n");  
}

/*设置IP端口*/
void SetIP(int32_t argc,char * arg_arg[])
{
	strcpy(CONFIG_GPRS.Server_IP,arg_arg[1]);
	strcpy(CONFIG_GPRS.Socket_Port,arg_arg[3]);
	//MB85RS16A_WRITE(2,(uint8_t*)(&CONFIG_GPRS),sizeof(CONFIG_GPRS));
  IsSaveCONFIG_GPRS = true;
	printf("OKOKOK\r\n");  
}

/*设置是否使用APN,其实都需要使用,只不过有些事默认的*/
void USE_GPRS_APN(int32_t argc,char * arg_arg[])
{
	strcpy(CONFIG_GPRS.USE_GPRS_APN,arg_arg[0]);
	//MB85RS16A_WRITE(2,(uint8_t*)(&CONFIG_GPRS),sizeof(CONFIG_GPRS));
	IsSaveCONFIG_GPRS = true;
	printf("OKOKOK\r\n");  
}

/*设置仪表使用的传感器类型*/
void USE_SENSOR(int32_t argc,char * arg_arg[])
{
	strcpy(CONFIG_Meter.USE_SENSOR,arg_arg[0]);
	//MB85RS16A_WRITE(2,(uint8_t*)(&CONFIG_GPRS),sizeof(CONFIG_GPRS));
	IsSaveCONFIG_Meter = true;
	printf("OKOKOK\r\n");  
}

/*设置是否使用看门狗*/
void VeryLowPower(int32_t argc,char * arg_arg[])
{
	CONFIG_Meter.IsNormalReset = true;
	IsNeedRestart = true;
	CONFIG_Meter.NotHaveDog = (*arg_arg[0] == '1' ? true : false);
	IsSaveCONFIG_Meter = true;
	//printf("OKOKOK\r\n");  
	printf("VeryLowPower %d\r\n",CONFIG_Meter.NotHaveDog); 
}

/*读取历史电压*/
void ReadHisVoltage(int32_t argc,char * arg_arg[])
{
	//printf("OKOKOK\r\n");  
	sscanf(arg_arg[0],"%d",&VoltageStart);
	sscanf(arg_arg[1],"%d",&VoltageEnd);
	IsNeedReportVoltage = true;
}

/*设置APN*/
void SetAPN(int32_t argc,char * arg_arg[])
{
	strcpy(CONFIG_GPRS.APN,arg_arg[0]);
	//MB85RS16A_WRITE(2,(uint8_t*)(&CONFIG_GPRS),sizeof(CONFIG_GPRS));
	IsSaveCONFIG_GPRS = true;
	printf("OKOKOK\r\n");  
}

/*导出日志*/
void ExportLog(int32_t argc,char * arg_arg[])
{
	sscanf(arg_arg[0],"%d",&logStart);
	sscanf(arg_arg[1],"%d",&logEnd);
	IsNeedReportLog = true;
}

/*导出日志索引*/
void FindLogIndex(int32_t argc,char * arg_arg[])
{
	printf("FindLogIndex %d %d\r\n",Current_Log.indexSendNumber,Current_Log.indexCurrent);
}

/*设置APN用户*/
void SetName_APN(int32_t argc,char * arg_arg[])
{	
	strcpy(CONFIG_GPRS.APN_UserName,arg_arg[0]);
	//MB85RS16A_WRITE(2,(uint8_t*)(&CONFIG_GPRS),sizeof(CONFIG_GPRS));
	IsSaveCONFIG_GPRS = true;
	printf("OKOKOK\r\n");  
}

/*设置APN密码*/
void SetAP_N_secret(int32_t argc,char * arg_arg[])
{
	strcpy(CONFIG_GPRS.APN_Password,arg_arg[0]);
	//MB85RS16A_WRITE(2,(uint8_t*)(&CONFIG_GPRS),sizeof(CONFIG_GPRS));
	IsSaveCONFIG_GPRS = true;
	printf("OKOKOK\r\n");  
}
/*设置Pin密码*/
void SetPin(int32_t argc,char * arg_arg[])
{
	strcpy(CONFIG_GPRS.Pin,arg_arg[0]);
	//MB85RS16A_WRITE(2,(uint8_t*)(&CONFIG_GPRS),sizeof(CONFIG_GPRS));
	IsSaveCONFIG_GPRS = true;
	printf("OKOKOK\r\n");  
}

/*设置为电压校准模式*/
void readVoltage(int32_t argc,char * arg_arg[])
{
	//IsReadVoltage = true;
	//strcpy(CONFIG_GPRS.APN_Password,arg_arg[0]);
	//MB85RS16A_WRITE(2,(uint8_t*)(&CONFIG_GPRS),sizeof(CONFIG_GPRS));
	//IsSaveCONFIG_GPRS = true;
	printf("OKOKOK\r\n");  
}

/*设置流量参数*/
void writeVoltage(int32_t argc,char * arg_arg[])
{

	//IsSaveCONFIG_GPRS = true;
	uint8_t i = 0; 
	sscanf(arg_arg[0],"%hhu",&REAL_Flow_CALIBRATION.ParamNumber);
	//printf("REAL_Flow_CALIBRATION.ParamNumber:%d\r\n",REAL_Flow_CALIBRATION.ParamNumber); 	
	
	for(;i < REAL_Flow_CALIBRATION.ParamNumber;i++)
	{
		sscanf(arg_arg[i + 1],"%f",&REAL_Flow_CALIBRATION.ABCDEF[i]);
		//printf("REAL_Flow_CALIBRATION.ABCDEF[%d]:%lf\r\n",i,REAL_Flow_CALIBRATION.ABCDEF[i]);
	}
	
	IsSaveFlowCALIBRATION = true;
	
	printf("OKOKOK\r\n");  
}

/*设置流量 由于连接上串口之后CPU测试的AD值会有偏差,故这个功能改用刷卡的方式,或者直接用标准表,但不能用电脑的串口*/
void RealTimeFlow(int32_t argc,char * arg_arg[])
{
	float * Up = &y2;
	sscanf(arg_arg[0],"%f",Up);

	//printf("y1 : %.3f\r\n",y1);
	//printf("y2 : %.3f\r\n",y2); 
	//printf("volt1 : %.3f\r\n",volt1);
	//printf("vert2 : %.3f\r\n",vert2); 
	
	if(y2 != 0 && vert2 != 0 && volt1 != 0 && vert2 > volt1)
	{
		REAL_DATA_CALIBRATION.slope = (y2 - y1)/(vert2 - volt1);
		REAL_DATA_CALIBRATION.zero =y1 - REAL_DATA_CALIBRATION.slope * volt1;
		
		//printf("slope : %.3f\r\n",REAL_DATA_CALIBRATION.slope);
	  //printf("zero : %.3f\r\n",REAL_DATA_CALIBRATION.zero);
		
		IsSaveCALIBRATION = true;
	}

	printf("OKOKOK\r\n");  
}

/*清除数据*/
void ClearZero(int32_t argc,char * arg_arg[])
{
	char *str1 = strstr(CONFIG_Meter.MeterNo,"KE");
	if(str1!=NULL)
	{
		REAL_DATA_PARAM.TankQuality = 13;
		REAL_DATA_Credit.UnitPrice = 233.00;
	}
	else
	{
		REAL_DATA_PARAM.TankQuality = 15;
		REAL_DATA_Credit.UnitPrice = 4000.00;
	}
	
	Current_Credit.CurrCredit = 0;
	REAL_DATA_Credit.CumulationCredit = 0;
	REAL_DATA_Credit.CumulationGasL = 0;
	REAL_DATA_Credit.UsedKg = 0;
	
	
	REAL_DATA_Credit.LPGDensity = 2.525;

	//REAL_DATA_Credit.CardTotal = 0;
	
	CONFIG_Meter.ResetSum = 0;
	IsSaveCONFIG_Meter = true;
	
	REAL_DATA_Credit.CookingSessionSendNumber = 0;
	REAL_DATA_Credit.CookingSessionEnd = 0;
  
	IsSaveREAL_DATA_PARAM = true;
	IsSaveREAL_DATA_Credit = true;
	
  //HAL_Delay(100);
	
	printf("OKOKOK\r\n");  
}

/*恢复出厂设置*/
void RESETALL(int32_t argc,char * arg_arg[])
{
	char *str1;
	//strcpy(CONFIG_GPRS.Server_IP,"35.177.78.198");//47.95.200.195 35.178.108.252
	strcpy(CONFIG_GPRS.Socket_Port,"5057");
	strcpy(CONFIG_GPRS.USE_GPRS_APN,"1");
	str1 = strstr(CONFIG_Meter.MeterNo,"KE");
	if(str1!=NULL)
	{
		strcpy(CONFIG_GPRS.APN,"vpn.mgas.com");
		strcpy(CONFIG_Meter.CURRENCY,"KSh");
	  CONFIG_Meter.CYLINDER_CAPACITY = 13;
		REAL_DATA_PARAM.TankQuality = 13;
		REAL_DATA_Credit.UnitPrice = 233.00;
	}
	else
	{
		strcpy(CONFIG_GPRS.APN,"kopagas");
		strcpy(CONFIG_Meter.CURRENCY,"TZS");
	  CONFIG_Meter.CYLINDER_CAPACITY = 15;
		REAL_DATA_PARAM.TankQuality = 15;
		REAL_DATA_Credit.UnitPrice = 4000.00;
	}
	memset(CONFIG_GPRS.APN_UserName,0,sizeof(CONFIG_GPRS.APN_UserName));
	memset(CONFIG_GPRS.APN_Password,0,sizeof(CONFIG_GPRS.APN_Password));
	memset(CONFIG_GPRS.CenterSimNumber,0,sizeof(CONFIG_GPRS.CenterSimNumber));
	memset(CONFIG_GPRS.ServerSimNumber,0,sizeof(CONFIG_GPRS.ServerSimNumber));
	strcpy(CONFIG_GPRS.Communications_Type,"1");
	
	//strcpy(CONFIG_Meter.MeterNo,"TZ00000001");
	CONFIG_Meter.MeterNoLength = 10;
	
	//strcpy(CONFIG_Meter.BATTERY_MODEL,"TX123");
//	strcpy(CONFIG_Meter.UIType,"2");
//	strcpy(CONFIG_Meter.Lid_Type,"2");
	CONFIG_Meter.UpDuty = 360;
	CONFIG_Meter.StartDuty = 180;
	CONFIG_Meter.LowBattery = 3.7;
	CONFIG_Meter.LowCredit = 200;
	CONFIG_Meter.LowGasVolume = 1;
	
	CONFIG_Meter.IsHaveRFID = true;
	CONFIG_Meter.NotHaveDog = false;
	
	strcpy(CONFIG_Meter.Longitude,"0000000000");
	strcpy(CONFIG_Meter.Latitude,"0000000000");
	
	strcpy(REAL_DATA_PARAM.LidSensorStatus,Opening_input == 1 ? "1" : "0");

	strcpy(REAL_DATA_PARAM.NEEDLESensorStatus,NEEDLE_input == GPIO_PIN_SET ? "0" : "1");

	strcpy(REAL_DATA_PARAM.TankSensorStatus,"1");//无用
	
	if(strcmp(REAL_DATA_PARAM.LidElectricLock, "1")==0)
	{
		 strcpy(REAL_DATA_PARAM.LidElectricLock,"0");
		 glockStatus = motor_open;
	}
	if(strcmp(REAL_DATA_PARAM.ElectricValveStatus, "1")==0)
	{
		 strcpy(REAL_DATA_PARAM.ElectricValveStatus,"0");
		 gmotorStatus = motor_close;
	}
	if(strcmp(REAL_DATA_PARAM.TankLockStatus, "0")==0)
	{
		 strcpy(REAL_DATA_PARAM.TankLockStatus,"1");
		 gassembleStatus = motor_close;
	}
	
	Current_Credit.CurrCredit = 0;
	REAL_DATA_Credit.CumulationCredit = 0;
	REAL_DATA_Credit.CumulationGasL = 0;
	REAL_DATA_Credit.UsedKg = 0;
	
	REAL_DATA_Credit.LPGDensity = 2.525;
	
	Current_Log.indexSendNumber = 0;
	Current_Log.indexCurrent = 0;
	Current_Log.addrBigData = 0x500000;
  
  //HAL_Delay(100);
	
	REAL_DATA_CALIBRATION.slope = 1;
	REAL_DATA_CALIBRATION.zero = 0;
  IsSaveCALIBRATION = true;
	
	REAL_DATA_Credit.Magnification = 1;
	IsSaveREAL_DATA_Credit = true;
	
	IsSaveCONFIG_GPRS = true;
  IsSaveCONFIG_Meter = true;
  IsSaveREAL_DATA_PARAM = true;
  IsSaveREAL_DATA_Credit = true;
	IsSaveREAL_Current_Log = true;
	if(argc == 0)//如果是串口操作
	{
		printf("OKOKOK\r\n");
	}	
	else
	{
		//如果是复位卡才能清除卡信息
		REAL_DATA_Credit.CardTotal = 0;
    //REAL_DATA_Credit.CookingSessionSendNumber = 0;
	  //REAL_DATA_Credit.CookingSessionEnd = 0;
	}	
}

/*重启动*/
void ResetSystem(int32_t argc,char * arg_arg[])
{
	CONFIG_Meter.IsNormalReset = true;
	IsSaveCONFIG_Meter = true;
	IsNeedRestart = true;
}

/*设置中*/
void SetParaming(int32_t argc,char * arg_arg[])
{
	  //printf("LCD_Delay_Time %d\r\n",LCD_Delay_Time);
		LCD_Delay_Time = 10 * 100;
}

