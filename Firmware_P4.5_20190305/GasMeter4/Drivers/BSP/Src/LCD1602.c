#include "LCD1602.H"
#include "cmsis_os.h"
#include "main.h"
#include "rtc.h"
#include "Sim80x.h"
#include "key.h"

extern REAL_DATA_Credit_t REAL_DATA_Credit; //仪表充值减值信息
extern REAL_DATA_PARAM_t REAL_DATA_PARAM;   //信号强度等

extern uint8_t commandType;

//uint8_t CGRAM_data[] = {
//                        0x80,0x1f,0x02,0x0f,0x0a,0x1f,0x02,0x02,//年
//                        0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,//偶竖
//                        0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,//奇竖
//                        0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,//奇横
//                        0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,//偶横
//                        0xff,0x11,0x11,0x11,0x11,0x11,0x11,0xff,//方框
//                        0xff,0x11,0x11,0x11,0x11,0x11,0x11,0xff,//方框
//                        0xff,0x11,0x11,0x11,0x11,0x11,0x11,0xff,//方框
//                        };
//char *Name1[6] = {"GASC","GR","CR","BL","VUse","VLeft"}; //用气质量、剩余气量、剩余金额、电池电量,用气体积，剩余体积
                 
/*
char *Name[23] = 
{
 "GASC","REMC","BATL","REMP","CUMC","REMV",//用气量、剩余金额、电池电量、液化气罐剩余量、累计用气量、剩余气量
 "UNIP","DENS","CAPA","FREQ","JXZT","BJXX","FFKG"，//"单价"，"密度"，"液化气罐容量，"上传周期"，"机械状态"，"报警信息"，"非法开盖报警"
 "PURC","DANJ","MIDU","RONL","FERQ","BAJL","BIZH","KAIG","TIME","NAME", //"充值金额"，"单价","密度","液化气罐容量","上传周期","报警量","币种","开盖","时间","液晶显示名字",
}; 
*/
//char *Unit[6] = {"g","g","T","%","L","L"};
//char *Unit1[6] = {"Kg","Kg","T","%","M3","M3"};
//uint32_t Parameter[23];//变量
//uint32_t ParSet[15] ={0,2500,2072,15,1,1,1,1,1,1,1212,1159,5,45} ;//设定变量

uint32_t ErrorMessage[10];//故障信息
extern uint8_t connectStep;
extern uint8_t connectStepStatus;

