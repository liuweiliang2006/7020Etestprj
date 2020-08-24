#include "lcd.h"
#include "key.h"
#include "Sim80x.h"

#define uchar unsigned char

static unsigned char  NUM_Addr[49] = {_1A,_1B,_1C,_1D,_1E,_1F,_1G, //第一个数字的七段数码管的内存地址
																			_2A,_2B,_2C,_2D,_2E,_2F,_2G,
																			_3A,_3B,_3C,_3D,_3E,_3F,_3G,
	                                    _4A,_4B,_4C,_4D,_4E,_4F,_4G,
																			_5A,_5B,_5C,_5D,_5E,_5F,_5G,
	                                    _6A,_6B,_6C,_6D,_6E,_6F,_6G,
																			_7A,_7B,_7C,_7D,_7E,_7F,_7G};

LCDPanel_TypeDef LCDPanelStruct;

static LCDPanel_TypeDef LCDPanel_Tmp;

extern uint16_t connectStep;

extern Sim80x_t      Sim80x;
extern REAL_DATA_PARAM_t REAL_DATA_PARAM;   //信号强度等
//extern uint32_t zeroFlowTime;
extern float TmpRealTimeFlow; 

bool IsTestLcd = false;  //是否测试过了LCD屏幕,全亮一次

/**
  * @brief  设置显示数字符号，内部调用
  * @param  Addr：符号的显存地址
  * @param  Num：要显示的数字
  * @retval None
  */
static void SetNum(int Num)
{
  //unsigned char *Addr_t = Addr;
	bool IsContinue = true;
	unsigned char i;
	unsigned char static num[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};//0-9位段编码
	unsigned char Hun_Thousand_number;//十万 1
	unsigned char The_Thousand_number;//万 2
	unsigned char one_Thousand_number;//千 3
	unsigned char Hundred_number;//百位 4
	unsigned char Decate_number;//十位 5
	unsigned char Unit_number;//个位 6
	unsigned char Dot_number;//小数点 7
	
	if(Num < 0)
	{
	  Hun_Thousand_number = 0x40;
		Num = Num * -1;
	}
	else
	{
		Hun_Thousand_number = num[Num/1000000%10];
	}
	
	if(LCDPanelStruct.POINT1_Sign == ON)//|| Num/1000000%10 == 0
	{
		 Hun_Thousand_number = 0x00;
	}
	
//  if(IsContinue == true && Num/100000%10 == 0)
//	{
//		The_Thousand_number = 0x00;
//	}
//	else
	{
		The_Thousand_number = num[Num/100000%10];
		IsContinue = false;
	}
	
//	if(IsContinue == true && Num/10000%10 == 0)
//	{
//		one_Thousand_number = 0x00;
//	}
//	else
	{
		one_Thousand_number = num[Num/10000%10];
		IsContinue = false;
	}
	
	Hundred_number = num[Num/1000%10];
	Decate_number = num[Num/100%10];
	Unit_number = num[Num/10%10];
	Dot_number = num[Num/1%10];//个位
	
	for(i=0;i<7;i++)//代表数码管的七段
	{
		Read_Modify_Write_1621(NUM_Addr[i+42]/4,1<<NUM_Addr[i+42]%4,(Dot_number & (1<<i))==0 ? 0 : 1);
		
		Read_Modify_Write_1621(NUM_Addr[i+35]/4,1<<NUM_Addr[i+35]%4,(Unit_number & (1<<i))==0 ? 0 :1);
		
		Read_Modify_Write_1621(NUM_Addr[i+28]/4,1<<NUM_Addr[i+28]%4,(Decate_number & (1<<i))==0 ? 0 : 1);
		
		Read_Modify_Write_1621(NUM_Addr[i+21]/4,1<<NUM_Addr[i+21]%4,(Hundred_number & (1<<i))==0 ? 0 : 1);
		
		Read_Modify_Write_1621(NUM_Addr[i+14]/4,1<<NUM_Addr[i+14]%4,(one_Thousand_number & (1<<i))==0 ? 0 : 1);
		
		Read_Modify_Write_1621(NUM_Addr[i+7]/4,1<<NUM_Addr[i+7]%4,(The_Thousand_number & (1<<i))==0 ? 0 : 1);
		
		Read_Modify_Write_1621(NUM_Addr[i]/4,1<<NUM_Addr[i]%4,(Hun_Thousand_number & (1<<i))==0 ? 0 : 1);
	}
}

/**
  * @brief  设置显示字符符号，内部调用
  * @param  Addr：符号的显存地址
  * @param  Num：要显示的数字
  * @retval None
  */
