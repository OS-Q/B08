/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                           (c) Copyright 2009-2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      APPLICATION CONFIGURATION
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                     Micrium uC-Eval-STM32F107
*                                         Evaluation Board
*
* Filename      : app_cfg.h
* Version       : V1.00
* Programmer(s) : JJL
*                 EHS
*                 DC
*********************************************************************************************************
*/

#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__

#define ON 	1
#define OFF 0

#define LED1(opt)	((opt) ? GPIO_ResetBits(GPIOB,GPIO_Pin_1):GPIO_SetBits(GPIOB,GPIO_Pin_1))

/* 任务优先级 */
#define STARTUP_TASK_PRIO 	4
#define LED1_TASK_PRIO 		  4
#define LED2_TASK_PRIO 		  4
#define RC522_TASK_PRIO 	  4
#define UART2_TASK_PRIO 	  4

/* 堆栈大小，用来存放局部变量，寄存器值和中断中的变量等 */
#define STARTUP_TASK_STK_SIZE 	80
#define LED1_TASK_STK_SIZE 		  80
#define UART2_TASK_STK_SIZE     256 + 150
#define RC522_TASK_STK_SIZE 	  100

#endif
