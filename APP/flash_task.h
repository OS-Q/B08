#ifndef  __FLASH_TASK_H__
#define  __FLASH_TASK_H__

/* 任务优先级 */
#define FLASH_TASK_PRIO 	  4

/* 堆栈大小，用来存放局部变量，寄存器值和中断中的变量等 */
#define FLASH_TASK_STK_SIZE 	  256 + 200

#define FLASH_MAGIC_FLAG        0x595A5B5C
#define FLASH_BLK_SIZE          4096
#define EEPROM_DATA_SIZE        252
#define FLASH_BLK_HEAD_SIZE     20
#define FLASH_BLK_NUM           1020
#define FLASH_EEPROM_SIZE_NUM   16

extern OS_TCB   FLASH_TASK_TCB;
extern CPU_STK  FLASH_TASK_stk[FLASH_TASK_STK_SIZE];

typedef struct _FLASH_BLk_LOC
{
	u32 blk_read;
  u32 read_len;
  u32 blk_write;
	u32 write_len;
} flash_blk_loc;

typedef struct _FLASH_SAVE_CONTAINER
{
	u8 *data_point;
  u32 loc;
  u16 data_len;
} flash_save_container;

void Flash_Task(void *p_arg);
uint8_t flash_get_blk_data(uint8_t* readData);
void flash_save_black_nameId(void);
void reset_flash(void);
void cleanBlackNameId( void );

#endif