static void SetDisChar(char *tmpDisChar)
{
	unsigned char i;
	char static num[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};//0-9位段编码
	char static dischar[] = {0x40};//负号,
	unsigned char char1[7] = {0};//1
	 
	for(i=0;i<7;i++)//代表数码管的七段
	{
		switch(*tmpDisChar++)
		{
		  case '0':
				char1[i] = num[0];
				break;
			case '1':
				char1[i] = num[1];
				break;
			case '2':
				char1[i] = num[2];
				break;
			case '3':
				char1[i] = num[3];
				break;
			case '4':
				char1[i] = num[4];
				break;
			case '5':
				char1[i] = num[5];
				break;
			case '6':
				char1[i] = num[6];
				break;
			case '7':
				char1[i] = num[7];
				break;
			case '8':
				char1[i] = num[8];
				break;
			case '9':
				char1[i] = num[9];
				break;
			case '-':
				char1[i] = dischar[0];
				break;
			case ' ':
				char1[i] = 0x00;
				break;
		}
	}
	
	for(i=0;i<7;i++)//代表数码管的七段
	{
		Read_Modify_Write_1621(NUM_Addr[i+42]/4,1<<NUM_Addr[i+42]%4,(char1[6] & (1<<i))==0 ? 0 : 1);
		
		Read_Modify_Write_1621(NUM_Addr[i+35]/4,1<<NUM_Addr[i+35]%4,(char1[5] & (1<<i))==0 ? 0 :1);
		
		Read_Modify_Write_1621(NUM_Addr[i+28]/4,1<<NUM_Addr[i+28]%4,(char1[4] & (1<<i))==0 ? 0 : 1);
		
		Read_Modify_Write_1621(NUM_Addr[i+21]/4,1<<NUM_Addr[i+21]%4,(char1[3] & (1<<i))==0 ? 0 : 1);
		
		Read_Modify_Write_1621(NUM_Addr[i+14]/4,1<<NUM_Addr[i+14]%4,(char1[2] & (1<<i))==0 ? 0 : 1);
		
		Read_Modify_Write_1621(NUM_Addr[i+7]/4,1<<NUM_Addr[i+7]%4,(char1[1] & (1<<i))==0 ? 0 : 1);
		
		Read_Modify_Write_1621(NUM_Addr[i]/4,1<<NUM_Addr[i]%4,(char1[0] & (1<<i))==0 ? 0 : 1);
	}
}
/**
  * @brief  不显示数字符号，内部调用
  * @param  Addr：符号的显存地址 NUM_Addr
  * @retval None
  */
//static void ClearNum(unsigned char* Addr)
//{
//	unsigned char i;

//	for(i=0;i<7;i++)
//	{
//		Read_Modify_Write_1621(Addr[i+42]/4,1<<Addr[i+7]%4,0);
//		Read_Modify_Write_1621(Addr[i+35]/4,1<<Addr[i+7]%4,0);
//		Read_Modify_Write_1621(Addr[i+28]/4,1<<Addr[i+7]%4,0);
//		Read_Modify_Write_1621(Addr[i+21]/4,1<<Addr[i+7]%4,0);
//		Read_Modify_Write_1621(Addr[i+14]/4,1<<Addr[i+7]%4,0);
//		Read_Modify_Write_1621(Addr[i+7]/4,1<<Addr[i+7]%4,0);
//		Read_Modify_Write_1621(Addr[i]/4,1<<Addr[i]%4,0);
//	}
//}

/**
  * @brief  设置显示单个符号，内部调用
  * @param  Addr：符号的显存地址
  * @param  NewState：符号的状态，取值为ON或者OFF
  * @retval None
  */
static void SetSymbolState(unsigned char Addr,LCDSymbolState_Type NewState)
{
	if(NewState==ON)
	{
		Read_Modify_Write_1621(Addr/4,1<<Addr%4, 1);
	}
	else
	{
		Read_Modify_Write_1621(Addr/4,1<<Addr%4, 0);
	}
}


void LCD_Close(void)//液晶控制器初始化
{
	HT1621_InitTypeDef HT1621_InitStruct;
	
	memset(&LCDPanelStruct,0,sizeof(LCDPanel_TypeDef));
	memset(&LCDPanel_Tmp,0,sizeof(LCDPanel_TypeDef));
	
	HT1621_InitStruct.BiasComNum = FourCOMS;
	HT1621_InitStruct.SysState   = SYSDIS;
	HT1621_InitStruct.LcdState   = LCDOFF;
	
	SendCmd(HT1621_InitStruct.BiasComNum);
	SendCmd(HT1621_InitStruct.SysState);
	SendCmd(HT1621_InitStruct.LcdState);
}

/**
  * @brief  按照LCDPanel的参数更新LCD的显示,
  * @param  LCDPanel: 指向LCDPanel_TypeDef结构体类型的指针，这个结构体包含了LCD的显示参数
  * @retval None
  */
