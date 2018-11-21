#ifndef __TIMER_H
#define __TIMER_H
#include "stm32f10x.h"

void TIM2_Int_Init(u16 arr,u16 psc);
void beep_start(uint16_t times);

#endif



