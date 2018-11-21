#include "includes.h"
#include "string.h"
#include "app_cfg.h"
#include "delay.h"
#include "debug.h"
#include "usart1.h"
#include "usart2.h"
#include "rc522.h"
#include "uart2_task.h"
#include "rc522_task.h"
#include "open_result_task.h"
#include "flash_task.h"
#include "24cxx.h"
#include "rtc_config.h"

OS_TCB   UART2_TASK_TCB;
CPU_STK  UART2_TASK_stk[UART2_TASK_STK_SIZE];

extern unsigned char device_id[4];

void SoftReset(void)
{
  OS_ERR err;
	DBG_ERROR("\nsoft reset after 250ms!!!\n");
	OSTimeDlyHMSM(0, 0, 0, 250, OS_OPT_TIME_HMSM_STRICT,&err);
	__set_FAULTMASK(1);      
  NVIC_SystemReset();
}


/* uart2 task function */
void UART2_Task(void *p_arg)
{
  OS_ERR err;
  uint8_t *cmd;
  
  //reset bt
  GPIO_ResetBits(GPIOA,GPIO_Pin_11);
  delay_ms(500);
  GPIO_SetBits(GPIOA,GPIO_Pin_11);
	
	while(1)
	{
		//DBG_INFO(" CPU Usage: %d%%\r\n", ((OSStatTaskCPUUsage == 10000u)? 0 : OSStatTaskCPUUsage/100));
    OSTimeDlyHMSM(0, 0, 0, 5, OS_OPT_TIME_HMSM_STRICT,&err);
    //get_rtc_to_local_time();
    if(rcv_cmd_flag == 1)
    {  
      usart_printf(USART1, usart2_cmd_buf, usart2_cmd_len);
      cmd = usart2_cmd_buf+1;
      if(*cmd == 0x02)
      {
        opend_lock();
      }
      else if(*cmd == 0x03)
      {
        device_id[0] = *(cmd+2);
        device_id[1] = *(cmd+3);
        device_id[2] = *(cmd+4);
        device_id[3] = *(cmd+5);
        usart2_write(USART2, usart2_cmd_buf, usart2_cmd_len);
        DBG_INFO("\ndevice id:%02x %02x %02x %02x\n",device_id[0],device_id[1],device_id[2],device_id[3]);
      }
      else if(*cmd == 0x8D)
      {
        uint32_t TimeMs = 0;
        uint8_t T_long = *(cmd+1) - 3;
        uint8_t i;
        //struct tm *tm;
        for(i = 0; i < T_long; i++)
        {
          TimeMs = TimeMs*10 + (*(cmd+2+i) - 0x30);
        }
        DBG_INFO("\ntime second: %d\n",TimeMs);
        TimeMs += 8*60*60;
        
        set_rtc_time_second(TimeMs);
        
        get_rtc_time(&TimeMs);
        //tm=localtime(&TimeMs);
        //tm->tm_year +=1900;
        //tm->tm_mon++;
        //DBG_INFO("get %04d-%02d-%02d %02d:%02d:%02d %d\n",tm->tm_year,tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec,tm->tm_wday);
      }
      else if(*cmd == CMD_ACQUIRE_OPEN_RST)
      {
        //read open door result!
        static uint8_t readData[255],n = 0;
        if(n != 0)
        {
          uint8_t k = AT24XX_BLK_SIZE*(4 - n);
          DBG_INFO("\nhave open door data\n");
          readData[k] = 0xFF;
          readData[k+1] = CMD_ACQUIRE_OPEN_RST;
          readData[k+2] = AT24XX_BLK_SIZE - 3;
          usart2_write(USART2, readData+k, AT24XX_BLK_SIZE);
          n--;
          {
            #if 0
            uint8_t i,j;
            for(j = 0; j < 1; j++)
            {  
              DBG_INFO("%02x %02x %02x\n",readData[k],readData[k + 1],readData[k + 2]);
              for(i = 3; i < 63; i++)
              {
                DBG_INFO("%02x ",readData[k + i]);
                if((i - 2)%12 == 0 && i != 0)
                  DBG_INFO("\n");
              }
            }  
            DBG_INFO("\n");
            #endif
          }
        } else if(flash_get_blk_data(readData) == M_TRUE){
          //flash have data
          DBG_INFO("\nflash have open door data\n");
          
          readData[0] = 0xFF;
          readData[1] = CMD_ACQUIRE_OPEN_RST;
          readData[2] = AT24XX_BLK_SIZE - 3;
          usart2_write(USART2, readData, AT24XX_BLK_SIZE);
          n = 3;
          
          {
            #if 0
            uint8_t i,j;
            for(j = 0; j < 1; j++)
            {  
              DBG_INFO("%02x %02x %02x\n",readData[j*63],readData[j*63 + 1],readData[j*63 + 2]);
              for(i = 3; i < 63; i++)
              {
                DBG_INFO("%02x ",readData[j*63 + i]);
                if((i - 2)%12 == 0 && i != 0)
                  DBG_INFO("\n");
              }
            }  
            DBG_INFO("\n");
            #endif
          }
          
        } else if(eeprom_get_blk_data(readData) == M_TRUE){
          //eeprom have data
          DBG_INFO("\neeprom have open door data\n");
          readData[0] = 0xFF;
          readData[1] = CMD_ACQUIRE_OPEN_RST;
          readData[2] = AT24XX_BLK_SIZE - 3;
          usart2_write(USART2, readData, AT24XX_BLK_SIZE);
          {
            #if 0
            uint8_t i,j;
            for(j = 0; j < 1; j++)
            {  
              DBG_INFO("%02x %02x %02x\n",readData[j*63],readData[j*63 + 1],readData[j*63 + 2]);
              for(i = 3; i < 63; i++)
              {
                DBG_INFO("%02x ",readData[j*63 + i]);
                if((i - 2)%12 == 0 && i != 0)
                  DBG_INFO("\n");
              }
            }  
            DBG_INFO("\n");
            #endif
          }
        }
      } else if(*cmd == CMD_SET_BLACK_NAME) {
        uint8_t mBlackNameID[4],i,k = *(cmd+1) / 4;
        for(i = 0; i < k; i++)
        {
          mBlackNameID[0] = *(cmd+2+4*i);
          mBlackNameID[1] = *(cmd+3+4*i);
          mBlackNameID[2] = *(cmd+4+4*i);
          mBlackNameID[3] = *(cmd+5+4*i);
          DBG_INFO("\nset black name ID=0x%08x\n",u8_to_u32(mBlackNameID));
          if(blackNameID[0] >= MAX_BLACK_NAME_ID_NUM)
          {  
            return;
          }
          blackNameID[0]++;
          blackNameID[blackNameID[0]] = u8_to_u32(mBlackNameID);
        }
        flash_save_black_nameId();
      } else if(*cmd == CMD_FACTORY_RESET) {
        reset_eeprom();
        reset_flash();
        SoftReset();
      } else if(*cmd == CMD_CLEAN_BLACK_NAME) {
        cleanBlackNameId();
      } else if(*cmd == CMD_SYS_RESET) {
        SoftReset();
      }
      memset(usart2_cmd_buf,0,usart2_cmd_len);
      rcv_cmd_flag = 0;
    }
	}
}

