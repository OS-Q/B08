

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "includes.h"
#include "app_cfg.h"
#include "debug.h"
#include "delay.h"
#include "rc522.h"
#include "gpio_init.h"
#include "rc522_task.h"
#include "uart2_task.h"
#include "usart2.h"
#include "timer2.h"
#include "rtc_config.h"
#include "spi_flash.h"
#include "24cxx.h"
#include "open_result_task.h"
#include "led_task.h"
#include "flash_task.h"
#include "watch_dog.h"
#include <os_app_hooks.h>

uint32_t swVer=0x10;

static OS_TCB   StartUp_TCB;
static CPU_STK 	StartUp_stk[STARTUP_TASK_STK_SIZE];

extern void SoftReset(void);

/* 启动任务，在这里启动其他任务 */
static void StartUp_Task(void *p_arg)
{
	OS_ERR err;
	(void)p_arg;
	
	#if (OS_CFG_STAT_TASK_EN > 0) 
		OSStatTaskCPUUsageInit(&err); 		/* Determine CPU capacity. */ 
	#endif
	
	#if (OS_CFG_APP_HOOKS_EN > 0)
		App_OS_SetAllHooks();
	#endif
	
	if(spiflash_init() != 0)
  {
    flash_init_error();
    SoftReset();
  }
  
  if(AT24CXX_Init() != 0)
  {
    at24cxx_init_error();
    SoftReset();
  } 
	
	/* 创建任务	PB1闪烁	*/
	OSTaskCreate((OS_TCB     *)&LED_TASK_TCB,       /* 任务控制块指针	*/   
				 (CPU_CHAR   *)"LED任务",									/* 任务名称，好像随便写。。。*/ 
				 (OS_TASK_PTR ) Led_Task,									/* 任务代码指针	*/ 
				 (void       *) 0,											  /* 传递给任务的参数parg	*/ 
				 (OS_PRIO     ) LED_TASK_PRIO,						/* 任务优先级	*/ 
				 (CPU_STK    *)&LED_TASK_stk[0],					/* 任务堆栈基地址	*/ 
				 (CPU_STK_SIZE) LED_TASK_STK_SIZE / 10,	  /* 堆栈剩余警戒线	*/ 
				 (CPU_STK_SIZE) LED_TASK_STK_SIZE,				/* 堆栈大小	*/ 
				 (OS_MSG_QTY  ) 0u,											  /* 可接收的最大消息队列数	 */ 
				 (OS_TICK     ) 0u,											  /* 时间片乱转时间	*/ 
				 (void       *) 0,											  /* 任务控制块扩展信息	*/ 
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),	/* 任务选项 */ 
				 (OS_ERR     *)&err);										  /* 返回值：对/错*/ 

	/* 创建任务	串口2接收数据处理 */
	OSTaskCreate((OS_TCB     *)&UART2_TASK_TCB,     /* 任务控制块指针*/   
				 (CPU_CHAR   *)"UART2任务",								/* 任务名称，好像随便写。。。*/ 
				 (OS_TASK_PTR ) UART2_Task,							  /* 任务代码指针				 */ 
				 (void       *) 0,											  /* 传递给任务的参数parg		 */ 
				 (OS_PRIO     ) UART2_TASK_PRIO,					/* 任务优先级				 */ 
				 (CPU_STK    *)&UART2_TASK_stk[0],				/* 任务堆栈基地址			 */ 
				 (CPU_STK_SIZE) UART2_TASK_STK_SIZE / 10,			/* 堆栈剩余警戒线			 */ 
				 (CPU_STK_SIZE) UART2_TASK_STK_SIZE,					/* 堆栈大小					 */ 
				 (OS_MSG_QTY  ) 0u,											  /* 可接收的最大消息队列数	 */ 
				 (OS_TICK     ) 0u,											  /* 时间片乱转时间			 */ 
				 (void       *) 0,											  /* 任务控制块扩展信息		 */ 
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),	/* 任务选项*/ 
				 (OS_ERR     *)&err);										  /* 返回值：对/错			 */ 

  	/* 创建任务	读卡任务*/
	OSTaskCreate((OS_TCB     *)&RC522_TASK_TCB,     /* 任务控制块指针*/   
				 (CPU_CHAR   *)"Read card Task",					/* 任务名称，好像随便写。。。*/ 
				 (OS_TASK_PTR ) Rc522_Task,							  /* 任务代码指针				 */ 
				 (void       *) 0,											  /* 传递给任务的参数parg		 */ 
				 (OS_PRIO     ) RC522_TASK_PRIO,					/* 任务优先级				 */ 
				 (CPU_STK    *)&RC522_TASK_stk[0],				/* 任务堆栈基地址			 */ 
				 (CPU_STK_SIZE) RC522_TASK_STK_SIZE / 10,			/* 堆栈剩余警戒线			 */ 
				 (CPU_STK_SIZE) RC522_TASK_STK_SIZE,					/* 堆栈大小					 */ 
				 (OS_MSG_QTY  ) 0u,											  /* 可接收的最大消息队列数	 */ 
				 (OS_TICK     ) 0u,											  /* 时间片乱转时间			 */ 
				 (void       *) 0,											  /* 任务控制块扩展信息		 */ 
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),	/* 任务选项*/ 
				 (OS_ERR     *)&err);										  /* 返回值：对/错			 */ 
  
  /* 创建任务 */
	OSTaskCreate((OS_TCB     *)&OPEN_RESULT_TASK_TCB,     /* 任务控制块指针*/   
				 (CPU_CHAR   *)"Open Result Task",					/* 任务名称，好像随便写。。。*/ 
				 (OS_TASK_PTR ) OPEN_RESULT_Task,							  /* 任务代码指针				 */ 
				 (void       *) 0,											  /* 传递给任务的参数parg		 */ 
				 (OS_PRIO     ) OPEN_RESULT_TASK_PRIO,					/* 任务优先级				 */ 
				 (CPU_STK    *)&OPEN_RESULT_TASK_stk[0],				/* 任务堆栈基地址			 */ 
				 (CPU_STK_SIZE) OPEN_RESULT_TASK_STK_SIZE / 10,			/* 堆栈剩余警戒线			 */ 
				 (CPU_STK_SIZE) OPEN_RESULT_TASK_STK_SIZE,					/* 堆栈大小					 */ 
				 (OS_MSG_QTY  ) 0u,											  /* 可接收的最大消息队列数	 */ 
				 (OS_TICK     ) 0u,											  /* 时间片乱转时间			 */ 
				 (void       *) 0,											  /* 任务控制块扩展信息		 */ 
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),	/* 任务选项*/ 
				 (OS_ERR     *)&err);										  /* 返回值：对/错			 */ 
         
           
	OSTaskCreate((OS_TCB     *)&FLASH_TASK_TCB,    
				 (CPU_CHAR   *)"Flash Task",					
				 (OS_TASK_PTR ) Flash_Task,							 
				 (void       *) 0,											  
				 (OS_PRIO     ) FLASH_TASK_PRIO,					
				 (CPU_STK    *)&FLASH_TASK_stk[0],				
				 (CPU_STK_SIZE) FLASH_TASK_STK_SIZE / 10,			
				 (CPU_STK_SIZE) FLASH_TASK_STK_SIZE,					
				 (OS_MSG_QTY  ) 0u,											  
				 (OS_TICK     ) 0u,											  
				 (void       *) 0,											  
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),	
				 (OS_ERR     *)&err);

	OSTaskCreate((OS_TCB     *)&WDOG_TASK_TCB,    
				 (CPU_CHAR   *)"watchDog Task",					
				 (OS_TASK_PTR ) WDOG_Task,							 
				 (void       *) 0,											  
				 (OS_PRIO     ) WDOG_TASK_PRIO,					
				 (CPU_STK    *)&WDOG_TASK_stk[0],				
				 (CPU_STK_SIZE) WDOG_TASK_STK_SIZE / 10,			
				 (CPU_STK_SIZE) WDOG_TASK_STK_SIZE,					
				 (OS_MSG_QTY  ) 0u,											  
				 (OS_TICK     ) 0u,											  
				 (void       *) 0,											  
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),	
				 (OS_ERR     *)&err);	
  
	/* 删除启动任务	*/
	OSTaskDel(&StartUp_TCB,&err);
}