void LCD_Refresh(LCDPanel_TypeDef* LCDPanel)
{
	LCDUpdate_Type Update_Type = Update_None;
	
	//if(LCDPanel_Tmp.DisNumber != LCDPanel->DisNumber || LCDPanel->DisNumber == 0)
	{
		Update_Type = Update_Type | Update_num;
		LCDPanel_Tmp.DisNumber = LCDPanel->DisNumber;
		strncpy(LCDPanel_Tmp.DisChar,LCDPanel->DisChar,7);
	}
//	
//	if(LCDPanel_Tmp.Sys_Time_HH != LCDPanel->Sys_Time_HH)
//	{
//		Update_Type = Update_Type | Update_num;
//		LCDPanel_Tmp.Sys_Time_HH = LCDPanel->Sys_Time_HH;
//	}
//	if(LCDPanel_Tmp.Sys_Time_MM != LCDPanel->Sys_Time_MM)
//	{
//		Update_Type = Update_Type | Update_num;
//		LCDPanel_Tmp.Sys_Time_MM = LCDPanel->Sys_Time_MM;
//	}
//	if(LCDPanel_Tmp.Sys_Time_SS != LCDPanel->Sys_Time_SS)
//	{
//		Update_Type = Update_Type | Update_num;
//		LCDPanel_Tmp.Sys_Time_SS = LCDPanel->Sys_Time_SS;
//	}
	
  if(LCDPanel_Tmp.XH1_Sign != LCDPanel->XH1_Sign)
	{
		Update_Type = Update_Type | Update_Signal_1;
		LCDPanel_Tmp.XH1_Sign = LCDPanel->XH1_Sign;
	}
	if(LCDPanel_Tmp.XH2_Sign != LCDPanel->XH2_Sign)
	{
		Update_Type = Update_Type | Update_Signal_2;
		LCDPanel_Tmp.XH2_Sign = LCDPanel->XH2_Sign;
	}
	if(LCDPanel_Tmp.XH3_Sign != LCDPanel->XH3_Sign)
	{
		Update_Type = Update_Type | Update_Signal_3;
		LCDPanel_Tmp.XH3_Sign = LCDPanel->XH3_Sign;
	}
	if(LCDPanel_Tmp.XH4_Sign != LCDPanel->XH4_Sign)
	{
		Update_Type = Update_Type | Update_Signal_4;
		LCDPanel_Tmp.XH4_Sign = LCDPanel->XH4_Sign;
	}
	if(LCDPanel_Tmp.XH5_Sign != LCDPanel->XH5_Sign)
	{
		Update_Type = Update_Type | Update_Signal_5;
		LCDPanel_Tmp.XH5_Sign = LCDPanel->XH5_Sign;
	}
	if(LCDPanel_Tmp.Bluetooth_Sign != LCDPanel->Bluetooth_Sign)
	{
		Update_Type = Update_Type | Update_Bluetooth;
		LCDPanel_Tmp.Bluetooth_Sign = LCDPanel->Bluetooth_Sign;
	}
	if(LCDPanel_Tmp.RFID_Sign != LCDPanel->RFID_Sign)
	{
		Update_Type = Update_Type | Update_RFID;
		LCDPanel_Tmp.RFID_Sign = LCDPanel->RFID_Sign;
	}
	if(LCDPanel_Tmp.WRAN_Sign != LCDPanel->WRAN_Sign)
	{
		Update_Type = Update_Type | Update_WRAN;
		LCDPanel_Tmp.WRAN_Sign = LCDPanel->WRAN_Sign;
	}
	if(LCDPanel_Tmp.ValveW_Sign != LCDPanel->ValveW_Sign)
	{
		Update_Type = Update_Type | Update_ValveW;
		LCDPanel_Tmp.ValveW_Sign = LCDPanel->ValveW_Sign;
	}
	if(LCDPanel_Tmp.ValveL_Sign != LCDPanel->ValveL_Sign)
	{
		Update_Type = Update_Type | Update_ValveL;
		LCDPanel_Tmp.ValveL_Sign = LCDPanel->ValveL_Sign;
	}
	if(LCDPanel_Tmp.ValveM_Sign != LCDPanel->ValveM_Sign)
	{
		Update_Type = Update_Type | Update_ValveM;
		LCDPanel_Tmp.ValveM_Sign = LCDPanel->ValveM_Sign;
	}
	if(LCDPanel_Tmp.ValveR_Sign != LCDPanel->ValveR_Sign)
	{
		Update_Type = Update_Type | Update_ValveR;
		LCDPanel_Tmp.ValveR_Sign = LCDPanel->ValveR_Sign;
	}
	if(LCDPanel_Tmp.syncBig_Sign != LCDPanel->syncBig_Sign)
	{
		Update_Type = Update_Type | Update_syncBig;
		LCDPanel_Tmp.syncBig_Sign = LCDPanel->syncBig_Sign;
	}
	if(LCDPanel_Tmp.syncMid_Sign != LCDPanel->syncMid_Sign)
	{
		Update_Type = Update_Type | Update_syncMid;
		LCDPanel_Tmp.syncMid_Sign = LCDPanel->syncMid_Sign;
	}
	if(LCDPanel_Tmp.syncSma_Sign != LCDPanel->syncSma_Sign)
	{
		Update_Type = Update_Type | Update_syncSma;
		LCDPanel_Tmp.syncSma_Sign = LCDPanel->syncSma_Sign;
	}
	if(LCDPanel_Tmp.cylinder_Sign != LCDPanel->cylinder_Sign)
	{
		Update_Type = Update_Type | Update_cylinder;
		LCDPanel_Tmp.cylinder_Sign = LCDPanel->cylinder_Sign;
	}
	if(LCDPanel_Tmp.KgSign_Sign != LCDPanel->KgSign_Sign)
	{
		Update_Type = Update_Type | Update_KgSign;
		LCDPanel_Tmp.KgSign_Sign = LCDPanel->KgSign_Sign;
	}
	if(LCDPanel_Tmp.MONEY_Sign != LCDPanel->MONEY_Sign)
	{
		Update_Type = Update_Type | Update_MONEY;
		LCDPanel_Tmp.MONEY_Sign = LCDPanel->MONEY_Sign;
	}
	if(LCDPanel_Tmp.POINT1_Sign != LCDPanel->POINT1_Sign)
	{
		Update_Type = Update_Type | Update_POINT1;
		LCDPanel_Tmp.POINT1_Sign = LCDPanel->POINT1_Sign;
	}
	if(LCDPanel_Tmp.POINT2_Sign != LCDPanel->POINT2_Sign)
	{
		Update_Type = Update_Type | Update_POINT2;
		LCDPanel_Tmp.POINT2_Sign = LCDPanel->POINT2_Sign;
	}
	if(LCDPanel_Tmp.POINT3_Sign != LCDPanel->POINT3_Sign)
	{
		Update_Type = Update_Type | Update_POINT3;
		LCDPanel_Tmp.POINT3_Sign = LCDPanel->POINT3_Sign;
	}
	if(LCDPanel_Tmp.POINT4_Sign != LCDPanel->POINT4_Sign)
	{
		Update_Type = Update_Type | Update_POINT4;
		LCDPanel_Tmp.POINT4_Sign = LCDPanel->POINT4_Sign;
	}
	if(LCDPanel_Tmp.POINT5_Sign != LCDPanel->POINT5_Sign)
	{
		Update_Type = Update_Type | Update_POINT5;
		LCDPanel_Tmp.POINT5_Sign = LCDPanel->POINT5_Sign;
	}
	if(LCDPanel_Tmp.POINT6_Sign != LCDPanel->POINT6_Sign)
	{
		Update_Type = Update_Type | Update_POINT6;
		LCDPanel_Tmp.POINT6_Sign = LCDPanel->POINT6_Sign;
	}
	if(LCDPanel_Tmp.BATS0_Sign != LCDPanel->BATS0_Sign)
	{
		Update_Type = Update_Type | Update_BATS0;
		LCDPanel_Tmp.BATS0_Sign = LCDPanel->BATS0_Sign;
	}
	if(LCDPanel_Tmp.BATS1_Sign != LCDPanel->BATS1_Sign)
	{
		Update_Type = Update_Type | Update_BATS1;
		LCDPanel_Tmp.BATS1_Sign = LCDPanel->BATS1_Sign;
	}
	if(LCDPanel_Tmp.BATS2_Sign != LCDPanel->BATS2_Sign)
	{
		Update_Type = Update_Type | Update_BATS2;
		LCDPanel_Tmp.BATS2_Sign = LCDPanel->BATS2_Sign;
	}
	if(LCDPanel_Tmp.BATS3_Sign != LCDPanel->BATS3_Sign)
	{
		Update_Type = Update_Type | Update_BATS3;
		LCDPanel_Tmp.BATS3_Sign = LCDPanel->BATS3_Sign;
	}
	if(LCDPanel_Tmp.BATS4_Sign != LCDPanel->BATS4_Sign)
	{
		Update_Type = Update_Type | Update_BATS4;
		LCDPanel_Tmp.BATS4_Sign = LCDPanel->BATS4_Sign;
	}
	
	if(Update_Type>0)//有数据要更新
	{
		LCDUpdate_Type type = Update_Type;
		unsigned char Type_n;
		unsigned int Current_Type,pos;
		unsigned int Total = Update_All;
		Update_Type = Update_None;
		
		for(Type_n = 0;Total>0;Type_n++)//更新数据
		{
			Total>>=1;//判断是否遍历完整个LCDUpdate_Type枚举类型
			pos = 1<<Type_n;
			Current_Type = type & pos;
			if(Current_Type == pos)
			{
				switch(Current_Type)
				{
					case Update_Signal_1:
						SetSymbolState(XH1,LCDPanel->XH1_Sign);
					  break;
					case Update_Signal_2:
						SetSymbolState(XH2,LCDPanel->XH2_Sign);
					  break;
					case Update_Signal_3:
						SetSymbolState(XH3,LCDPanel->XH3_Sign);
					  break;
					case Update_Signal_4:
						SetSymbolState(XH4,LCDPanel->XH4_Sign);
					  break;
					case Update_Signal_5:
						SetSymbolState(XH5,LCDPanel->XH5_Sign);
					  break;
					
					case Update_Bluetooth:
						SetSymbolState(Bluetooth,LCDPanel->Bluetooth_Sign);
					  break;
					case Update_RFID:
						SetSymbolState(RFID,LCDPanel->RFID_Sign);
					  break;
					case Update_WRAN:
						SetSymbolState(WRAN,LCDPanel->WRAN_Sign);
					  break;
					case Update_ValveW:
						SetSymbolState(ValveW,LCDPanel->ValveW_Sign);
					  break;
					case Update_ValveL:
						SetSymbolState(ValveL,LCDPanel->ValveL_Sign);
					  break;
					case Update_ValveM:
						SetSymbolState(ValveM,LCDPanel->ValveM_Sign);
					  break;
					case Update_ValveR:
						SetSymbolState(ValveR,LCDPanel->ValveR_Sign);
					  break;
					case Update_syncBig:
						SetSymbolState(syncBig,LCDPanel->syncBig_Sign);
					  break;
					case Update_syncMid:
						SetSymbolState(syncMid,LCDPanel->syncMid_Sign);
					  break;
					case Update_syncSma:
						SetSymbolState(syncSma,LCDPanel->syncSma_Sign);
					  break;
					case Update_cylinder:
						SetSymbolState(cylinder,LCDPanel->cylinder_Sign);
					  break;
					case Update_KgSign:
						SetSymbolState(KgSign,LCDPanel->KgSign_Sign);
					  break;
					case Update_MONEY:
						SetSymbolState(MONEY,LCDPanel->MONEY_Sign);
					  break;
					
					case Update_BATS0:
						SetSymbolState(BATS0,LCDPanel->BATS0_Sign);
					  break;
					case Update_BATS1:
						SetSymbolState(BATS1,LCDPanel->BATS1_Sign);
					  break;
					case Update_BATS2:
						SetSymbolState(BATS2,LCDPanel->BATS2_Sign);
					  break;
					case Update_BATS3:
						SetSymbolState(BATS3,LCDPanel->BATS3_Sign);
					  break;
					case Update_BATS4:
						SetSymbolState(BATS4,LCDPanel->BATS4_Sign);
					  break;
					
					case Update_POINT1:
						SetSymbolState(POINT1,LCDPanel->POINT1_Sign);
					  break;
					case Update_POINT2:
						SetSymbolState(POINT2,LCDPanel->POINT2_Sign);
					  break;
					case Update_POINT3:
						SetSymbolState(POINT3,LCDPanel->POINT3_Sign);
					  break;
					case Update_POINT4:
						SetSymbolState(POINT4,LCDPanel->POINT4_Sign);
					  break;
					case Update_POINT5:
						SetSymbolState(POINT5,LCDPanel->POINT5_Sign);
					  break;
					case Update_POINT6:
						SetSymbolState(POINT6,LCDPanel->POINT6_Sign);
					  break;
					
					case Update_num:
						if(LCDPanel->DisType == 0)
						{
								SetNum(LCDPanel->DisNumber);
						}
						else
						{
								SetDisChar(LCDPanel->DisChar);
						}
					  break;
//					case Update_BackLight://背光灯
//						LCDPanel->BackLight==ON ? BL_ON() : BL_OFF();
//						break;
					default:
						break;
				}
			}
		}
		memset(LCDPanel,0,sizeof(LCDPanel_TypeDef));
	}
}

