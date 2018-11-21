/*------------------------------------------------------------- 
*硬件串口函数声明
-----------------------------------------------------------------*/   
#ifndef __USART1_H 
#define __USART1_H  
#include "stm32f10x.h"
#include "stdio.h"

//#define DMA_USART1	//使用DMA，不用请注释
#define USART1_RXNE_INTERRUPT	1u

void USART1_Init(uint32_t Usart_BaudRate);
void USART1_Send_Byte(uint8_t dat);
void USART1_Send_String(uint8_t *string);
void usart_printf(USART_TypeDef* USARTx, uint8_t *Data,uint8_t len);

int fputc(int ch, FILE *f);     //fputc重定向
int fgetc(FILE *f); 			//fgetc重定向
  
#endif  
