#ifndef __W25Q64_H
#define __W25Q64_H			    
#include "stm32f0xx_hal.h"           

/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
#define SPI_FLASH_PageSize              256
#define SPI_FLASH_PerWritePageSize      256
#define W25X_WriteEnable		            0x06          //设置写使能锁存器
#define W25X_WriteDisable		            0x04 					//写禁止
#define W25X_ReadStatusReg		          0x05					//读状态寄存器 
#define W25X_WriteStatusReg		          0x01          //写状态寄存器 
#define W25X_ReadData			              0x03					//读存储器数据 
#define W25X_FastReadData		            0x0B 
#define W25X_FastReadDual		            0x3B 
#define W25X_PageProgram		            0x02					//写存储器数据 
#define W25X_BlockErase			            0xD8 
#define W25X_SectorErase		            0x20 					//擦除一个扇区
#define W25X_ChipErase			            0xC7 					//擦除正片
#define W25X_PowerDown			            0xB9					//进入掉电模式 
#define W25X_ReleasePowerDown	          0xAB					//唤醒flash  
#define W25X_DeviceID			              0xAB 					//读取flash设备ID
#define W25X_ManufactDeviceID   	      0x90 
#define W25X_JedecDeviceID		          0x9F					//读器件型号的ID 

#define WIP_Flag                        0x01  /* Write In Progress (WIP) flag */

#define  FLASH_WriteAddress     0x0000
#define  FLASH_ReadAddress      FLASH_WriteAddress
#define  FLASH_SectorToErase    FLASH_WriteAddress

#define  FLASH_RechargeBase           0x100000  //卡片信息的基地址 1M 的位置             0 1 0 0 0 0 0 0 
#define  FLASH_LogIndexBase           0x200000  //日志索引从 2M  开始                    0 0 1 1 0 0 0 0  没有附加网络数据的日志
#define  FLASH_CookingSessionBase     0x400000  //做饭信息的基地址 4M 的位置             0 0 0 0 1 0 0 0
#define  FLASH_LogBase                0x500000  //日志包含发送数据的从 5M 开始           0 0 0 0 0 1 1 0 
#define  FLASH_VoltageBase            0x700000  //日志索引从 7M 开始                     0 0 0 0 0 0 0 1

/* 类型定义 ------------------------------------------------------------------*/
/* 宏定义 --------------------------------------------------------------------*/
//#define  SPI_FLASH_ID                       0xEF3015     //W25X16   2MB
//#define  SPI_FLASH_ID                       0xEF4015	   //W25Q16   4MB
#define  SPI_FLASH_ID                       0XEF4017     //W25Q64   8MB
//#define  SPI_FLASH_ID                       0XEF4018     //W25Q128  16MB YS-F1Pro开发默认使用

//#define FLASH_SPIx                                 SPI2
//#define FLASH_SPIx_RCC_CLK_ENABLE()                __HAL_RCC_SPI2_CLK_ENABLE()
//#define FLASH_SPIx_RCC_CLK_DISABLE()               __HAL_RCC_SPI2_CLK_DISABLE()

//#define FLASH_SPI_GPIO_ClK_ENABLE()                __HAL_RCC_GPIOB_CLK_ENABLE() 
//#define FLASH_SPI_GPIO_PORT                        GPIOB
//#define FLASH_SPI_SCK_PIN                          GPIO_PIN_13
//#define FLASH_SPI_MISO_PIN                         GPIO_PIN_14
//#define FLASH_SPI_MOSI_PIN                         GPIO_PIN_15

//#define FLASH_SPI_CS_CLK_ENABLE()                  __HAL_RCC_GPIOA_CLK_ENABLE()    

#define FLASH_SPI_CS_ENABLE                      HAL_GPIO_WritePin(CS_FLASH_GPIO_Port, CS_FLASH_Pin, GPIO_PIN_RESET)
#define FLASH_SPI_CS_DISABLE                     HAL_GPIO_WritePin(CS_FLASH_GPIO_Port, CS_FLASH_Pin, GPIO_PIN_SET)

/* 函数声明 ------------------------------------------------------------------*/

void SPI_FLASH_SectorErase(uint32_t SectorAddr);
void SPI_FLASH_BulkErase(void);
void SPI_FLASH_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void SPI_FLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void W25Q64_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);
void SPI_FLASH_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
uint32_t SPI_FLASH_ReadID(void);
uint32_t SPI_FLASH_ReadDeviceID(void);
void SPI_FLASH_StartReadSequence(uint32_t ReadAddr);
void SPI_Flash_PowerDown(void);
void SPI_Flash_WAKEUP(void);

void SPI_FLASH_WriteEnable(void);
void SPI_FLASH_WaitForWriteEnd(void);

void TestW25Q64(void);

#endif