/**
  * @brief  LCD初始化操作
  * @param  none
  * @retval None
  */
void LCD_Init()
{
	unsigned char i;
	
	HT1621_InitTypeDef HT1621_InitStruct;
	
	HT1621_InitStruct.BiasComNum = FourCOMS;
	HT1621_InitStruct.SysState   = SYSEN;
	HT1621_InitStruct.LcdState   = LCDON;
	
	GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
//  __HAL_RCC_GPIOC_CLK_ENABLE();
	
  GPIO_InitStruct.Pin = LCD_CS_Pin|LCD_RD_Control_Pin|LCD_WR_Control_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL; 
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF0_SPI2;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);	

  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP; 
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

osDelay(100);//必须有,不然屏幕无法点亮
	
	SendCmd(HT1621_InitStruct.BiasComNum);
	SendCmd(HT1621_InitStruct.SysState);
	SendCmd(HT1621_InitStruct.LcdState);
	
	if(IsTestLcd == false)
	{
	  IsTestLcd = true;
		for(i = 0; i< 32; i++)//复位屏幕，写满字符
		{
			Write_1621(i,0xF); 
		}  
		
		osDelay(500);
	}
	for(i = 0; i< 32; i++)//复位屏幕,清零字符
	{
		Write_1621(i,0x0); 
	} 

  memset(&LCDPanelStruct,0,sizeof(LCDPanel_TypeDef));
	memset(&LCDPanel_Tmp,0,sizeof(LCDPanel_TypeDef));	
}

