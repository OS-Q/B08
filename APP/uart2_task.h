#ifndef  __UART2_TASK_H__
#define  __UART2_TASK_H__

#define CMD_ACQUIRE_OPEN_RST    0x04
#define CMD_SYS_RESET           0x05
#define CMD_FACTORY_RESET       0x87
#define CMD_SET_BLACK_NAME      0x8F
#define CMD_CLEAN_BLACK_NAME    0x90

extern OS_TCB   UART2_TASK_TCB;
extern CPU_STK  UART2_TASK_stk[UART2_TASK_STK_SIZE];

void UART2_Task(void *p_arg);

#endif
