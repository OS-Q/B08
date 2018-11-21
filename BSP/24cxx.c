#include "24cxx.h"
#include "delay.h"
#include "app_cfg.h"
#include "debug.h"
#include "timer2.h"
#include "open_result_task.h"

//初始化IIC接口
int AT24CXX_Init(void)
{
	IIC_Init();
  return AT24CXX_Check();
}
//在AT24CXX指定地址读出一个数据
//ReadAddr:开始读数的地址  
//返回值  :读到的数据
uint8_t AT24CXX_ReadOneByte(uint16_t ReadAddr)
{				  
	uint8_t temp=0;		  	    																 
    IIC_Start();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	   //发送写命令
		IIC_Wait_Ack();
		IIC_Send_Byte(ReadAddr>>8);//发送高地址
		IIC_Wait_Ack();		 
	}else IIC_Send_Byte(0XA0+((ReadAddr/256)<<1));   //发送器件地址0XA0,写数据 	 

	IIC_Wait_Ack(); 
    IIC_Send_Byte(ReadAddr%256);   //发送低地址
	IIC_Wait_Ack();	    
	IIC_Start();  	 	   
	IIC_Send_Byte(0XA1);           //进入接收模式			   
	IIC_Wait_Ack();	 
    temp=IIC_Read_Byte(0);		   
    IIC_Stop();//产生一个停止条件	    
	return temp;
}
//在AT24CXX指定地址写入一个数据
//WriteAddr  :写入数据的目的地址    
//DataToWrite:要写入的数据
void AT24CXX_WriteOneByte(uint16_t WriteAddr,uint8_t DataToWrite)
{				   	  	    																 
	IIC_Start();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	    //发送写命令
		IIC_Wait_Ack();
		IIC_Send_Byte(WriteAddr>>8);//发送高地址
	}
	else
	{
		IIC_Send_Byte(0XA0+((WriteAddr/256)<<1));   //发送器件地址0XA0,写数据 
	}
	
	IIC_Wait_Ack();	   
	IIC_Send_Byte(WriteAddr%256);   //发送低地址
	IIC_Wait_Ack(); 	 										  		   
	IIC_Send_Byte(DataToWrite);     //发送字节							   
	IIC_Wait_Ack();  		    	   
	IIC_Stop();//产生一个停止条件 
	
	delay_ms(10);// 合适值需要自己测试
}
//在AT24CXX里面的指定地址开始写入长度为Len的数据
//该函数用于写入16bit或者32bit的数据.
//WriteAddr  :开始写入的地址  
//DataToWrite:数据数组首地址
//Len        :要写入数据的长度2,4
void AT24CXX_WriteLenByte(uint16_t WriteAddr,uint32_t DataToWrite,uint8_t Len)
{  	
	uint8_t t;
	for(t=0;t<Len;t++)
	{
		AT24CXX_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
	}												    
}

//在AT24CXX里面的指定地址开始读出长度为Len的数据
//该函数用于读出16bit或者32bit的数据.
//ReadAddr   :开始读出的地址 
//返回值     :数据
//Len        :要读出数据的长度2,4
uint32_t AT24CXX_ReadLenByte(uint16_t ReadAddr,uint8_t Len)
{  	
	uint8_t t;
	uint32_t temp=0;
	for(t=0;t<Len;t++)
	{
		temp<<=8;
		temp+=AT24CXX_ReadOneByte(ReadAddr+Len-t-1); 	 				   
	}
	return temp;												    
}
//检查AT24CXX是否正常
//这里用了24XX的最后一个地址(255)来存储标志字.
//如果用其他24C系列,这个地址要修改
//返回1:检测失败
//返回0:检测成功
int AT24CXX_Check(void)
{
	uint8_t temp;
	OS_ERR err;
	temp=AT24CXX_ReadOneByte(254);//避免每次开机都写AT24CXX			   
	if(temp==EEPROM_MAGIC_FLAG)
    return 0;		   
	else//排除第一次初始化的情况
	{
		reset_eeprom();
		OSTimeDlyHMSM(0, 0, 0, 25,OS_OPT_TIME_HMSM_STRICT,&err);
		AT24CXX_WriteOneByte(254,EEPROM_MAGIC_FLAG);
	  temp=AT24CXX_ReadOneByte(254);	  
		if(temp==EEPROM_MAGIC_FLAG)
      return 0;
	}
	return -1;											  
}

//在AT24CXX里面的指定地址开始读出指定个数的数据
//ReadAddr :开始读出的地址 对24c02为0~255
//pBuffer  :数据数组首地址
//NumToRead:要读出数据的个数
void AT24CXX_Read(uint16_t ReadAddr,uint8_t *pBuffer,uint16_t NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++=AT24CXX_ReadOneByte(ReadAddr++);	
		NumToRead--;
	}
}  
//在AT24CXX里面的指定地址开始写入指定个数的数据
//WriteAddr :开始写入的地址 对24c02为0~255
//pBuffer   :数据数组首地址
//NumToWrite:要写入数据的个数
void AT24CXX_Write(uint16_t WriteAddr,uint8_t *pBuffer,uint16_t NumToWrite)
{
	uint8_t readback,i;
  while(NumToWrite--)
	{
		for(i = 0; i < 5; i++)
    {
      AT24CXX_WriteOneByte(WriteAddr,*pBuffer);
      readback = AT24CXX_ReadOneByte(WriteAddr);
      if(readback != *pBuffer)
      {
        //DBG_ERROR("write to AT24CXX error %d times,addr=%d,w_data=0x%x,r_data=0x%x\n",i,WriteAddr,*pBuffer,readback);
        delay_ms(1);
      } else {
        //DBG_INFO("write to AT24CXX ok,addr=%d,w_data=0x%x,r_data=0x%x\n",WriteAddr,*pBuffer,readback);
        break;
      }
      if(i >= 5)
      {
        DBG_ERROR("write to AT24CXX error try %d times,addr=%d,w_data=0x%x,r_data=0x%x\n",i,WriteAddr,*pBuffer,readback);
      }  
    }  
		WriteAddr++;
		pBuffer++;
	}
}

void eeprom_debug_all_data(void)
{
  uint8_t i,j;
  uint8_t readData[255];
  AT24CXX_Read(0,readData,255);
  for(j = 0; j < 4; j++)
  {  
    DBG_INFO("%02x %02x %02x",readData[j*63],readData[j*63 + 1],readData[j*63 + 2]);
    DBG_INFO("\n");
    for(i = 3; i < 63; i++)
    {
      DBG_INFO("%02x ",readData[j*63 + i]);
      if((i - 2)%12 == 0 && i != 0)
        DBG_INFO("\n");
    }
   }  
   DBG_INFO("\n");
}

void at24cxx_init_error(void)
{
  DBG_ERROR("!!!!!!eeprom read err\n");
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
  beep_start(2000);
  LED1(ON);
  hw_delay_ms(500);
  LED1(OFF);
  hw_delay_ms(500);
} 