/**
  * @brief  显示信号强度
  * @retval None
  */
void RefreshSignal(void)
{
	  static uint8_t LCD_Net_Time  = 0;
	
		REAL_DATA_PARAM.GprsCsq = Sim80x.Status.Signal;
	  //printf("connectStep %d\r\n",connectStep);
		if(connectStep > 4)
		{			
			if(REAL_DATA_PARAM.GprsCsq <=10 || REAL_DATA_PARAM.GprsCsq > 31)
			{
					if(GSM_ON_FLAG == 1)
					{
						LCDPanelStruct.XH1_Sign = ON;
						LCDPanelStruct.XH2_Sign = OFF;
					  LCDPanelStruct.XH3_Sign = OFF;
					  LCDPanelStruct.XH4_Sign = OFF;
					  LCDPanelStruct.XH5_Sign = OFF;
					}
			} 
			else if(REAL_DATA_PARAM.GprsCsq >10 && REAL_DATA_PARAM.GprsCsq <=15)
			{
					if(GSM_ON_FLAG == 1)
					{
						LCDPanelStruct.XH1_Sign = ON;
						LCDPanelStruct.XH2_Sign = ON;
					  LCDPanelStruct.XH3_Sign = OFF;
					  LCDPanelStruct.XH4_Sign = OFF;
					  LCDPanelStruct.XH5_Sign = OFF;
					}
			} 
			else if(REAL_DATA_PARAM.GprsCsq >15 && REAL_DATA_PARAM.GprsCsq <= 20)
			{
					if(GSM_ON_FLAG == 1)
					{
						LCDPanelStruct.XH1_Sign = ON;
						LCDPanelStruct.XH2_Sign = ON;
						LCDPanelStruct.XH3_Sign = ON;
						LCDPanelStruct.XH4_Sign = OFF;
					  LCDPanelStruct.XH5_Sign = OFF;
					}
			} 
			else if(REAL_DATA_PARAM.GprsCsq >20 && REAL_DATA_PARAM.GprsCsq <= 25)
			{
					if(GSM_ON_FLAG == 1)
					{
						LCDPanelStruct.XH1_Sign = ON;
						LCDPanelStruct.XH2_Sign = ON;
						LCDPanelStruct.XH3_Sign = ON;
						LCDPanelStruct.XH4_Sign = ON;
						LCDPanelStruct.XH5_Sign = OFF;
					}
			} 
			else
			{
				if(GSM_ON_FLAG == 1)
				{
					LCDPanelStruct.XH1_Sign = ON;
					LCDPanelStruct.XH2_Sign = ON;
					LCDPanelStruct.XH3_Sign = ON;
					LCDPanelStruct.XH4_Sign = ON;
					LCDPanelStruct.XH5_Sign = ON;
				}
			}
		}
		else
		{
		    if(LCD_Net_Time == 0)
				{
					LCDPanelStruct.XH1_Sign = OFF;
					LCDPanelStruct.XH2_Sign = OFF;
					LCDPanelStruct.XH3_Sign = OFF;
					LCDPanelStruct.XH4_Sign = OFF;
					LCDPanelStruct.XH5_Sign = OFF;
				}
				if(LCD_Net_Time == 1)
				{
					LCDPanelStruct.XH1_Sign = ON;
					LCDPanelStruct.XH2_Sign = OFF;
					LCDPanelStruct.XH3_Sign = OFF;
					LCDPanelStruct.XH4_Sign = OFF;
					LCDPanelStruct.XH5_Sign = OFF;
				}
				if(LCD_Net_Time == 2)
				{
					LCDPanelStruct.XH1_Sign = ON;
					LCDPanelStruct.XH2_Sign = ON;
					LCDPanelStruct.XH3_Sign = OFF;
					LCDPanelStruct.XH4_Sign = OFF;
					LCDPanelStruct.XH5_Sign = OFF;
				}
				if(LCD_Net_Time == 3)
				{
					LCDPanelStruct.XH1_Sign = ON;
					LCDPanelStruct.XH2_Sign = ON;
					LCDPanelStruct.XH3_Sign = ON;
					LCDPanelStruct.XH4_Sign = OFF;
					LCDPanelStruct.XH5_Sign = OFF;
				}
				if(LCD_Net_Time == 4)
				{
					LCDPanelStruct.XH1_Sign = ON;
					LCDPanelStruct.XH2_Sign = ON;
					LCDPanelStruct.XH3_Sign = ON;
					LCDPanelStruct.XH4_Sign = ON;
					LCDPanelStruct.XH5_Sign = OFF;
				}
				if(LCD_Net_Time == 5)
				{
					LCDPanelStruct.XH1_Sign = ON;
					LCDPanelStruct.XH2_Sign = ON;
					LCDPanelStruct.XH3_Sign = ON;
					LCDPanelStruct.XH4_Sign = ON;
					LCDPanelStruct.XH5_Sign = ON;
				}
		}
		if(GSM_ON_FLAG == 1)
		{
			if(connectStep == 11)//网络连接同步数据中
			{
			  LCDPanelStruct.syncSma_Sign = ON;	
				if(LCD_Net_Time == 0)
				{
					LCDPanelStruct.syncMid_Sign = OFF;
					LCDPanelStruct.syncBig_Sign = OFF;
				}
				if(LCD_Net_Time == 1)
				{
					LCDPanelStruct.syncMid_Sign = ON;
					LCDPanelStruct.syncBig_Sign = OFF;
				}
				if(LCD_Net_Time == 2)
				{
					LCDPanelStruct.syncMid_Sign = ON;
					LCDPanelStruct.syncBig_Sign = ON;
				}
				if(LCD_Net_Time == 3)
				{
					LCDPanelStruct.syncMid_Sign = ON;
					LCDPanelStruct.syncBig_Sign = ON;
				}
				if(LCD_Net_Time == 4)
				{
					LCDPanelStruct.syncMid_Sign = OFF;
					LCDPanelStruct.syncBig_Sign = ON;
				}
				if(LCD_Net_Time == 5)
				{
					LCDPanelStruct.syncMid_Sign = OFF;
					LCDPanelStruct.syncBig_Sign = OFF;
				}
			}
			else if(connectStep > 4 && connectStep < 11)//注册成功,联网中
			{
				LCDPanelStruct.syncSma_Sign = ON;
				LCDPanelStruct.syncBig_Sign = OFF;
				if(LCD_Net_Time == 0)
				{
					LCDPanelStruct.syncMid_Sign = OFF;
				}
				else
				{
					LCDPanelStruct.syncMid_Sign = ON;
				}
			}
			else
			{
				LCDPanelStruct.syncSma_Sign = OFF;
				LCDPanelStruct.syncBig_Sign = OFF;
				LCDPanelStruct.syncMid_Sign = OFF;
			}
		}
		else
		{
		  LCDPanelStruct.syncBig_Sign = OFF;
      LCDPanelStruct.syncMid_Sign = OFF;
			LCDPanelStruct.syncSma_Sign = OFF;
			
			LCDPanelStruct.XH1_Sign = OFF;
			LCDPanelStruct.XH2_Sign = OFF;
			LCDPanelStruct.XH3_Sign = OFF;
			LCDPanelStruct.XH4_Sign = OFF;
			LCDPanelStruct.XH5_Sign = OFF;
		}

		LCD_Net_Time = (LCD_Net_Time + 1) % 6;
}

