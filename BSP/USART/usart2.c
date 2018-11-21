
#include "stm32f10x.h"
#include "string.h"
#include "usart2.h"

volatile unsigned char  rcv_cmd_start=0;
volatile unsigned char  rcv_cmd_flag=0;

unsigned char  usart2_rcv_buf[MAX_RCV_LEN];
volatile unsigned int   usart2_rcv_len=0;

unsigned char  usart2_cmd_buf[MAX_CMD_LEN];
volatile unsigned int   usart2_cmd_len=0;

unsigned char device_id[4] = {0xAA, 0xAA, 0xAA, 0x32};

/**
  * @brief  USART2初始化函数
**/
void USART2_Init(uint32_t Usart_BaudRate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
	
    /* config USART2 clock */   
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); 

    /* USART2 GPIO config */
    /* Configure USART2 Tx (PA.02) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);    
    /* Configure USART2 Rx (PA.03) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
      
    /* USART2 mode config */
    USART_InitStructure.USART_BaudRate = Usart_BaudRate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure); 
    USART_Cmd(USART2, ENABLE);
		
		//Enable usart2 receive interrupt
		USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); 
		
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
    NVIC_Init(&NVIC_InitStructure);
}


/**
  * @brief  USART2发送一个字符串
**/
void usart2_write(USART_TypeDef* USARTx, uint8_t *Data,uint32_t len)
{
    uint32_t i;
		USART_ClearFlag(USART2,USART_FLAG_TC); 
    for(i=0; i<len; i++)
    {                                         
        USART_SendData(USARTx, *Data++);
        while( USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET );
    }
}

/**
  * @brief  This function handles usart2 global interrupt request.
  * @param  None
  * @retval : None
  */
void USART2_IRQHandler(void)
{
  unsigned char data;
  if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)         //判断是否接收中断  
  {
    data = USART_ReceiveData(USART2);
    //USART_SendData(USART1,data);
    //while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);  //判断发送标志
  
    if(data==0xFF)
		{
			rcv_cmd_start = 1;
		}
		if(rcv_cmd_start==1)
		{
			usart2_rcv_buf[usart2_rcv_len] = data;
      usart2_rcv_len++;
      if(usart2_rcv_len > 5)
      {
        if(usart2_rcv_buf[2] == 0)
        {
          if(usart2_rcv_len == 6 || (usart2_rcv_len>=MAX_CMD_LEN-1))
          {
            memcpy(usart2_cmd_buf,usart2_rcv_buf,usart2_rcv_len);
            usart2_cmd_len = usart2_rcv_len;
            rcv_cmd_start = 0;
            memset(usart2_rcv_buf,0,usart2_rcv_len);
            usart2_rcv_len = 0;
            rcv_cmd_flag = 1;
          }  
        } else {
          if(usart2_rcv_len == (5 + usart2_rcv_buf[2]) || (usart2_rcv_len>=MAX_CMD_LEN-1))
          {
            memcpy(usart2_cmd_buf,usart2_rcv_buf,usart2_rcv_len);
            usart2_cmd_len = usart2_rcv_len;
            rcv_cmd_start = 0;
            memset(usart2_rcv_buf,0,usart2_rcv_len);
            usart2_rcv_len = 0;
            rcv_cmd_flag = 1;
          }
        }  
      }
		}
  }
}



