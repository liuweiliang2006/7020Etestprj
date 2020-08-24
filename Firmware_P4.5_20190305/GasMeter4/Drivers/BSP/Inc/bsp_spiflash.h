#ifndef __BSP_SPIFLASH_H__
#define __BSP_SPIFLASH_H__

/* 包含头文件 ----------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* 获取缓冲区的长度 */
#define countof(a)      (sizeof(a) / sizeof(*(a)))
#define TxBufferSize1   (countof(TxBuffer1) - 1)
#define RxBufferSize1   (countof(TxBuffer1) - 1)
#define BufferSize      (countof(Tx_Buffer_W25Q64)-1)
#define BufferSize2(a)  (countof(a)-1)
#define Dummy_Byte                      0xFF

/* 扩展变量 ------------------------------------------------------------------*/
//extern uint8_t Tx_Buffer_W25Q64[];
//extern uint8_t Rx_Buffer_W25Q64[];

/* 函数声明 ------------------------------------------------------------------*/

uint8_t SPI_FLASH_ReadByte(void);
uint8_t SPI_FLASH_SendByte(uint8_t byte);

#endif  /* __BSP_SPIFLASH_H__ */

/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/
