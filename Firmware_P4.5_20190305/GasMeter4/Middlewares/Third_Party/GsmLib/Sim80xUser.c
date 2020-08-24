#include "Sim80x.h"
#include "bsp.h"
#include "SetupPacket.h"
#include "ControlPacket.h"
#include "UsedResetPacket.h"
#include "RechargePacket.h"

//�����������
extern int16_t connectStep;      //���ӵĲ���,ֹͣ�����ӵ�����,ÿ�ɹ�һ����һ -1Ϊ����֮ǰ
//���ͱ������
extern bool IsHaveSimError;
extern bool IsNeedWarning;
extern uint8_t WarnRetryNumber;
extern bool IsNeedCustomerCreditResponse;
extern bool IsNeedInformationResponse;
extern bool IsNeedSendCook; //���극�ϱ�
extern bool IsReceivedCSRPReply;
extern bool IsSendedCSRP;

extern CONFIG_Meter_t CONFIG_Meter;

extern uint32_t TimeForCurrStart;   //��ǰ���迪ʼ���е�ʱ��
extern uint32_t loopSumCurr;        //ѭ�����ԵĴ���
extern uint32_t loopSumPDP;         //ѭ�����ԵĴ��� ���ƶ�����������,���ӵ�ʱ����ܲ��ɹ�,���ʱ��Ҫ��ǰ������,���п���һֱ���ɹ�,������Ҫ���ӱ���

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
					LogWrite(Connect,"SimCardError",NULL);//CPIN READY û���յ����
					break;
				case 4:
					LogWrite(Connect,"RegToNetError ",NULL);//AT+CGREG
					break;
				case 5:
					LogWrite(Connect,"CGATTError",NULL);//CGATT  ���ŵ����� GPRS
					break;
				case 6:
					LogWrite(Connect,"AT+CIPSHUTError",NULL);//AT+CIPSHUT ��������� 
					break;
				case 8:
					LogWrite(Connect,"AT+CIICRError",NULL);//AT+CIICR  ����������IP��ַ��,�ٷ����� PDP
					break;
				case 10:
					LogWrite(Connect,"ServerError",NULL);//���ӵ�����������
					break;
				case 11:
					LogWrite(Connect,"HeartFinish",NULL);//���ӵ�����������
					break;
				case 12:
					LogWrite(Connect,"LongButton",NULL);
					break;
			}

		Sim80x_SetPower(false);
}
