#ifndef  __WATCH_DOG_H__
#define  __WATCH_DOG_H__

/* 任务优先级 */
#define WDOG_TASK_PRIO 	  4

/* 堆栈大小，用来存放局部变量，寄存器值和中断中的变量等 */
#define WDOG_TASK_STK_SIZE 	    80

extern OS_TCB   WDOG_TASK_TCB;
extern CPU_STK  WDOG_TASK_stk[WDOG_TASK_STK_SIZE];

void WDOG_Task(void *p_arg);

#endif