void Delay(uint16_t time)
{
  uint8_t i,j;
  for(i=time;i>0;i--)
  for(j=50;j>0;j--);
}
/******************************************************************************/
//LCD port initial;
//2017.1.24.
void LcdPortini(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
//	__HAL_RCC_GPIOC_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_9;                         
//  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
//	GPIO_InitStruct.Pull = GPIO_PULLUP; 
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
//  GPIO_Init(GPIOD,GPIO_Pin_7, GPIO_Mode_Out_PP_Low_Slow);//LCD_SID 配置为输出模式
//  GPIO_Init(GPIOD,GPIO_Pin_6, GPIO_Mode_Out_PP_Low_Slow);//LCD_SCLK 配置为输出模式
//  GPIO_Init(GPIOD,GPIO_Pin_5, GPIO_Mode_Out_PP_High_Slow);//LCD_CS 配置为输出模式
//  GPIO_Init(GPIOD,GPIO_Pin_4, GPIO_Mode_Out_PP_Low_Slow);//LCD_RS 配置为输出模式
}
/******************************************************************************/
//LCD port deinitial;
//2017.1.24.
void LcdPortDeini(void)
{
  	GPIO_InitTypeDef GPIO_InitStruct;
	  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;                         
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	  __HAL_RCC_GPIOC_CLK_DISABLE();
}
/******************************************************************************
写数据或指令到LCD
Date：2016.4.19
*******************************************************************************/
void WriteLCD(uint8_t command,uint8_t data)
{
  uint8_t i;
  //LcdPortini();
  ReSetLCD_CS;//CS = 0
  if(SET == command)
	{
		ReSetLCD_RS;//RS = 0,写命令
	}
  else if(RESET == command)
	{
		SetLCD_RS;//RS = 1,写数据
	}
  for(i=8;i>0;i--)
  {
    ReSetLCD_SCLK;//SCLK = 0;
    Delay(1);
    if(data & 0x80)
		{
			SetLCD_SID;//SID = 1;
		}
    else 
		{
			ReSetLCD_SID;//SID = 0;
		}
    Delay(1);
    SetLCD_SCLK;//SCLK = 1;
    Delay(1);
    data<<=1;
  }
  //LcdPortDeini();
}
/*******************************************************************************
液晶1602初始化
Date：2016.4.19
*******************************************************************************/
void LCD_INI()
{
	LcdPortini();
  ReSetLCD_CS;//CS = 0  
  Delay(50);
  WriteLCD(SET,MODE);//写命令0x38，功能选择
  WriteLCD(SET,CLR);//写命令0x01,清屏
  //Delay(100);
  WriteLCD(SET,CursorDirectionRight);//写命令0x06，光标移动设置 
  WriteLCD(SET,OPENCream);//写命令0x0c,开屏幕显示
  SetLCD_CS;//CS = 1
}
/*******************************************************************************
自编字符并存储起来，存至字库列表的0x00到0x07单元
Date：2016.4.19
*******************************************************************************/
//void WriteCGRAM()
//{
//  uint8_t i;
//  WriteLCD(SET,0X40);//写命令0x40,设置CGRAM 地址：低几个CGRAM,0X40表示第0个。
//  for(i=0;i<64;i++)
//  {
//   WriteLCD(RESET,CGRAM_data[i]);
//   // WriteLCD(RESET,Name[i]);
//  }
//}
/*******************************************************************************
显示自编的字符
data：2016.4.19
*******************************************************************************/
//void DisplayCGRAM()
//{
//  uint8_t i,j;
//  for(j=0;j<6;j++)
//  {
//    WriteLCD(SET,0X80);//写命令0x80,设置DDRAM 地址：低几个行,第几列。
//    for(i=0;i<16;i++)
//      {
//         WriteLCD(RESET,j);//写数据
//      }
//    WriteLCD(SET,0Xc0);//写命令0xc0,设置DDRAM 地址：低几个行,第几列。
//    for(i=0;i<16;i++)
//    {
//       WriteLCD(RESET,j);//写数据
//    }
//  }
//}
/*******************************************************************************
在指定行和列位置显示指定的字母、数字（5*7点阵）
Date：2016.4.19
*******************************************************************************/
void DisplayChar(uint8_t line,uint8_t column,char *dp)
{
	//taskENTER_CRITICAL();
  uint8_t i;
  WriteLCD(SET,0x80+(line-1)*0x40+(column-1));//设置DDRAM 地址：行和列

  for(i=0;i<16;i++)
   {
      if(*dp!='\0')
       {
          WriteLCD(RESET,*dp);//写数据
          dp = dp+1;//指想下一个元素
       }
       else break;
   }
	 //taskEXIT_CRITICAL();
}
//将整数转换成字符串数字
uint8_t *PParChange(uint32_t Parameter)
{
  uint8_t i;
  uint8_t *PArrey;
  uint32_t temp;
  static uint8_t Arrey[11]={0};
  PArrey = Arrey;//指针指向数组
  temp = Parameter;
  for(i=0;i<9;i++)
  {
    Arrey[8-i] = temp%10;//余数
    temp /= 10;//商 
  }
  return (PArrey);
}
/*******************************************************************************
在指定行和列位置显示指定的变量
*******************************************************************************/
void DisplayDatax(uint8_t line,uint8_t column,uint32_t data)
{
  uint8_t i,wei=0;
  uint8_t *P;
  P = PParChange(data);
  WriteLCD(SET,0x80+(line-1)*0x40+(column-1));//设置DDRAM 地址：行和列
  //------------------------------去掉前位0-------------------------------------
 #if 1
	for(i=0;i<8;i++)
  {
    if(*P==0)
    {
      WriteLCD(SET,RIGHT);
      P++;
      wei++;
    }
    else 
		{
			break;
		}
  }
//------------------------------------------------------------------------------
	//从有数据的位置开始写
  for(i=wei;i<9;i++)
  {
  /*
    if(0 != *P && i == 4) 
    {
      if(wei==4)//显示0.1前0
       {
          WriteLCD(SET,LEFT);
          WriteLCD(RESET,0X30);//写0
        }
      WriteLCD(RESET,'.');//写小数点
      WriteLCD(RESET,*P+0X30);//写数据 
    }
    else if(0 == *P && i == 5) *P = 0X20;//写空格0X20
    else *P += 0X30;
    */
    WriteLCD(RESET,*P+0x30);//写数据
    P++;
  }
  if(wei==6)
  {
    WriteLCD(SET,LEFT);
    WriteLCD(SET,LEFT);
    WriteLCD(RESET,0X30);//写0
  }
	#endif
}
/*******************************************************************************
菜单显示
Date：2016.4.20
*******************************************************************************/
void DisplayData2x(uint8_t line,uint8_t column,uint8_t data)
{
	  uint8_t i;
    WriteLCD(SET,0x80+(line-1)*0x40+(column-1));//设置DDRAM 地址：行和列
		i= data/10;
    WriteLCD(RESET,i+0x30);//写数据
	  i= data%10;
    WriteLCD(RESET,i+0x30);//写数据
}
/******************************************************************************/
//时间测试
extern uint32_t Date;//日期
extern uint32_t Time;//时间
//extern uint8_t LCDOffCount;

