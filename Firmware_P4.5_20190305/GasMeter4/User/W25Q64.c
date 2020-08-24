#include "bsp.h"

/* ˽�б��� ------------------------------------------------------------------*/
uint8_t Tx_Buffer_W25Q64[] = " ��л��ѡ��Ӳʯstm32������\n�����Ǹ�������";
uint8_t Rx_Buffer_W25Q64[BufferSize2(Tx_Buffer_W25Q64)];
extern TestStatus TransferStatus1;

/**
  *�����ڴ���
  */
void TestW25Q64(void)
{
	uint32_t DeviceID = 0;
  uint32_t FlashID = 0;
	
	
	HAL_Delay(100);
	
	DeviceID = SPI_FLASH_ReadDeviceID();
  
  HAL_Delay(100);
  
  /* Get SPI Flash Type ID */
	FlashID = SPI_FLASH_ReadID();
  
  printf("FlashID is 0x%X,  Manufacturer Device ID is 0x%X\n", DeviceID,FlashID);
	
	/* Check the SPI Flash ID */
	if (FlashID == SPI_FLASH_ID)  /* #define  sFLASH_ID  0XEF4018 */
	{	
		printf("��⵽�����flash W25Q64 !\n");
		HAL_Delay(500);
		/* ����SPI��������д�� */
		SPI_FLASH_SectorErase(FLASH_SectorToErase);	 	 
		
		/* �����ͻ�����������д��flash�� */ 	
		SPI_FLASH_BufferWrite(Tx_Buffer_W25Q64, FLASH_WriteAddress, BufferSize2(Tx_Buffer_W25Q64));
		//SPI_FLASH_BufferWrite(Tx_Buffer_W25Q64, 252,  BufferSize2(Tx_Buffer_W25Q64));
		printf("д�������Ϊ��\n%s \n", Tx_Buffer_W25Q64);
		//DisplayChar(1,1,"--WRITHE OK--");
		/* ���ո�д������ݶ������ŵ����ջ������� */
		SPI_FLASH_BufferRead(Rx_Buffer_W25Q64, FLASH_ReadAddress,  BufferSize2(Tx_Buffer_W25Q64));
		printf("����������Ϊ��\n %s\n", Rx_Buffer_W25Q64);
		//DisplayChar(2,1,"--READ OK--");
		/* ���д�������������������Ƿ���� */
		TransferStatus1 = Buffercmp(Tx_Buffer_W25Q64, Rx_Buffer_W25Q64,  BufferSize2(Tx_Buffer_W25Q64));
		HAL_Delay(500);
		if( PASSED == TransferStatus1 )
		{    
			printf("8M����flash(W25Q64)���Գɹ�!\r");
			//DisplayChar(2,1,"--Test OK--");
		}
		else
		{        
			printf("8M����flash(W25Q64)����ʧ��!\r");
			//DisplayChar(2,1,"--Test Err--");
		}
	}
	else
	{    
		printf("��ȡ���� W25Q64 ID!\n");
		//DisplayChar(1,1,"--Check Err--");
	}
}

/**
  * ��������: ��������
  * �������: SectorAddr��������������ַ��Ҫ��Ϊ4096����
  * �� �� ֵ: ��
  * ˵    ��������Flash��С�������СΪ4KB(4096�ֽ�)����һ��������С��Ҫ���������
  *           Ϊ4096��������������FlashоƬд������֮ǰҪ���Ȳ����ռ䡣
  */
void SPI_FLASH_SectorErase(uint32_t SectorAddr)
{
  /* ����FLASHдʹ������ */
  SPI_FLASH_WriteEnable();
  SPI_FLASH_WaitForWriteEnd();
  /* �������� */
  /* ѡ����FLASH: CS�͵�ƽ */
  FLASH_SPI_CS_ENABLE;
  /* ������������ָ��*/
  SPI_FLASH_SendByte(W25X_SectorErase);
  /*���Ͳ���������ַ�ĸ�λ*/
  SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
  /* ���Ͳ���������ַ����λ */
  SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
  /* ���Ͳ���������ַ�ĵ�λ */
  SPI_FLASH_SendByte(SectorAddr & 0xFF);
  /* ���ô���FLASH: CS �ߵ�ƽ */
  FLASH_SPI_CS_DISABLE;
  /* �ȴ��������*/
  SPI_FLASH_WaitForWriteEnd();
}

