#ifndef __USARTCONFIG_H__
#define __USARTCONFIG_H__

/* 包含头文件 ----------------------------------------------------------------*/
#include "stm32f0xx_hal.h"


#define ARG_NUM     8          //命令中允许的参数个数
#define CMD_LEN     20         //命令名占用的最大字符长度
#define CMD_BUF_LEN 200         //命令缓存的最大长度

/* 扩展变量 ------------------------------------------------------------------*/ 

typedef struct {
    char rec_buf[CMD_BUF_LEN];            //接收命令缓冲区
    char processed_buf[CMD_BUF_LEN];      //存储加工后的命令(去除控制字符)
    char * cmd_arg[ARG_NUM];              //保存命令的参数
}cmd_analyze_struct;

typedef struct {
        char const *cmd_name;                        //命令字符串
        int32_t max_args;                            //最大参数数目
        void (*handle)(int argc,char * cmd_arg[]);   //命令回调函数
        char  *help;                                 //帮助信息
}cmd_list_struct;

//extern const cmd_list_struct cmd_list[3];
extern uint8_t Usart3RxTemp;

/* 函数声明 ------------------------------------------------------------------*/

void fill_rec_buf(void);

void printf_hello(int32_t argc,char *cmd_arg[]);
void handle_arg(int32_t argc,char * cmd_arg[]);
void ReadSystemPara(int32_t argc,char * arg_arg[]);
void SetAddr(int32_t argc,char * arg_arg[]);
void DTISETTIME(int32_t argc,char * arg_arg[]);
void UpDuty(int32_t argc,char * arg_arg[]);
void StartDuty(int32_t argc,char * arg_arg[]);
void VeryLowPower(int32_t argc,char * arg_arg[]);
void HMI_type(int32_t argc,char * arg_arg[]);
//void Lock_version(int32_t argc,char * arg_arg[]);
void WarnBattery(int32_t argc,char * arg_arg[]);
void WarnPerpaid(int32_t argc,char * arg_arg[]);
void WarnVolume(int32_t argc,char * arg_arg[]);
void NominalCapacity(int32_t argc,char * arg_arg[]);
void GasRemain(int32_t argc,char * arg_arg[]);
void SetIP(int32_t argc,char * arg_arg[]);
void RESETALL(int32_t argc,char * arg_arg[]);
void ClearZero(int32_t argc,char * arg_arg[]);
void SetAPN(int32_t argc,char * arg_arg[]);
void ExportLog(int32_t argc,char * arg_arg[]);
void FindLogIndex(int32_t argc,char * arg_arg[]);
void SetName_APN(int32_t argc,char * arg_arg[]);
void SetAP_N_secret(int32_t argc,char * arg_arg[]);
void SetPin(int32_t argc,char * arg_arg[]);
void USE_GPRS_APN(int32_t argc,char * arg_arg[]);
void USE_SENSOR(int32_t argc,char * arg_arg[]);
void RealTimeFlow(int32_t argc,char * arg_arg[]);
void ResetSystem(int32_t argc,char * arg_arg[]);
void SetParaming(int32_t argc,char * arg_arg[]);
void readVoltage(int32_t argc,char * arg_arg[]);
void writeVoltage(int32_t argc,char * arg_arg[]);
void ReadHisVoltage(int32_t argc,char * arg_arg[]);

uint32_t get_true_char_stream(char *dest,const char *src);
int32_t cmd_arg_analyze(char *rec_buf,unsigned int len);

#endif  /* __BSP_SPIFLASH_H__ */

/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/
