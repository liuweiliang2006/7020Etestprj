/**
  ******************************************************************************
  * File Name          : HT1621B.c
  * Description        :
	* xudong
  ******************************************************************************
  */

#include "ht1621b.h"

#define uchar unsigned char

void _nop(void)		//@11.0592MHz
{
//  __nop();
}

//功 能：写数据函数,cnt为传送数据位数,数据传送为高位在前
static void SendBit_1621(unsigned char Data,unsigned char cnt)
{
	uchar i;
	for(i =0; i <cnt; i ++)
	{
		if((Data&0x80)==0) 
		{
			DAT_W(0);
		}
		else 
		{
			DAT_W(1);
		}
		WR_W(0);
		_nop();
		WR_W(1);
		Data<<=1;
	}
}

//发送一个字节到芯片,与上面的区别是,先发送头尾的问题
static void SendDataBit_1621(unsigned char Data,unsigned char cnt)  
{
	uchar i;
	for(i =0; i <cnt; i ++)
	{
		if((Data&0x01)==0) 
		{
			DAT_W(0);
		}
		else 
		{
			DAT_W(1);
		}
		WR_W(0);
		_nop();
		WR_W(1);
		Data>>=1;
	}
}

//读取一个字节
static unsigned char ReadDataBit_1621(unsigned char cnt)
{
	uchar i;
	uchar Data=0;
	for(i = 0; i < cnt; i++)
	{
		DAT_W(1);
    _nop();
		RD_W(0);
		_nop();
		RD_W(1);
		_nop();
		if(DAT_R==1)
		{
			Data|=(1<<i);
		}
	}
	return Data;
}

//static unsigned char ReaData_1621(unsigned char addr)  //Data??????
//{
//	unsigned Data=0;
//	CS_W(0);
//	_nop_();
//	SendBit_1621(0xc0,3);//110
//	SendBit_1621(addr<<2,6);  //D¤??6??addr
//	Data=ReadDataBit_1621(4);
//	CS_W(1);
//	return Data;
//}

//发送命令
void SendCmd(unsigned char command)
{
	CS_W(0);
	SendBit_1621(0x80,3); 
	SendBit_1621(command,9); //写入9位数据,其中前8位为command 命令,最后1位任意
	CS_W(1);
}

//-----------------------------------------------------------------------------------------
//函数名称：void Write_1621(uchar addr,uchar Data)
//功能描述: HT1621在指定地址写入数据函数
//参数说明：Addr为写入初始地址，Data为写入数据
//说 明：因为HT1621的数据位4位，所以实际写入数据为参数的后4位
//-----------------------------------------------------------------------------------------
void Write_1621(unsigned char addr,unsigned char Data)
{
	CS_W(0);
	SendBit_1621(0xa0,3); 			//写入数据标志101
	SendBit_1621(addr<<2,6); 		//写入地址数据 
	SendDataBit_1621(Data,4);   //写入数据
	CS_W(1);
}

void WriteAll_1621(unsigned char addr,unsigned char *p,uchar cnt)
{
	uchar i;
	CS_W(0);
	SendBit_1621(0xa0,3); 
	SendBit_1621(addr<<2,6);  
	for(i =0; i < cnt; i++,p++)  
	{
		SendDataBit_1621(*p,8);
	}
	CS_W(1);
}

/**
* @brief  先读取然后在基础上修改,避免闪烁用的吧
* @param addr: 
* @param Mask
* @param Value
* @retval None
*/
void Read_Modify_Write_1621(unsigned char addr,unsigned char Mask, unsigned char Value)
{
	unsigned char i,pos;
	volatile unsigned char tmp_data;
	CS_W(0);
  _nop();
	SendBit_1621(0xa0,3); //	
	SendBit_1621(addr<<2,6);  
	tmp_data=ReadDataBit_1621(4);
//	tmp_data=ReaData_1621(addr);
  _nop();
	for(i=0;i<4;i++)
	{
		pos = 1<<i;
		if((Mask&pos)==pos)
		{
			if(Value==0)
			{
				tmp_data&=~pos;
			}
			else
			{
				tmp_data|=pos;
			}
		}
	}
	SendDataBit_1621(tmp_data,4);
	CS_W(1);
//	Write_1621(addr,tmp_data);
}

