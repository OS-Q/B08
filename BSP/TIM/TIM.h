/*-----------------------------------------------------------------*/
/*********************定时器初始化与中断函数************************/
/*-----------------------------------------------------------------*/  
#ifndef __TIM_H 
#define __TIM_H  
#include "V_USART.h" 
#include "stm32f0xx.h"
  
void TIM3_init(u16 arr,u16 psc);

#endif  
