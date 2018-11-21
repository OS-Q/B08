#ifndef  __OPRN_RESULT_TASK_H__
#define  __OPRN_RESULT_TASK_H__

/* 任务优先级 */
#define OPEN_RESULT_TASK_PRIO 	  4

/* 堆栈大小，用来存放局部变量，寄存器值和中断中的变量等 */
#define OPEN_RESULT_TASK_STK_SIZE 	  256 + 100

#define EEPROM_MAGIC_FLAG      0x59
#define EEPROM_BLK_NUM         4
#define EEPROM_BLK_SIZE        5
#define EEPROM_ALL_SIZE        252

extern OS_TCB   OPEN_RESULT_TASK_TCB;
extern CPU_STK  OPEN_RESULT_TASK_stk[OPEN_RESULT_TASK_STK_SIZE];

void OPEN_RESULT_Task(void *p_arg);
uint8_t eeprom_get_blk_data(uint8_t* readData);
void reset_eeprom(void);

#endif