void TimeDisplay(void)
{
	RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
  //LCD_INI();
  DisplayChar(1,2,"Date:");
  DisplayData2x(1,8,sDate.Year);//第一行显示日期
	DisplayChar(1,10,"-");
	DisplayData2x(1,11,sDate.Month);//第一行显示日期
	DisplayChar(1,13,"-");
	DisplayData2x(1,14,sDate.Date);//第一行显示日期
  DisplayChar(2,2,"Time:");
  DisplayData2x(2,8,sTime.Hours);//第二行显示时间
	DisplayChar(2,10,":");
	DisplayData2x(2,11,sTime.Minutes);//第二行显示时间
	DisplayChar(2,13,":");
	DisplayData2x(2,14,sTime.Seconds);//第二行显示时间
}

/******************************************************************************/
//Display RTC time
//2018.11.28
void DisplayTime(uint8_t line)
{
  char str_time[16] = {"                "};//The contents of  LCD first line 
	
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sdate; 
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
	
  str_time[4]  = sTime.Hours/10 + 0x30;
  str_time[5]  = sTime.Hours%10 + 0x30;
  str_time[6]  = ':';
  str_time[7]  = sTime.Minutes/10 + 0x30;
  str_time[8]  = sTime.Minutes%10 + 0x30;
  str_time[9]  = ':';
  str_time[10]  = sTime.Seconds/10 + 0x30;
  str_time[11]  = sTime.Seconds%10 + 0x30;
  //________________Display cleander____________________________________________
  DisplayChar(line,1,str_time);//Show date in defined line
	
	//printf("time:%s\r\n",str_time);
}

