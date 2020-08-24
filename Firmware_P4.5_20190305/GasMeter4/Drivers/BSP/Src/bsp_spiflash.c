/**
  ******************************************************************************
  * �ļ�����: bsp_spiflash.c 
  * ��    ��: ӲʯǶ��ʽ�����Ŷ�
  * ��    ��: V1.0
  * ��д����: 2015-10-04
  * ��    ��: ���ش���Flash�ײ�����ʵ��
  ******************************************************************************
  * ˵����
  * ����������Ӳʯstm32������YS-F1Proʹ�á�
  * 
  * �Ա���
  * ��̳��http://www.ing10bbs.com
  * ��Ȩ��ӲʯǶ��ʽ�����Ŷ����У��������á�
  ******************************************************************************
  */

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "bsp.h"


/* ˽�б��� ------------------------------------------------------------------*/
//SPI_HandleTypeDef hspiflash;

TestStatus TransferStatus1 = FAILED;
/* ��չ���� ------------------------------------------------------------------*/
/* ˽�к���ԭ�� --------------------------------------------------------------*/
/* ������ --------------------------------------------------------------------*/

/**
  * ��������: ����FLASH��ʼ��,Ĭ�Ϲ�����ʵ��
  * �������: huart�����ھ������ָ��
  * �� �� ֵ: ��
  * ˵    ��: �ú�����HAL���ڲ�����
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
  * ��������: SPI����ϵͳ����ʼ��,Ĭ�Ϲ�����ʵ��
  * �������: hspi��SPI�������ָ��
  * �� �� ֵ: ��
  * ˵    ��: �ú�����HAL���ڲ�����
  */
//void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
//{
//  GPIO_InitTypeDef GPIO_InitStruct;
//  if(hspi->Instance==FLASH_SPIx)
//  {
//    /* SPI����ʱ��ʹ�� */
//    FLASH_SPIx_RCC_CLK_ENABLE();
//    /* GPIO����ʱ��ʹ�� */
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
  * ��������: SPI����ϵͳ������ʼ��,���˿ڸ�λ����ʼ״̬,Ĭ�Ϲ�����ʵ��
  * �������: hspi��SPI�������ָ��
  * �� �� ֵ: ��
  * ˵    ��: �ú�����HAL���ڲ�����
  */
//void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
//{

//  if(hspi->Instance==FLASH_SPIx)
//  {
//    /* SPI����ʱ�ӽ��� */
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
  * ��������: �Ӵ���Flash��ȡһ���ֽ�����
  * �������: ��
  * �� �� ֵ: uint8_t����ȡ��������
  * ˵    ����This function must be used only if the Start_Read_Sequence
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
  * ��������: ������Flashд��һ���ֽ����ݲ�����һ���ֽ�����
  * �������: byte������������
  * �� �� ֵ: uint8_t�����յ�������
  * ˵    ������
  */
uint8_t SPI_FLASH_SendByte(uint8_t byte)
{
  uint8_t d_read,d_send=byte;
  if(HAL_SPI_TransmitReceive(&hspi2,&d_send,&d_read,1,0xFFFFFF)!=HAL_OK)
    d_read=Dummy_Byte;
  
  return d_read; 
}   
   
/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/

