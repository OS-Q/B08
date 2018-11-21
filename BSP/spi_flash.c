#include "spi_flash.h"
#include "string.h"
#include "timer2.h"
#include "delay.h"
#include "app_cfg.h"
#include "flash_task.h"

uint16_t SPI_FLASH_TYPE;//读 flash 芯片型号；

void spi_gpio_write_byte(uint8_t dat)
{
	uint8_t i;
	
	CLR_SPI_FLASH_SCK;
	for (i=0; i<8; i++)
	{
		if ((dat&0x80)==0x80)	 
		{			
			SET_SPI_FLASH_MOSI;
		}
		else
		{
			CLR_SPI_FLASH_MOSI;
		}
		
		SET_SPI_FLASH_SCK;		 
		dat <<=1;
		CLR_SPI_FLASH_SCK;		 
	}
}

uint8_t spi_gpio_read_byte(void)
{
	uint8_t i, in=0;
	
	CLR_SPI_FLASH_SCK;
	for (i=0; i<8; i++)
	{
		in <<= 1;
		SET_SPI_FLASH_SCK;
		if (GET_SPI_FLASH_MISO==1)			 
			in |=1;		 
		CLR_SPI_FLASH_SCK;
	}
	
	return in;
}

uint8_t SPI_Flash_ReadSR(void)   
{  
	uint8_t byte=0;
	
	CLR_SPI_FLASH_CS;
	spi_gpio_write_byte(W25X_ReadStatusReg);//发送读取状态寄存器命令    
	byte=spi_gpio_read_byte();  
	SET_SPI_FLASH_CS;
	
	return byte;   
} 

void SPI_Flash_Wait_Busy(void)   
{   
	while ((SPI_Flash_ReadSR()&0x01)==0x01);   // 等待BUSY位清空
}

void SPI_Flash_PowerDown(void)   
{ 
	CLR_SPI_FLASH_CS;
	spi_gpio_write_byte(W25X_PowerDown);//发送掉电命令  
	SET_SPI_FLASH_CS;	      
}   

void SPI_Flash_WAKEUP(void)   
{  
	CLR_SPI_FLASH_CS;
	spi_gpio_write_byte(W25X_ReleasePowerDown);   
	SET_SPI_FLASH_CS; 
}

void SPI_FLASH_Write_Enable(void)   
{
	CLR_SPI_FLASH_CS;
	spi_gpio_write_byte(W25X_WriteEnable);//发送写使能  
	SET_SPI_FLASH_CS;

	while ((SPI_Flash_ReadSR()&0x02)!=0x02); 
} 
 
void SPI_FLASH_Write_Disable(void)   
{  
	CLR_SPI_FLASH_CS;
	spi_gpio_write_byte(W25X_WriteDisable);//发送写禁止指令    
	SET_SPI_FLASH_CS;

	while ((SPI_Flash_ReadSR()&0x02)==0x02); 
}

void SPI_FLASH_Write_SR(uint8_t sr)   
{
	SPI_FLASH_Write_Enable();
	CLR_SPI_FLASH_CS;
	spi_gpio_write_byte(W25X_WriteStatusReg);//发送写取状态寄存器命令    
	spi_gpio_write_byte(sr);
	SET_SPI_FLASH_CS;

	SPI_Flash_Wait_Busy();
	SPI_FLASH_Write_Disable();
}  

uint16_t SPI_Flash_ReadID(void)
{
	uint16_t tmp = 0;
	
	CLR_SPI_FLASH_CS;				    
	spi_gpio_write_byte(W25X_ManufactDeviceID);//发送读取ID命令	    
	spi_gpio_write_byte(0x00); 	    
	spi_gpio_write_byte(0x00); 	    
	spi_gpio_write_byte(0x00); 	 			   
	tmp |=spi_gpio_read_byte()<<8;  
	tmp |=spi_gpio_read_byte();	 
	SET_SPI_FLASH_CS;
	
	return tmp;
}