int main(void)
{
	OS_ERR err;
  
  /* gpio 初始化*/
  gpio_init();
  
	/* LED硬件初始化*/
	LED_Init();
	
	/* 串口1硬件初始化*/
	USART1_Init(115200);
  USART2_Init(115200);
  DBG_ERROR("\nsoftware version=%x.%x\n",swVer>>4,swVer&0x0F);
  
  /* time2 */
  //TIM2_Int_Init(9999,7199);   ((1+7199 )/72M)*(1+9999)=1
  //TIM2_Int_Init(369,71); //2700HZ
	//TIM2_Int_Init(249,71);   //4000HZ
  TIM2_Int_Init(487,71);   //2048HZ
  beep_start(1250);
  
  /* rtc config */
  if(rtc_config_init() != 0)
  {
    rtc_init_error();
    goto out;
  }
  
  //RC522 读卡初始化
  rc522_init();
    
	/* 心跳时钟初始化*/
	OS_CPU_SysTickInit();
	
	/* 操作系统初始化，此处已创建空闲任务*/
	OSInit(&err);
  
	/* 进入临界区 */
	OS_CRITICAL_ENTER();
	
	/* 创建起始任务*/
	OSTaskCreate((OS_TCB     *)&StartUp_TCB,          /* 任务控制块指针	*/   
				 (CPU_CHAR   *)"起始任务",									/* 任务名称，好像随便写。。。*/ 
				 (OS_TASK_PTR ) StartUp_Task,								/* 任务代码指针	*/ 
				 (void       *) 0,											    /* 传递给任务的参数parg	*/ 
				 (OS_PRIO     ) STARTUP_TASK_PRIO,					/* 任务优先级	*/ 
				 (CPU_STK    *)&StartUp_stk[0],							/* 任务堆栈基地址 */ 
				 (CPU_STK_SIZE) STARTUP_TASK_STK_SIZE / 10,	/* 堆栈剩余警戒线	*/ 
				 (CPU_STK_SIZE) STARTUP_TASK_STK_SIZE,			/* 堆栈大小	*/ 
				 (OS_MSG_QTY  ) 0u,											    /* 可接收的最大消息队列数*/ 
				 (OS_TICK     ) 0u,											    /* 时间片乱转时间	*/ 
				 (void       *) 0,											    /* 任务控制块扩展信息*/ 
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),	/* 任务选项*/ 
				 (OS_ERR     *)&err);										    /* 返回值：对/错	*/ 
		
	/* 退出临界区 */
	OS_CRITICAL_EXIT();
				 
	/* 操作系统开始开始运行 */
	OSStart(&err);

out:
  SoftReset();
	return 0;
}
