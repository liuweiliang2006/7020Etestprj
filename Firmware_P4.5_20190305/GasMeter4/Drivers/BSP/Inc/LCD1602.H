#ifndef _LCD1602_H
#define _LCD1602_H
  #include "stm32f0xx_hal.h" //导入操作hal库的接口
	
  #define SetLCD_SID  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_SET)          //LCD_SID置位
  #define ReSetLCD_SID HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_RESET)        //LCD_SID复位
  
	#define ReadLCD_SID HAL_GPIO_ReadPin(GPIOC, GPIO_Pin_9) //读LCD_SID
	
  #define SetLCD_SCLK HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET) //LCD_SCLK置位 时钟
  #define ReSetLCD_SCLK HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET) //LCD_SCLK复位
	
  #define SetLCD_CS HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_SET) //LCD_CS置位  片选
  #define ReSetLCD_CS HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_RESET) //LCD_CS复位
	
  #define SetLCD_RS HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_SET) //LCD_RS置位    写命令
  #define ReSetLCD_RS HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET) //LCD_RS复位  写数据

  //____________________________________________________________________________
  //命令定义
  #define CLR 0x01//清除屏幕，置AC为零
  #define CursorRest 0x02//设DDRAM地址为零，显示回原位，DDRAM内容不变，光标复位
  #define CursorDirectionLeft 0x04//写入新数据后光标左移
  #define CursorLeftCreamRight 0x05//写入新数据后光标左移，显示屏整体右移1个字
  #define CursorDirectionRight 0x06//写入新数据后光标右移
  #define CursorRightCreamRight 0x07//写入新数据后光标右移，显示屏整体右移1个字
  #define CLOSECream 0x08//关闭显示
  #define OPENCream 0x0C//开屏幕显示，无光标
  #define Cursor 0x0E//开光标显示，不闪烁
  #define CursorBlink 0x0F//开光标显示，闪烁
  #define LEFT 0x10//光标左移1格
  #define RIGHT 0x14//光标右移1格
  #define LEFTCream 0x18//显示内容左移
  #define RIGHTCream 0x1C//显示内容右移
  #define MODE 0x38//显示模式设置：8位数据总线，显示两行，5*7点阵
  //____________________________________________________________________________
  void LCD_INI(void);//LCD初始化
  void DisplayChar(uint8_t line,uint8_t column,char *dp);//在指定行和列显示指针指向内容
  void ReadKey(void);//读按键
  //void Menu(uint8_t i);//显示菜单
  uint8_t *PParChange(uint32_t Parameter);
  void WriteLCD(uint8_t command,uint8_t data);
	void TimeDisplay(void);
	void DisplayTime(uint8_t);
  void DisplayDate(uint8_t);
	void DisplayParameter(uint8_t PageTemp,float data);
	void Lcd1602_show(uint8_t * sStep);

#endif