uint8_t devUniqueID[8];
void SPI_Flash_Read_Unique_ID(void)
{
	uint8_t i;
	
	CLR_SPI_FLASH_CS;				    
	spi_gpio_write_byte(W25X_Unique_ID);	    
	spi_gpio_write_byte(0x00);
	spi_gpio_write_byte(0x00);
	spi_gpio_write_byte(0x00);
	spi_gpio_write_byte(0x00);
	
	for(i=0;i<8;i++)
		devUniqueID[i]=spi_gpio_read_byte();

	SET_SPI_FLASH_CS;
	DBG_INFO("devID=");
	for(i=0;i<8;i++)
		DBG_INFO("%02X",devUniqueID[i]);
	DBG_INFO("\n");
}

void spiflash_read(uint32_t dest,void *buf,uint16_t len)
{
	uint8_t *p=buf;
	uint16_t i=0;

	if(p == NULL)
		return;

	CLR_SPI_FLASH_CS;
	spi_gpio_write_byte(W25X_ReadData); //发送读取命令   
	spi_gpio_write_byte((dest>>16)&0xFF);  
	spi_gpio_write_byte((dest>>8)&0xFF);   
	spi_gpio_write_byte(dest&0xFF);   
	for(i=0;i<len;i++)
	{ 
		p[i]=spi_gpio_read_byte();   //循环读
	}
	SET_SPI_FLASH_CS;
}

void spiflash_write(uint32_t dest,void *buf, uint16_t len)
{
	uint8_t *p=buf;
	uint16_t i=0,page,j,lastlen,wrlen;
	uint32_t destL;

	if(p == NULL)
		return;
	if((SPI_FLASH_TYPE&0xFF00)==0XBF00)
	{
		for(i=0;i<len;)
		{
			SPI_FLASH_Write_Enable();
			CLR_SPI_FLASH_CS; 
			spi_gpio_write_byte(W25X_PageProgram_AAI); //W25X_PageProgram
			spi_gpio_write_byte((dest>>16)&0xFF);
			spi_gpio_write_byte((dest>>8)&0xFF);
			spi_gpio_write_byte(dest&0xFF);
			spi_gpio_write_byte(p[i++]);
			dest++;
			if(i<len)
			{
				spi_gpio_write_byte(p[i++]);
				dest++;
			}
			SET_SPI_FLASH_CS;

			SPI_Flash_Wait_Busy(); 
			SPI_FLASH_Write_Disable();
		}
	}
	else
	{
		page=len/256;
		lastlen=len%256;
		if(lastlen>0)
			page++;

		for(j=0;j<page;j++)
		{
			SPI_FLASH_Write_Enable();
			wrlen=256;
			if(j==(page-1))
			{
				if(lastlen>0)
					wrlen=lastlen;
			}
			destL=dest+j*256;
			
			CLR_SPI_FLASH_CS;
			spi_gpio_write_byte(W25X_PageProgram);      //发送写命令   
			spi_gpio_write_byte((destL>>16)&0xFF);  
			spi_gpio_write_byte((destL>>8)&0xFF);   
			spi_gpio_write_byte(destL&0xFF);   
			for(i=0;i<wrlen;i++)
				spi_gpio_write_byte(p[i+j*256]);//循环写数  
			SET_SPI_FLASH_CS;
			SPI_Flash_Wait_Busy();
			SPI_FLASH_Write_Disable();
		}
	}
}

//擦除一个扇区
//Dst_Addr:扇区地址 0~511 for w25x16
//擦除一个山区的最少时间:150ms
void spiflash_sector4k_erase(uint32_t dest) 
{
	dest *= 4096;
  SPI_FLASH_Write_Enable();
	
	SPI_Flash_Wait_Busy();   
	CLR_SPI_FLASH_CS;  
	spi_gpio_write_byte(W25X_SectorErase);      //发送扇区擦除指令 
	spi_gpio_write_byte((dest>>16)&0xFF);
	spi_gpio_write_byte((dest>>8)&0xFF);   
	spi_gpio_write_byte(dest&0xFF);  
	SET_SPI_FLASH_CS;     
	SPI_Flash_Wait_Busy();   				   //等待擦除完成
	
	SPI_FLASH_Write_Disable();
}

