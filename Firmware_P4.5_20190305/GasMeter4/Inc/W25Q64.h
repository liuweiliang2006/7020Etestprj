#ifndef __W25Q64_H
#define __W25Q64_H			    
#include "stm32f0xx_hal.h"           

/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
#define SPI_FLASH_PageSize              256
#define SPI_FLASH_PerWritePageSize      256
#define W25X_WriteEnable		            0x06          //����дʹ��������
#define W25X_WriteDisable		            0x04 					//д��ֹ
#define W25X_ReadStatusReg		          0x05					//��״̬�Ĵ��� 
#define W25X_WriteStatusReg		          0x01          //д״̬�Ĵ��� 
#define W25X_ReadData			              0x03					//���洢������ 
#define W25X_FastReadData		            0x0B 
#define W25X_FastReadDual		            0x3B 
#define W25X_PageProgram		            0x02					//д�洢������ 
#define W25X_BlockErase			            0xD8 
#define W25X_SectorErase		            0x20 					//����һ������
#define W25X_ChipErase			            0xC7 					//������Ƭ
#define W25X_PowerDown			            0xB9					//�������ģʽ 
#define W25X_ReleasePowerDown	          0xAB					//����flash  
#define W25X_DeviceID			              0xAB 					//��ȡflash�豸ID
#define W25X_ManufactDeviceID   	      0x90 
#define W25X_JedecDeviceID		          0x9F					//�������ͺŵ�ID 

#define WIP_Flag                        0x01  /* Write In Progress (WIP) flag */

#define  FLASH_WriteAddress     0x0000
#define  FLASH_ReadAddress      FLASH_WriteAddress
#define  FLASH_SectorToErase    FLASH_WriteAddress

#define  FLASH_RechargeBase           0x100000  //��Ƭ��Ϣ�Ļ���ַ 1M ��λ��             0 1 0 0 0 0 0 0 
#define  FLASH_LogIndexBase           0x200000  //��־������ 2M  ��ʼ                    0 0 1 1 0 0 0 0  û�и����������ݵ���־
#define  FLASH_CookingSessionBase     0x400000  //������Ϣ�Ļ���ַ 4M ��λ��             0 0 0 0 1 0 0 0
#define  FLASH_LogBase                0x500000  //��־�����������ݵĴ� 5M ��ʼ           0 0 0 0 0 1 1 0 
#define  FLASH_VoltageBase            0x700000  //��־������ 7M ��ʼ                     0 0 0 0 0 0 0 1

/* ���Ͷ��� ------------------------------------------------------------------*/
/* �궨�� --------------------------------------------------------------------*/
//#define  SPI_FLASH_ID                       0xEF3015     //W25X16   2MB
//#define  SPI_FLASH_ID                       0xEF4015	   //W25Q16   4MB
#define  SPI_FLASH_ID                       0XEF4017     //W25Q64   8MB
//#define  SPI_FLASH_ID                       0XEF4018     //W25Q128  16MB YS-F1Pro����Ĭ��ʹ��

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

/* �������� ------------------------------------------------------------------*/

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
