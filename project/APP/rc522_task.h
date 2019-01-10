#ifndef  __RC522_TASK_H__
#define  __RC522_TASK_H__

#define MAX_BLACK_NAME_ID_NUM 99

extern OS_TCB   RC522_TASK_TCB;
extern CPU_STK  RC522_TASK_stk[RC522_TASK_STK_SIZE];
extern uint32_t blackNameID[MAX_BLACK_NAME_ID_NUM+1];

void Rc522_Task(void *p_arg);
void opend_lock(void);

#endif