void spiflash_sector64k_erase(uint32_t dest)
{
	SPI_FLASH_Write_Enable();
	
	SPI_Flash_Wait_Busy();   
	CLR_SPI_FLASH_CS;  
	spi_gpio_write_byte(W25X_BlockErase);      //发送扇区擦除指令 
	spi_gpio_write_byte((dest>>16)&0xFF);
	spi_gpio_write_byte((dest>>8)&0xFF);   
	spi_gpio_write_byte(dest&0xFF);  
	SET_SPI_FLASH_CS;     
	SPI_Flash_Wait_Busy();   				   //等待擦除完成
	
	SPI_FLASH_Write_Disable();
}

void spiflash_sector_all_erase(void)
{
	SPI_FLASH_Write_Enable();
	SPI_Flash_Wait_Busy();  
	
	CLR_SPI_FLASH_CS;
	spi_gpio_write_byte(W25X_ChipErase);// 擦除命令  
	SET_SPI_FLASH_CS;      
	SPI_Flash_Wait_Busy();   				   //等待擦除结束
	
	SPI_FLASH_Write_Disable();
}

//SPI在一页(0~65535)内写入少于256个字节的数据,在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区;WriteAddr:开始写入的地址(24bit);NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
void SPI_Flash_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
 	u16 i;  
  SPI_FLASH_Write_Enable();                  //SET WEL 
	CLR_SPI_FLASH_CS;                          //使能器件   
  spi_gpio_write_byte(W25X_PageProgram);     //发送写页命令   
  spi_gpio_write_byte((u8)((WriteAddr)>>16)); //发送24bit地址    
  spi_gpio_write_byte((u8)((WriteAddr)>>8));   
  spi_gpio_write_byte((u8)WriteAddr);   
  for(i=0;i<NumByteToWrite;i++)
    spi_gpio_write_byte(pBuffer[i]);//循环写数  
	SET_SPI_FLASH_CS;                 //取消片选 
	SPI_Flash_Wait_Busy();					   //等待写入结束
  SPI_FLASH_Write_Disable();
} 

//无检验写SPI FLASH,在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区,WriteAddr:开始写入的地址(24bit),NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void SPI_Flash_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; //单页剩余的字节数		 	    
	if(NumByteToWrite<=pageremain)
    pageremain=NumByteToWrite;//不大于256个字节
	while(1)
	{	   
		SPI_Flash_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)
      break;
	 	else
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			//减去已经写入了的字节数
			if(NumByteToWrite>256)
        pageremain=256;               //一次可以写入256个字节
			else 
        pageremain=NumByteToWrite; 	  //不够256个字节了
		}
	}	    
} 

//读取SPI FLASH ,在指定地址开始读取指定长度的数据
//pBuffer:数据存储区,ReadAddr:开始读取的地址(24bit),NumByteToRead:要读取的字节数(最大65535)
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
 	u16 i;    												    
	CLR_SPI_FLASH_CS;                           //使能器件   
  spi_gpio_write_byte(W25X_ReadData);        //发送读取命令   
  spi_gpio_write_byte((u8)((ReadAddr)>>16)); //发送24bit地址    
  spi_gpio_write_byte((u8)((ReadAddr)>>8));   
  spi_gpio_write_byte((u8)ReadAddr);   
  for(i=0;i<NumByteToRead;i++)
	{ 
    pBuffer[i]=spi_gpio_read_byte();   //循环读数  
  }
	SET_SPI_FLASH_CS;                    //取消片选
}  
 
//在指定地址开始写入指定长度的数据,该函数带擦除操作!
//pBuffer:数据存储区,WriteAddr:开始写入的地址(24bit),NumByteToWrite:要写入的字节数(最大65535)
u8 SPI_FLASH_BUF[4096];
void SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff,secremain,i;
	 
	secpos=WriteAddr/4096;//扇区地址 0~511 for w25x16
	secoff=WriteAddr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小   
	
	if(NumByteToWrite<=secremain)
    secremain=NumByteToWrite;//不大于4096个字节
  
	while(1) 
	{
		SPI_Flash_Read(SPI_FLASH_BUF,secpos*4096,4096);//读出整个扇区的内容
		 
		for(i=0;i<secremain;i++)//校验数据
		{
			if(SPI_FLASH_BUF[secoff+i]!=0XFF)
        break;//需要擦除  	  
		}
    
		if(i<secremain)//需要擦除
		{
			spiflash_sector4k_erase(secpos);//擦除这个扇区
			for(i=0;i<secremain;i++)	   //复制
			{
				SPI_FLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			SPI_Flash_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);//写入整个扇区  
		}else{
      SPI_Flash_Write_NoCheck(pBuffer,WriteAddr,secremain);//写已经擦除了的,直接写入扇区剩余区间.
    }
    
		if(NumByteToWrite==secremain)
    {
      //写入结束了
      break;
    } else {
      //写入未结束
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 
		  pBuffer+=secremain;  //指针偏移
			WriteAddr+=secremain;//写地址偏移	   
		  NumByteToWrite-=secremain;				//字节数递减
			if(NumByteToWrite>4096)
        secremain=4096;	//下一个扇区还是写不完
			else 
        secremain=NumByteToWrite;			//下一个扇区可以写完了
		}
	} 	 
}