/**
  * @brief 显示电池电压
  * @retval None
  */
void RefreshBatVoltage(void)
{
	  static uint8_t LCD_Voltage_Time  = 0;
		if(REAL_DATA_PARAM.BatVoltage <= 3.6)//REAL_DATA_PARAM.BatVoltage >= 3 && 
		{
			 if(LCD_Voltage_Time == 0)
			 {				 
				 LCDPanelStruct.BATS0_Sign =ON;
			 }
			 else
			 {
			   LCDPanelStruct.BATS0_Sign =OFF;
			 }
		} 
		else if(REAL_DATA_PARAM.BatVoltage > 3.6 && REAL_DATA_PARAM.BatVoltage <= 3.602)
		{
				LCDPanelStruct.BATS0_Sign =ON;
				if(LCD_Voltage_Time == 0)
				{				 
					 LCDPanelStruct.BATS1_Sign =ON;
				}
				else
				{
					 LCDPanelStruct.BATS1_Sign =OFF;
				}
				LCDPanelStruct.BATS2_Sign =OFF;
				LCDPanelStruct.BATS3_Sign =OFF;
			  LCDPanelStruct.BATS4_Sign =OFF;
		} 
		else if(REAL_DATA_PARAM.BatVoltage > 3.602 && REAL_DATA_PARAM.BatVoltage <= 3.7)
		{
				LCDPanelStruct.BATS0_Sign =ON;				 
				LCDPanelStruct.BATS1_Sign =ON;
				LCDPanelStruct.BATS2_Sign =OFF;
				LCDPanelStruct.BATS3_Sign =OFF;
			  LCDPanelStruct.BATS4_Sign =OFF;
		} 
		else if(REAL_DATA_PARAM.BatVoltage > 3.7 && REAL_DATA_PARAM.BatVoltage <= 3.702)
		{
				LCDPanelStruct.BATS0_Sign =ON;
				LCDPanelStruct.BATS1_Sign =ON;
				if(LCD_Voltage_Time == 0)
				{				 
					 LCDPanelStruct.BATS2_Sign =ON;
				}
				else
				{
					 LCDPanelStruct.BATS2_Sign =OFF;
				}
				LCDPanelStruct.BATS3_Sign =OFF;
			  LCDPanelStruct.BATS4_Sign =OFF;
		} 
		else if(REAL_DATA_PARAM.BatVoltage > 3.702 && REAL_DATA_PARAM.BatVoltage <= 3.9)
		{
				LCDPanelStruct.BATS0_Sign =ON;
				LCDPanelStruct.BATS1_Sign =ON;
				LCDPanelStruct.BATS2_Sign =ON;
				LCDPanelStruct.BATS3_Sign =OFF;
			  LCDPanelStruct.BATS4_Sign =OFF;
		} 
		else if(REAL_DATA_PARAM.BatVoltage > 3.9 && REAL_DATA_PARAM.BatVoltage <= 3.902)
		{
				LCDPanelStruct.BATS0_Sign =ON;
				LCDPanelStruct.BATS1_Sign =ON;
				LCDPanelStruct.BATS2_Sign =ON;
				if(LCD_Voltage_Time == 0)
				{				 
					 LCDPanelStruct.BATS3_Sign =ON;
				}
				else
				{
					 LCDPanelStruct.BATS3_Sign =OFF;
				}
			  LCDPanelStruct.BATS4_Sign =OFF;
		}
		else if(REAL_DATA_PARAM.BatVoltage > 3.902 && REAL_DATA_PARAM.BatVoltage <= 4.0)
		{
				LCDPanelStruct.BATS0_Sign =ON;
				LCDPanelStruct.BATS1_Sign =ON;
				LCDPanelStruct.BATS2_Sign =ON;
				LCDPanelStruct.BATS3_Sign =ON;
			  LCDPanelStruct.BATS4_Sign =OFF;
		}
		else
		{
				LCDPanelStruct.BATS0_Sign =ON;
				LCDPanelStruct.BATS1_Sign =ON;
				LCDPanelStruct.BATS2_Sign =ON;
				LCDPanelStruct.BATS3_Sign =ON;
			  LCDPanelStruct.BATS4_Sign =ON;
		}
		LCD_Voltage_Time = (LCD_Voltage_Time + 1) % 2;
}	

