/**
  ******************************************************************************
  * 文件名程: bsp_spiflash.c 
  * 作    者: 硬石嵌入式开发团队
  * 版    本: V1.0
  * 编写日期: 2015-10-04
  * 功    能: 板载串行Flash底层驱动实现
  ******************************************************************************
  * 说明：
  * 本例程配套硬石stm32开发板YS-F1Pro使用。
  * 
  * 淘宝：
  * 论坛：http://www.ing10bbs.com
  * 版权归硬石嵌入式开发团队所有，请勿商用。
  ******************************************************************************
  */

/* 包含头文件 ----------------------------------------------------------------*/
#include "bsp.h"


/* 私有变量 ------------------------------------------------------------------*/
//SPI_HandleTypeDef hspiflash;

TestStatus TransferStatus1 = FAILED;
/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原形 --------------------------------------------------------------*/
/* 函数体 --------------------------------------------------------------------*/

/**
  * 函数功能: 串行FLASH初始化,默认工具有实现
  * 输入参数: huart：串口句柄类型指针
  * 返 回 值: 无
  * 说    明: 该函数被HAL库内部调用
*/
//void MX_SPIFlash_Init(void)
//{
//  hspiflash.Instance = FLASH_SPIx;
//  hspiflash.Init.Mode = SPI_MODE_MASTER;
//  hspiflash.Init.Direction = SPI_DIRECTION_2LINES;
//  hspiflash.Init.DataSize = SPI_DATASIZE_8BIT;
//  hspiflash.Init.CLKPolarity = SPI_POLARITY_HIGH;
//  hspiflash.Init.CLKPhase = SPI_PHASE_2EDGE;
//  hspiflash.Init.NSS = SPI_NSS_SOFT;
//  hspiflash.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
//  hspiflash.Init.FirstBit = SPI_FIRSTBIT_MSB;
//  hspiflash.Init.TIMode = SPI_TIMODE_DISABLE;
//  hspiflash.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//  hspiflash.Init.CRCPolynomial = 10;
//	if (HAL_SPI_Init(&hspiflash) != HAL_OK)
//  {
//    _Error_Handler(__FILE__, __LINE__);
//  }
//}

/**
  * 函数功能: SPI外设系统级初始化,默认工具有实现
  * 输入参数: hspi：SPI句柄类型指针
  * 返 回 值: 无
  * 说    明: 该函数被HAL库内部调用
  */
//void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
//{
//  GPIO_InitTypeDef GPIO_InitStruct;
//  if(hspi->Instance==FLASH_SPIx)
//  {
//    /* SPI外设时钟使能 */
//    FLASH_SPIx_RCC_CLK_ENABLE();
//    /* GPIO外设时钟使能 */
//    FLASH_SPI_GPIO_ClK_ENABLE();
//    FLASH_SPI_CS_CLK_ENABLE();
//    
//    /**SPI1 GPIO Configuration    
//    PA4     ------> SPI1_NSS
//    PA5     ------> SPI1_SCK
//    PA6     ------> SPI1_MISO
//    PA7     ------> SPI1_MOSI 
//    */
//		GPIO_InitStruct.Pin = FLASH_SPI_SCK_PIN|FLASH_SPI_MOSI_PIN|FLASH_SPI_MISO_PIN;
//		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//		GPIO_InitStruct.Pull = GPIO_NOPULL;
//		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//		GPIO_InitStruct.Alternate = GPIO_AF0_SPI2;
//		HAL_GPIO_Init(FLASH_SPI_GPIO_PORT, &GPIO_InitStruct);
//    
//    HAL_GPIO_WritePin(FLASH_SPI_CS_PORT, FLASH_SPI_CS_PIN, GPIO_PIN_SET);
//    GPIO_InitStruct.Pin = FLASH_SPI_CS_PIN;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    HAL_GPIO_Init(FLASH_SPI_CS_PORT, &GPIO_InitStruct);
//  }
//}

/**
  * 函数功能: SPI外设系统级反初始化,将端口复位到初始状态,默认工具有实现
  * 输入参数: hspi：SPI句柄类型指针
  * 返 回 值: 无
  * 说    明: 该函数被HAL库内部调用
  */
//void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
//{

//  if(hspi->Instance==FLASH_SPIx)
//  {
//    /* SPI外设时钟禁用 */
//    FLASH_SPIx_RCC_CLK_DISABLE();
//  
//    /**SPI1 GPIO Configuration    
//    PA4     ------> SPI1_NSS
//    PA5     ------> SPI1_SCK
//    PA6     ------> SPI1_MISO
//    PA7     ------> SPI1_MOSI 
//    */
//    HAL_GPIO_DeInit(FLASH_SPI_GPIO_PORT, FLASH_SPI_SCK_PIN|FLASH_SPI_MOSI_PIN|FLASH_SPI_MISO_PIN);
//    HAL_GPIO_DeInit(FLASH_SPI_CS_PORT, FLASH_SPI_CS_PIN);
//  }
//} 


/**
  * 函数功能: 从串行Flash读取一个字节数据
  * 输入参数: 无
  * 返 回 值: uint8_t：读取到的数据
  * 说    明：This function must be used only if the Start_Read_Sequence
  *           function has been previously called.
  */
uint8_t SPI_FLASH_ReadByte(void)
{
  uint8_t d_read,d_send=Dummy_Byte;
  if(HAL_SPI_TransmitReceive(&hspi2,&d_send,&d_read,1,0xFFFFFF)!=HAL_OK)
    d_read=Dummy_Byte;
  
  return d_read;    
}

/**
  * 函数功能: 往串行Flash写入一个字节数据并接收一个字节数据
  * 输入参数: byte：待发送数据
  * 返 回 值: uint8_t：接收到的数据
  * 说    明：无
  */
uint8_t SPI_FLASH_SendByte(uint8_t byte)
{
  uint8_t d_read,d_send=byte;
  if(HAL_SPI_TransmitReceive(&hspi2,&d_send,&d_read,1,0xFFFFFF)!=HAL_OK)
    d_read=Dummy_Byte;
  
  return d_read; 
}   
   
/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/