void SPI_Flash_Write_Str(flash_save_container* pBuffer,u8 length)   
{ 
	u32 secpos;
	u16 i,j;
  
	secpos=pBuffer[0].loc/4096;//扇区地址 0~511 for w25x16
	DBG_INFO("5 flash write data\n");
	SPI_Flash_Read(SPI_FLASH_BUF,secpos*4096,4096);//读出整个扇区的内容
  spiflash_sector4k_erase(secpos);//擦除这个扇区
  DBG_INFO("6 flash write data t\n");
  for(j = 0; j < length; j++)
  {
    DBG_INFO("5 flash write length = %d, loc = %d, data_len = %d\n",j,pBuffer[j].loc,pBuffer[j].data_len);
    for(i = 0; i < pBuffer[j].data_len; i++)	   //复制
    {
      SPI_FLASH_BUF[i+pBuffer[j].loc]=pBuffer[j].data_point[i];
      //DBG_INFO("5 flash write  = %d,  = %d\n",i+pBuffer[j].loc,pBuffer[j].data_point[i]);
    }
  }
  DBG_INFO("7 flash write data\n");
  SPI_Flash_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);//写入整个扇区   
  DBG_INFO("8 flash write data\n");
}

int Flash_Check(void)
{
	uint32_t writeData = FLASH_MAGIC_FLAG,readData;
  uint32_t address=1023*FLASH_BLK_SIZE;
	OS_ERR err;
  SPI_Flash_Read((uint8_t *)&readData,address,4);	
	if(readData==FLASH_MAGIC_FLAG)
    return 0;		   
	else//排除第一次初始化的情况
	{
		reset_flash();
		OSTimeDlyHMSM(0, 0, 0, 25,OS_OPT_TIME_HMSM_STRICT,&err);
		SPI_Flash_Write((uint8_t *)&writeData,address,4);
		OSTimeDlyHMSM(0, 0, 0, 25,OS_OPT_TIME_HMSM_STRICT,&err);
		SPI_Flash_Read((uint8_t *)&readData,address,4);  
		if(readData==FLASH_MAGIC_FLAG)
      return 0;
	}
	return -1;											  
}

int spiflash_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	CLR_SPI_FLASH_CS;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	SET_SPI_FLASH_CS;
	SET_SPI_FLASH_SCK;
	SET_SPI_FLASH_MOSI;
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_14;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	SPI_FLASH_Write_SR(0x00);
	
	SPI_FLASH_TYPE=SPI_Flash_ReadID();//读取FLASH ID. 
	if(((SPI_FLASH_TYPE&0xFF00)==0xFF00)||((SPI_FLASH_TYPE&0xFF00)==0x0000))
  {  
		DBG_ERROR("!!!!!!SPI_FLASH read err\n");
    return -1;
  }  
  
  DBG_INFO("Flash ID:%04X\n",SPI_FLASH_TYPE);
  return Flash_Check();
}

void flash_init_error(void)
{
  DBG_ERROR("!!!!!!flash read err\n");
	hw_delay_ms(2000);
  beep_start(2000);
  LED1(ON);
  hw_delay_ms(500);
  LED1(OFF);
  hw_delay_ms(500);
  beep_start(2000);
  LED1(ON);
  hw_delay_ms(500);
  LED1(OFF);
  hw_delay_ms(500);
  beep_start(2000);
  LED1(ON);
  hw_delay_ms(500);
  LED1(OFF);
  hw_delay_ms(500);
} 