/**
  * ��������: ������Ƭ
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������������Flash��Ƭ�ռ�
  */
void SPI_FLASH_BulkErase(void)
{
 /* ����FLASHдʹ������ */
  SPI_FLASH_WriteEnable();
  SPI_FLASH_WaitForWriteEnd();
  /* ��Ƭ���� Erase */
  /* ѡ����FLASH: CS�͵�ƽ */
  FLASH_SPI_CS_ENABLE;
  /* ������Ƭ����ָ��*/
  SPI_FLASH_SendByte(W25X_ChipErase);
  /* ���ô���FLASH: CS�ߵ�ƽ */
  FLASH_SPI_CS_DISABLE;

  /* �ȴ��������*/
  SPI_FLASH_WaitForWriteEnd();
}

/**
  * ��������: ������FLASH��ҳд�����ݣ����ñ�����д������ǰ��Ҫ�Ȳ�������
  * �������: pBuffer����д�����ݵ�ָ��
  *           WriteAddr��д���ַ
  *           NumByteToWrite��д�����ݳ��ȣ�����С�ڵ���SPI_FLASH_PerWritePageSize
  * �� �� ֵ: ��
  * ˵    ��������Flashÿҳ��СΪ256���ֽ�
  */
void SPI_FLASH_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  /* ����FLASHдʹ������ */
  SPI_FLASH_WriteEnable();

  /* Ѱ�Ҵ���FLASH: CS�͵�ƽ */
  FLASH_SPI_CS_ENABLE;
  /* д��дָ��*/
  SPI_FLASH_SendByte(W25X_PageProgram);
  /*����д��ַ�ĸ�λ*/
  SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /*����д��ַ����λ*/
  SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);
  /*����д��ַ�ĵ�λ*/
  SPI_FLASH_SendByte(WriteAddr & 0xFF);

  if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
  {
     NumByteToWrite = SPI_FLASH_PerWritePageSize;
     //printf("Err: SPI_FLASH_PageWrite too large!\n");
  }

  /* д������*/
  while (NumByteToWrite--)
  {
     /* ���͵�ǰҪд����ֽ����� */
    SPI_FLASH_SendByte(*pBuffer);
     /* ָ����һ�ֽ����� */
    pBuffer++;
  }

  /* ���ô���FLASH: CS �ߵ�ƽ */
  FLASH_SPI_CS_DISABLE;

  /* �ȴ�д�����*/
  SPI_FLASH_WaitForWriteEnd();
}

/**
  * ��������: ������FLASHд�����ݣ����ñ�����д������ǰ��Ҫ�Ȳ�������
  * �������: pBuffer����д�����ݵ�ָ��
  *           WriteAddr��д���ַ
  *           NumByteToWrite��д�����ݳ���
  * �� �� ֵ: ��
  * ˵    �����ú���������������д�����ݳ���
  */
