#ifndef __XY_IIC_H
#define __XY_IIC_H
#include "stm32f10x.h"

//IO方向设置
#define SDA_IN()  {GPIOB->CRL   &=0x0FFFFFFF;GPIOB->CRL  |=0x80000000;}
#define SDA_OUT() {GPIOB->CRL &=0x0FFFFFFF;GPIOB->CRL  |=0x30000000;}

#define SET_IIC_SCL		GPIO_SetBits(GPIOB,GPIO_Pin_6)
#define CLR_IIC_SCL		GPIO_ResetBits(GPIOB,GPIO_Pin_6)

#define SET_IIC_SDA		GPIO_SetBits(GPIOB,GPIO_Pin_7)
#define CLR_IIC_SDA		GPIO_ResetBits(GPIOB,GPIO_Pin_7)

#define GET_IIC_SDA		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7)

//IIC所有操作函数
void IIC_Init(void);                //初始化IIC的IO口				 
void IIC_Start(void);				//发送IIC开始信号
void IIC_Stop(void);	  			//发送IIC停止信号
void IIC_Send_Byte(uint8_t txd);			//IIC发送一个字节
uint8_t IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
uint8_t IIC_Wait_Ack(void); 				//IIC等待ACK信号
void IIC_Ack(void);					//IIC发送ACK信号
void IIC_NAck(void);				//IIC不发送ACK信号

void IIC_Write_One_Byte(uint8_t daddr,uint8_t addr,uint8_t data);
uint8_t IIC_Read_One_Byte(uint8_t daddr,uint8_t addr);	  
#endif
















