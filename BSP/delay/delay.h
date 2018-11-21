#ifndef __DELAY_H
#define __DELAY_H 			   
#include "stm32f10x.h"

//void delay_Config(u8 SYSCLK);
void delay_ms(u32 nms);
void delay_s(u32 nTime);
//void delay_us(u32 nus);
void hw_delay_ms(u32 nTime);

#endif





























