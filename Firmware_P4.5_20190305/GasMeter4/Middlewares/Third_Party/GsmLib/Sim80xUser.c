#include "Sim80x.h"
#include "bsp.h"
#include "SetupPacket.h"
#include "ControlPacket.h"
#include "UsedResetPacket.h"
#include "RechargePacket.h"

//联网步骤相关
extern int16_t connectStep;      //连接的步骤,停止无连接等于零,每成功一步加一 -1为联网之前
//发送报文相关
extern bool IsHaveSimError;
extern bool IsNeedWarning;
extern uint8_t WarnRetryNumber;
extern bool IsNeedCustomerCreditResponse;
extern bool IsNeedInformationResponse;
extern bool IsNeedSendCook; //做完饭上报
extern bool IsReceivedCSRPReply;
extern bool IsSendedCSRP;

extern CONFIG_Meter_t CONFIG_Meter;

extern uint32_t TimeForCurrStart;   //当前步骤开始运行的时间
extern uint32_t loopSumCurr;        //循环重试的次数
extern uint32_t loopSumPDP;         //循环重试的次数 当移动场景建立后,连接的时候可能不成功,这个时候要往前跳步骤,但有可能一直不成功,所以需要增加变量

void  Sim80x_UserInit(void)
{
    //GPRS_ConnectToNetwork("mcinet","","",false);
    //GPRS_HttpGet("www.google.com");
}

void  Sim80x_GPRSClose(int16_t cStep)
{
    printf("Sim80x_GPRSClose(%d)\r\n",cStep);
	
		IsHaveSimError = false;
	
		IsNeedWarning = false;
		//IsNeedSendCook = false;
	  IsSendedCSRP = false;
	  IsReceivedCSRPReply = false;
		IsNeedCustomerCreditResponse = false;
		IsNeedInformationResponse = false;
		IsNeedRepaySetup = false;
		IsNeedRepayControl = false;
		IsNeedRepayCYCH = false;
		IsNeedRepayReCharge = false;
		
    loopSumCurr = 0;
		loopSumPDP = 0;
	
		TimeForCurrStart=0;	
		GSM_ON_FLAG = 0;
		
		if(connectStep == 11)
		{
			if(GPRS_CloseSocket() == true)
			{
				//osDelay(500);
			}
		}
		
		connectStep = -1;
		
//		if(cStep == 10)
//		{
//			CONFIG_Meter.IsNormalReset = true;
//	    CONFIG_Meter_Write();
//			HAL_NVIC_SystemReset();
//		}

			switch(cStep)
			{
				case 1:
					LogWrite(Connect,"Sim800cError",NULL);//AT
					break;
				case 2:
					LogWrite(Connect,"AT+CGDCONT",NULL);//AT+CGDCONT
					break;
				case 3:
					LogWrite(Connect,"SimCardError",NULL);//CPIN READY 没有收到这个
					break;
				case 4:
					LogWrite(Connect,"RegToNetError ",NULL);//AT+CGREG
					break;
				case 5:
					LogWrite(Connect,"CGATTError",NULL);//CGATT  附着到网络 GPRS
					break;
				case 6:
					LogWrite(Connect,"AT+CIPSHUTError",NULL);//AT+CIPSHUT 清除上下文 
					break;
				case 8:
					LogWrite(Connect,"AT+CIICRError",NULL);//AT+CIICR  从网络申请IP地址等,官方叫做 PDP
					break;
				case 10:
					LogWrite(Connect,"ServerError",NULL);//连接到服务器错误
					break;
				case 11:
					LogWrite(Connect,"HeartFinish",NULL);//连接到服务器错误
					break;
				case 12:
					LogWrite(Connect,"LongButton",NULL);
					break;
			}

		Sim80x_SetPower(false);
}