/******************************************************************************/
//Display RTC Date
//2018.11.28
void DisplayDate(uint8_t line)
{
  char str_time[16] = {"                "};//The contents of  LCD first line 
	
  RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate; 
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	
  str_time[4]  = sDate.Year/10 + 0x30;
  str_time[5]  = sDate.Year%10 + 0x30;
  str_time[6]  = '-';
  str_time[7]  = sDate.Month/10 + 0x30;
  str_time[8]  = sDate.Month%10 + 0x30;
  str_time[9]  = '-';
  str_time[10]  = sDate.Date/10 + 0x30;
  str_time[11]  = sDate.Date%10 + 0x30;
  //________________Display cleander____________________________________________
  DisplayChar(line,1,str_time);//Show date in defined line
}
/******************************************************************************/
/*
读按键
Date：2016.4.19
*/
//void ReadKey()
//{
//   if(RESET == ReadKEY1) SET_BEEP;
//   if(RESET == ReadKEY2)SET_BEEP;
// // else if(RESET == ReadKEY3)SET_BEEP;
//  else if(RESET == ReadKEY4)RESET_BEEP;
// // else if(RESET == ReadKEY5)SET_BEEP;
//}

/******************************************************************************/
//Get display parameters
//2016.12.29
void DisplayParameter(uint8_t PageTemp,float data)
{
  uint32_t data_len = 0;
  uint8_t i;
  char  temp[11];
  char str_parameter[20][16] = 
  {
    //______Customer parameter leader define____________________
    {"Muse:         kg"},
    {" GR :         kg"},
    //{"$use:          T"},
    {" CR :          T"},
    {" BL :          %"},
    {" VU :          L"},
		{" NO.:           "},
    //{"Vlft:          L"},
    /*
    {"Muse:         kg"},
    {"Mlft:         kg"},
    {"$use:          T"},
    {"$lft:          T"},
    {"Btry:          %"},
    {"Vuse:          L"},
    {"Vlft:          L"},
    */
    //______Administrator parameter leader define_______________
    {"$in :          T"},
    {"$unt:       T/kg"},
    {"Dsty:      kg/m3"},
    {"Mtnk:         kg"},
    {"Duty:           "},
    {"Balm:          %"},
    {"$alm:          %"},
    {"Valm:          %"},
    {"$knd:           "},
    {"Cver:           "},
    {"Date:           "},
    {"Time:           "},
    {"Chck:           "},
  };

	
	if(PageTemp == 1)
	{
		//printf("data:%f\r\n",data);
		data_len = sprintf(temp,"%3.3f",data);
		for(i=data_len;i>0;i--)//Get the contents of data,which depend on page.
		{
			str_parameter[PageTemp][(16-3) - i] = temp[data_len-i];
		}
	}
	
	if(PageTemp == 2)
	{
		//printf("data:%f\r\n",data);
		data_len = sprintf(temp,"%7.2f",data);
		for(i=data_len;i>0;i--)//Get the contents of data,which depend on page.
		{
			str_parameter[PageTemp][(16-2) - i] = temp[data_len-i];
		}
	}
	
	if(PageTemp == 3)
	{
		//printf("data:%f\r\n",data);
		data = (data - 3.6)/ (0.6) * 100;
		if(data> 100)
		{
			data = 100;
		}
		if(data < 0)
		{
			data = 0;
		}
		data_len = sprintf(temp,"%5.0f",data);
		for(i=data_len;i>0;i--)//Get the contents of data,which depend on page.
		{
			str_parameter[PageTemp][(16-2) - i] = temp[data_len-i];
		}
	}
	
	if(PageTemp == 4)
	{
		data_len = sprintf(temp,"%7.1f",data);
		for(i=data_len;i>0;i--)//Get the contents of data,which depend on page.
		{
			str_parameter[PageTemp][(16-2) - i] = temp[data_len-i];
		}
	}
	
  DisplayChar(2,1,str_parameter[PageTemp]);//Show date in defined line
}