/**
  * @brief 刷新阀门
  * @retval None
  */
void RefreshValve(void)
{
	  static uint8_t LCD_Valve_Time  = 0;
	  LCDPanelStruct.ValveW_Sign = ON;
	
	  if(NEEDLE_input == 0)
		{
		   LCDPanelStruct.ValveL_Sign = ON;
		}
		else
		{
		  LCDPanelStruct.ValveL_Sign = OFF;
		}
		
		if(strcmp(REAL_DATA_PARAM.ElectricValveStatus,"1") == 0)
		{
			 LCDPanelStruct.ValveM_Sign = ON;
		}

		if(TmpRealTimeFlow != 0)
		{
			  if(LCD_Valve_Time == 0)
				{
					LCDPanelStruct.ValveL_Sign =ON;
					LCDPanelStruct.ValveM_Sign =ON;
					LCDPanelStruct.ValveR_Sign =ON;
				}
				else if(LCD_Valve_Time == 1)
				{
					LCDPanelStruct.ValveL_Sign =OFF;
					LCDPanelStruct.ValveM_Sign =OFF;
					LCDPanelStruct.ValveR_Sign =OFF;
				}
				else if(LCD_Valve_Time == 2)
				{
					LCDPanelStruct.ValveL_Sign =ON;
					LCDPanelStruct.ValveM_Sign =OFF;
					LCDPanelStruct.ValveR_Sign =OFF;
				}
				else if(LCD_Valve_Time == 3)
				{
					LCDPanelStruct.ValveL_Sign =ON;
					LCDPanelStruct.ValveM_Sign =ON;
					LCDPanelStruct.ValveR_Sign =OFF;
				}

			  LCD_Valve_Time = (LCD_Valve_Time + 1) % 4;
		} 
		else
		{
			  LCDPanelStruct.ValveR_Sign = OFF;
		} 
}	