void SPI_FLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

  Addr = WriteAddr % SPI_FLASH_PageSize;
  count = SPI_FLASH_PageSize - Addr;
  NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
  NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

  if (Addr == 0) /* ����ַ�� SPI_FLASH_PageSize ����  */
  {
    if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
    {
      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
      while (NumOfPage--)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }

      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else /* ����ַ�� SPI_FLASH_PageSize ������ */
  {
    if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
    {
      if (NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PageSize */
      {
        temp = NumOfSingle - count;

        SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;

        SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
      }
      else
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
      NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

      SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;

      while (NumOfPage--)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }

      if (NumOfSingle != 0)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

/**
  * ��������: �Ӵ���Flash��ȡ����
  * �������: pBuffer����Ŷ�ȡ�����ݵ�ָ��
  *           ReadAddr����ȡ����Ŀ���ַ
  *           NumByteToRead����ȡ���ݳ���
  * �� �� ֵ: ��
  * ˵    �����ú����������������ȡ���ݳ���
  */
void SPI_FLASH_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
	
  /* ѡ����FLASH: CS�͵�ƽ */
  FLASH_SPI_CS_ENABLE;

  /* ���� �� ָ�� */
  SPI_FLASH_SendByte(W25X_ReadData);

  /* ���� �� ��ַ��λ */
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* ���� �� ��ַ��λ */
  SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* ���� �� ��ַ��λ */
  SPI_FLASH_SendByte(ReadAddr & 0xFF);

  while (NumByteToRead--) /* ��ȡ���� */
  {
     /* ��ȡһ���ֽ�*/
    *pBuffer = SPI_FLASH_SendByte(Dummy_Byte);
    /* ָ����һ���ֽڻ����� */
    pBuffer++;
  }

  /* ���ô���FLASH: CS �ߵ�ƽ */
  FLASH_SPI_CS_DISABLE;
}

//дSPI FLASH  
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ú�������������!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)   
uint8_t W25Q64_BUFFER[4096];  //�ȶ���һ��Buffer�����洢��������һ������������ݡ�

void W25Q64_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)  //��ڲ�����Buffer�ĵ�ַָ�룬                                                                                                                        Ҫд�ĵ�ַ�����ݡ� 
{ 
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;   
	uint16_t i;    
	uint8_t * W25QXX_BUF;  
	W25QXX_BUF = W25Q64_BUFFER;  
   
  secpos = WriteAddr / 4096;//�����������ַ 
  secoff = WriteAddr % 4096;//ȡ����������������ڵ�ƫ��
  secremain = 4096 - secoff;//����ʣ��ռ��С 	 
 
  if(NumByteToWrite<=secremain)
	{
		secremain=NumByteToWrite;//���Ҫд�����ݲ�����4096���ֽڣ�û�п�����   	
	}		
	while(1) 
	{
		SPI_FLASH_BufferRead(W25QXX_BUF,secpos*4096,4096);//�����������������ݣ�������Buffer�С�

		for(i=0; i < secremain;i++)  //У������
		{
			if(W25QXX_BUF[secoff+i]!=0XFF)
			{
				break;//����в�����0xFF�����ݣ�����Ҫ����  
			}				
		}

		if(i < secremain)//��Ҫ����
		{
			SPI_FLASH_SectorErase(secpos * 4096); //�����������
			for(i=0;i<secremain;i++)   //���»����е�����
			{
				W25QXX_BUF[secoff + i]=pBuffer[i];  //�����pBuffer��������Ҫд�����ݣ�����Щ���ݸ��µ������ж�Ӧ��                                                               λ��
			}		
			SPI_FLASH_BufferWrite(W25QXX_BUF,secpos*4096,4096);//����д����������  
		}
		else
		{		
			SPI_FLASH_BufferWrite(pBuffer,WriteAddr,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������.
		}	
		if(NumByteToWrite==secremain)
		{
			break;//д�������
		}
		else//д��δ����
		{
			secpos++;//������ַ��1
			secoff=0;//ƫ��λ��Ϊ0  

			pBuffer+=secremain;   //ָ��ƫ��
			WriteAddr+=secremain; //д��ַƫ��   
			NumByteToWrite-=secremain; //�ֽ����ݼ�
			if(NumByteToWrite>4096)
			{
				secremain=4096;//��һ����������д����
			}
			else 
			{
				secremain=NumByteToWrite; //��һ����������д����
			}
		}  
	}  
}

/**
  * ��������: ��ȡ����Flash�ͺŵ�ID
  * �������: ��
  * �� �� ֵ: uint32_t������Flash���ͺ�ID
  * ˵    ����  FLASH_ID      IC�ͺ�      �洢�ռ��С         
                0xEF3015      W25X16        2M byte
                0xEF4015	    W25Q16        4M byte
                0XEF4017      W25Q64        8M byte
                0XEF4018      W25Q128       16M byte  (YS-F1Pro������Ĭ������)
  */
uint32_t SPI_FLASH_ReadID(void)
{
  uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

  /* ѡ����FLASH: CS�͵�ƽ */
  FLASH_SPI_CS_ENABLE;

  /* ���������ȡоƬ�ͺ�ID */
  SPI_FLASH_SendByte(W25X_JedecDeviceID);

  /* �Ӵ���Flash��ȡһ���ֽ����� */
  Temp0 = SPI_FLASH_SendByte(Dummy_Byte);

  /* �Ӵ���Flash��ȡһ���ֽ����� */
  Temp1 = SPI_FLASH_SendByte(Dummy_Byte);

  /* �Ӵ���Flash��ȡһ���ֽ����� */
  Temp2 = SPI_FLASH_SendByte(Dummy_Byte);

  /* ���ô���Flash��CS�ߵ�ƽ */
  FLASH_SPI_CS_DISABLE;
  
  Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
  return Temp;
}

/**
  * ��������: ��ȡ��
  * �������: ��
  * �� �� ֵ: uint32_t������Flash���豸ID
  * ˵    ����
  */
uint32_t SPI_FLASH_ReadDeviceID(void)
{
  uint32_t Temp = 0;

  /* ѡ����FLASH: CS�͵�ƽ */
  FLASH_SPI_CS_ENABLE;

  /* ���������ȡоƬ�豸ID * */
  SPI_FLASH_SendByte(W25X_DeviceID);
  SPI_FLASH_SendByte(Dummy_Byte);
  SPI_FLASH_SendByte(Dummy_Byte);
  SPI_FLASH_SendByte(Dummy_Byte);
  
  /* �Ӵ���Flash��ȡһ���ֽ����� */
  Temp = SPI_FLASH_SendByte(Dummy_Byte);

  /* ���ô���Flash��CS�ߵ�ƽ */
  FLASH_SPI_CS_DISABLE;

  return Temp;
}

/**
  * ��������: ����������ȡ���ݴ�
  * �������: ReadAddr����ȡ��ַ
  * �� �� ֵ: ��
  * ˵    ����Initiates a read data byte (READ) sequence from the Flash.
  *           This is done by driving the /CS line low to select the device,
  *           then the READ instruction is transmitted followed by 3 bytes
  *           address. This function exit and keep the /CS line low, so the
  *           Flash still being selected. With this technique the whole
  *           content of the Flash is read with a single READ instruction.
  */
void SPI_FLASH_StartReadSequence(uint32_t ReadAddr)
{
  /* Select the FLASH: Chip Select low */
  FLASH_SPI_CS_ENABLE;

  /* Send "Read from Memory " instruction */
  SPI_FLASH_SendByte(W25X_ReadData);

  /* Send the 24-bit address of the address to read from -----------------------*/
  /* Send ReadAddr high nibble address byte */
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte */
  SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte */
  SPI_FLASH_SendByte(ReadAddr & 0xFF);
}

/**
  * ��������: ʹ�ܴ���Flashд����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
void SPI_FLASH_WriteEnable(void)
{
  /* ѡ����FLASH: CS�͵�ƽ */
  FLASH_SPI_CS_ENABLE;

  /* �������дʹ�� */
  SPI_FLASH_SendByte(W25X_WriteEnable);

  /* ���ô���Flash��CS�ߵ�ƽ */
  FLASH_SPI_CS_DISABLE;
}

