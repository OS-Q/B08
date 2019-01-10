#include "includes.h"
#include "app_cfg.h"
#include "delay.h"
#include "rc522.h"
#include "debug.h"
#include "timer2.h"
#include "rtc_config.h"
#include "rc522_task.h"
#include "led_task.h"

OS_TCB   RC522_TASK_TCB;
CPU_STK  RC522_TASK_stk[RC522_TASK_STK_SIZE];

record_rfid_open g_record_rfid_open[10];
__IO uint8_t record_rfid_open_len = 0;
uint32_t blackNameID[MAX_BLACK_NAME_ID_NUM+1];

extern OS_SEM Open_Rsl_Sem;

void opend_lock(void)
{
  HalLedSet(HAL_LED_MODE_ON);
  DBG_INFO("\nopen lock\n");
  beep_start(1250);
  GPIO_SetBits(GPIOB,GPIO_Pin_3);
  delay_ms(500);
  GPIO_ResetBits(GPIOB,GPIO_Pin_3);
  delay_s(1);
  HalLedSet(HAL_LED_MODE_FLASH_SLOW);
}

u8 addCheckCode(u8 *data)
{
  u8 i;
  u8 checkCode;
  checkCode = data[0];
  for(i = 1; i < 11; i++)
  {
    checkCode = checkCode ^ data[i];
  }
  data[11] = checkCode;
  return checkCode;
}

/* read card task function */
void Rc522_Task(void *p_arg)
{
	OS_ERR err;
	//CPU_INT16U CPU_Usage;
  u8 ret;
  PcdReset();
	
	while(1)
	{
    ret = ReadCardFuntion();
    if(ret == MI_OK)
    {
      DBG_INFO("match device id,open the door\n");
      g_record_rfid_open[record_rfid_open_len].card_id[0] = CardNum[3];
      g_record_rfid_open[record_rfid_open_len].card_id[1] = CardNum[2];
      g_record_rfid_open[record_rfid_open_len].card_id[2] = CardNum[1];
      g_record_rfid_open[record_rfid_open_len].card_id[3] = CardNum[0];
      DBG_INFO("card_id[0] = %x,card_id[1] = %x,card_id[2] = %x,card_id[3] = %x\n",CardNum[0],CardNum[1],CardNum[2],CardNum[3]);
      opend_lock();
      get_rtc_time(&(g_record_rfid_open[record_rfid_open_len].time));
      DBG_INFO("open time = %x\n",g_record_rfid_open[record_rfid_open_len].time);
      //printf("uinix time = %d\n",g_record_rfid_open[record_rfid_open_len].time);
      g_record_rfid_open[record_rfid_open_len].result = 1;
      addCheckCode((u8 *)(&g_record_rfid_open[record_rfid_open_len]));
      record_rfid_open_len++;
      //usart2_write(USART2, (uint8 *)(g_record_rfid_open+record_rfid_open_len), 12);
      OSTimeDlyHMSM(0, 0, 0, 50,OS_OPT_TIME_HMSM_STRICT,&err);
      OSSemPost (&Open_Rsl_Sem,OS_OPT_POST_ALL,&err);
      OSTimeDlyHMSM(0, 0, 0, 500,OS_OPT_TIME_HMSM_STRICT,&err);
    } else if(ret == MI_NOTAGERR) {
      DBG_INFO("dismatch device id,open the door\n");
      g_record_rfid_open[record_rfid_open_len].card_id[0] = CardNum[3];
      g_record_rfid_open[record_rfid_open_len].card_id[1] = CardNum[2];
      g_record_rfid_open[record_rfid_open_len].card_id[2] = CardNum[1];
      g_record_rfid_open[record_rfid_open_len].card_id[3] = CardNum[0];
      DBG_INFO("card_id[0] = %x,card_id[1] = %x,card_id[2] = %x,card_id[3] = %x\n",CardNum[0],CardNum[1],CardNum[2],CardNum[3]);
      get_rtc_time(&(g_record_rfid_open[record_rfid_open_len].time));
      DBG_INFO("dismatch device time = %x\n",g_record_rfid_open[record_rfid_open_len].time);
      //printf("uinix time = %d\n",g_record_rfid_open[record_rfid_open_len].time);
      g_record_rfid_open[record_rfid_open_len].result = 0;
      addCheckCode((u8 *)(&g_record_rfid_open[record_rfid_open_len])); 
      record_rfid_open_len++;
      //usart2_write(USART2, (uint8 *)(g_record_rfid_open+record_rfid_open_len), 12);
      OSTimeDlyHMSM(0, 0, 0, 50,OS_OPT_TIME_HMSM_STRICT,&err);
      OSSemPost (&Open_Rsl_Sem,OS_OPT_POST_ALL,&err);
      OSTimeDlyHMSM(0, 0, 0, 500,OS_OPT_TIME_HMSM_STRICT,&err);
    }
		OSTimeDlyHMSM(0, 0, 0, 150,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}