/**
  * @brief 刷新报警
  * @retval None
  */
void RefreshWarn(void)
{
	  static uint8_t LCD_Warn_Time = 0;
	  if(LCD_Warn_Time == 0)
		{
			LCDPanelStruct.WRAN_Sign = ON;
			LCD_Warn_Time = 1;
		}
		else 
		{
		  LCDPanelStruct.WRAN_Sign = OFF;
			LCD_Warn_Time = 0;
		}
}

/**
  * @brief 刷新钢瓶
  * @retval None
  */
void RefreshCylinder(void)
{
	  static uint8_t LCD_cylinder_Time = 0;
	  if(LCD_cylinder_Time == 0)
		{
			LCDPanelStruct.cylinder_Sign = ON;
			LCD_cylinder_Time = 1;
		}
		else 
		{
		  LCDPanelStruct.cylinder_Sign = OFF;
			LCD_cylinder_Time = 0;
		}
		LCDPanelStruct.WRAN_Sign = ON;
}

/**
  * @brief 刷新金钱
  * @retval None
  */
void RefreshMoney(void)
{
	  static uint8_t LCD_money_Time = 0;
	  if(LCD_money_Time == 0)
		{
			LCDPanelStruct.MONEY_Sign = ON;
			LCD_money_Time = 1;
		}
		else 
		{
		  LCDPanelStruct.MONEY_Sign = OFF;
			LCD_money_Time = 0;
		}
		LCDPanelStruct.WRAN_Sign = ON;
}