void ShowGprs(void)
{
	uint8_t i;
	char emptyString[17] = {"                "};
//  if(Sim80x.Status.Power==0)
//	{
//	   DisplayChar(1,1,"-GPRS Power ing-");
//	}
//	else
	{
		//printf("connectStep:%d\r\n",connectStep);
		//printf("connectStepStatus:%d\r\n",connectStepStatus);
	  switch(connectStep)
		{
		  case 0:
				//DisplayChar(1,1,"--GSM Powering--");
			  DisplayChar(1,1,"--GSM Power ON--");
			  //WriteLCD(SET,Cursor);
				break;
			case 1:
				//DisplayChar(1,1,"--GSM Power ON--");
			  //WriteLCD(SET,Cursor);
				break;
			case 2:
				if(connectStepStatus == 0)
				{
				   DisplayChar(1,1,"Looking SIM Card");
					 //WriteLCD(SET,Cursor);
				}
				else if(connectStepStatus == 1)
				{
				   DisplayChar(1,1,"--SIM Card  OK--");
					 osDelay(1000);
					 //WriteLCD(SET,Cursor);
				}
				else
				{
				   DisplayChar(1,1,"-SIM Card error-");
					 //WriteLCD(SET,Cursor);
				}
				break;
			case 3:
				if(connectStepStatus == 0)
				{
				   DisplayChar(1,1," Searching  net ");
					 //WriteLCD(SET,CursorBlink);
				}
				else if(connectStepStatus == 1)
				{
				   DisplayChar(1,1,"-----NET OK-----");
					 osDelay(1000);
					 //WriteLCD(SET,Cursor); 
					 connectStepStatus = 0;
				}
				else
				{
				   DisplayChar(1,1,"---NET  error---");
					 //WriteLCD(SET,Cursor);
				}
				break;
			case 4:
				if(connectStepStatus == 0)
				{
				   DisplayChar(1,1," Registering net");
					 //WriteLCD(SET,CursorBlink);
				}
				else if(connectStepStatus == 1)
				{
				   DisplayChar(1,1,"  Register OK  ");
					 osDelay(1000);
					 //WriteLCD(SET,Cursor);
					 connectStepStatus = 0;
				}
//				else
//				{
//				   DisplayChar(1,1,"-Register error-");
//				}
				break;
			case 5:
				DisplayChar(1,1," Landing server ");
			  //WriteLCD(SET,CursorBlink);
				break;
			case 6:
//				for(i=16;i>0;i--)//Get the contents of data,which depend on page.
//				{
//					emptyString[16 - i] = Sim80x.GPRS.LocalIP[16 - i];
//				}
//				DisplayChar(1,2,emptyString);
				break;
			case 7:
				if(commandType == 1)
				{
				
				}
				else if(commandType == 2)
				{
					DisplayChar(1,1," Charge money OK");
					osDelay(3000);
				}
				else if(commandType == 3)
				{
					DisplayChar(1,1,"   Control OK   ");
					osDelay(3000);
				}
				else
				{					
				   DisplayChar(1,1,"----GPRS  OK----");
			  //WriteLCD(SET,Cursor);
				}
				break;
		}
	}
}

void Lcd1602_show(uint8_t * sStep)
{
	static uint8_t LCD_Refresh_Time  = 0;	
	
	if(*sStep == 1)
	{
		LCD_POWER(1);
		LCD_INI();//LCD initial.
		*sStep = 2;
	}
	
	if(*sStep == 6)
	{
			*sStep = 2;
	}
	
	if(LCD_Refresh_Time < 3)
	{
		LCD_Refresh_Time++;
	}
	else
	{	
			if(Key2_input == GPIO_PIN_SET && strcmp(REAL_DATA_PARAM.Electric_Meter.LidElectricLock,"1") == 0)
			{
				DisplayChar(1,1,"--Cover Opened--");
			}
			else
			{
				if(GSM_ON_FLAG == 0)
				{
					DisplayTime(1);
				}
				else
				{
					ShowGprs();
				}
			}
			
			if(*sStep == 2)
			{	
				DisplayParameter(1,REAL_DATA_PARAM.TankQuality);
			}

			if(*sStep == 3)
			{
				DisplayParameter(2,REAL_DATA_Credit.RemainedCredit);
			}
			
			if(*sStep == 4)
			{
				DisplayParameter(3,REAL_DATA_PARAM.BatVoltage);
			}
			
			if(*sStep == 5)
			{
				DisplayParameter(4,REAL_DATA_Credit.CumulationGasL);
			}
	}
}