/**
  * ��������: �ȴ�����д�����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ����Polls the status of the Write In Progress (WIP) flag in the
  *           FLASH's status  register  and  loop  until write  opertaion
  *           has completed.
  */
void SPI_FLASH_WaitForWriteEnd(void)
{
  uint8_t FLASH_Status = 0; //д�������һλ��1,�����0��ʱ��д�����

  /* Select the FLASH: Chip Select low */
  FLASH_SPI_CS_ENABLE;

  /* Send "Read Status Register" instruction */
  SPI_FLASH_SendByte(W25X_ReadStatusReg);

  /* Loop as long as the memory is busy with a write cycle */
  do
  {
    /* Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    FLASH_Status = SPI_FLASH_SendByte(Dummy_Byte);	 
  }
  while ((FLASH_Status & WIP_Flag) == SET); /* Write in progress */

  /* Deselect the FLASH: Chip Select high */
  FLASH_SPI_CS_DISABLE;
}


/**
  * ��������: �������ģʽ
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
void SPI_Flash_PowerDown(void)   
{ 
  /* Select the FLASH: Chip Select low */
  FLASH_SPI_CS_ENABLE;

  /* Send "Power Down" instruction */
  SPI_FLASH_SendByte(W25X_PowerDown);

  /* Deselect the FLASH: Chip Select high */
  FLASH_SPI_CS_DISABLE;
}   

/**
  * ��������: ���Ѵ���Flash
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
void SPI_Flash_WAKEUP(void)   
{
  /* Select the FLASH: Chip Select low */
  FLASH_SPI_CS_ENABLE;

  /* Send "Power Down" instruction */
  SPI_FLASH_SendByte(W25X_ReleasePowerDown);

  /* Deselect the FLASH: Chip Select high */
  FLASH_SPI_CS_DISABLE; 
}
