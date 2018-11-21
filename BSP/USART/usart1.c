/*------------------------------------------------------------- 
*硬件串口函数定义
-----------------------------------------------------------------*/  
#include "string.h"
#include "usart1.h"
#include "debug.h"
#include "stm32f10x.h"

void USART1_Init(uint32_t Usart_BaudRate)
{
  NVIC_InitTypeDef  NVIC_InitStructure; 
  GPIO_InitTypeDef  GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure; 
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   	//enable clock of GPIOA
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); 	//enable clock of USART1
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);		//复用时钟
  
  /* Configure USART1 TX */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure USARTy RX as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  USART_InitStructure.USART_BaudRate          =   Usart_BaudRate;
  USART_InitStructure.USART_WordLength        =   USART_WordLength_8b;
  USART_InitStructure.USART_StopBits          =   USART_StopBits_1;
  USART_InitStructure.USART_Parity            =   USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);

  /* Configure the NVIC Preemption Priority Bits */  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  /* Enable the USARTy Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

#if USART1_RXNE_INTERRUPT      
  USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
#endif

  USART_Cmd(USART1, ENABLE);//enable USART1
  
#if USART_TC_INTERRUPT        
  USART_ClearFlag(USART1,USART_FLAG_TC);   
  USART_ITConfig(USART1,USART_IT_TC,ENABLE);
#endif
}

void USART1_Send_Byte(uint8_t dat)
{
  USART_SendData(USART1,dat);
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);//wait untill transmitt complete
}

void USART1_Send_String(uint8_t *string)
{
  while(*string != '\0')
    USART1_Send_Byte(*string++);
}

void usart_printf(USART_TypeDef* USARTx, uint8_t *Data,uint8_t len)
{
  char char_data[150] = {'\0'};
  char char_tmp[3] = {0,0,0};
  uint8_t i;
  //memset(char_data,0,150);
  for(i = 0; i < len; i++)
  {
    sprintf(char_tmp,"%02x",Data[i]);
    strcat(char_data,char_tmp);
    //DBG_INFO("%s",char_data);
  }
  DBG_INFO("%s",char_data);
}

int fputc(int ch, FILE *f)
{
   USART_SendData(USART1, (u8) ch);
   while(!(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == SET))
   {
   }
   return ch;
}

int fgetc(FILE *f)
{
   while(!(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET))
   {
   }
   return (USART_ReceiveData(USART1));
}

