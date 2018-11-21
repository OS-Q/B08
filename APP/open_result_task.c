#include "includes.h"
#include "string.h"
#include "delay.h"
#include "debug.h"
#include "usart2.h"
#include "open_result_task.h"
#include "rc522.h"
#include "24cxx.h"

OS_TCB   OPEN_RESULT_TASK_TCB;
CPU_STK  OPEN_RESULT_TASK_stk[OPEN_RESULT_TASK_STK_SIZE];

OS_SEM Open_Rsl_Sem;
extern OS_SEM Flash_Sem;

extern record_rfid_open g_record_rfid_open[10];
extern uint8_t record_rfid_open_len;
at24xx_blk_loc g_at24xx_blk_loc;
extern uint8_t flash_data_buf[256];

void eeprom_get_data(void);
void eeprom_save_data(void);

void eeprom_test(void)
{
	int address=0x0, readData=0, writeData=0;

	AT24CXX_Read(address,(unsigned char *)&readData,4);
	if (readData==0xF0F1F2F3)//0x12345678  0xFFFFEFFF
		DBG_INFO("eeprom read ok, data match,address=0x%X,readData=0x%X\n", address,readData);
	else
	{
		writeData=0xF0F1F2F3;
		DBG_INFO("eeprom read err readData=0x%X,eeprom write=0x%X\n",readData,writeData);
		AT24CXX_Write(address,(uint8_t*)&writeData,4);
	}
}

/* task function */
void OPEN_RESULT_Task(void *p_arg)
{
  OS_ERR err;
  OSSemCreate(&Open_Rsl_Sem,"Open_Rsl_Sem",0,&err);
  //OS_CRITICAL_ENTER();
  //eeprom_test();
  //OS_CRITICAL_EXIT();
  
  eeprom_get_data();
  
	while(1)
	{
		//DBG_INFO(" CPU Usage: %d%%\r\n", ((OSStatTaskCPUUsage == 10000u)? 0 : OSStatTaskCPUUsage/100));
    //OSTimeDlyHMSM(0, 0, 1, 0,OS_OPT_TIME_HMSM_STRICT,&err);
    OSSemPend (&Open_Rsl_Sem,0,OS_OPT_PEND_BLOCKING,NULL,&err);
    //get_rtc_to_local_time();
    eeprom_save_data();
	}
}

void reset_eeprom(void)
{
	uint8_t writeData,i;
	DBG_INFO("Reset EEPROM!!!\n");
  g_at24xx_blk_loc.blk_read = 0;
  g_at24xx_blk_loc.blk_write = 0;
  g_at24xx_blk_loc.write_len = 0;
  writeData = 0;
  for(i = 0; i < 255; i++)
  {
    AT24CXX_Write(i,&writeData,1);
  }  
}

void eeprom_get_data(void)
{
	uint16_t address;
  uint8_t readData[3],writeData[3];
  OS_ERR err;
  
  uint8_t i;
  for(i = 0; i < EEPROM_BLK_NUM; i++)
  {
    address = i*AT24XX_BLK_SIZE;
    AT24CXX_Read(address,readData,2);
    if(readData[0] == EEPROM_MAGIC_FLAG)
    {
      g_at24xx_blk_loc.blk_read = i;
      g_at24xx_blk_loc.blk_write = readData[1];
      //g_at24xx_blk_loc.write_len = readData[2];
      OSTimeDlyHMSM(0, 0, 0, 25,OS_OPT_TIME_HMSM_STRICT,&err);
      AT24CXX_Read(g_at24xx_blk_loc.blk_write*AT24XX_BLK_SIZE+2,readData+2,1);
      g_at24xx_blk_loc.write_len = readData[2];
      //DBG_INFO("find eeprom EEPROM_MAGIC_FLAG, i = %d\n",i);
      break;
    } 
  }
  OSTimeDlyHMSM(0, 0, 0, 25,OS_OPT_TIME_HMSM_STRICT,&err);
  if(i >= EEPROM_BLK_NUM)
  {
    g_at24xx_blk_loc.blk_read = 0;
    g_at24xx_blk_loc.blk_write = 0;
    g_at24xx_blk_loc.write_len = 0;
    writeData[0] = EEPROM_MAGIC_FLAG;
    writeData[1] = g_at24xx_blk_loc.blk_write;
    writeData[2] = g_at24xx_blk_loc.write_len;
    AT24CXX_Write(0,writeData,3);
  }
 
  DBG_INFO("eeprom blk_read=%d,blk_write=%d,write_len=%d\n",g_at24xx_blk_loc.blk_read,g_at24xx_blk_loc.blk_write,g_at24xx_blk_loc.write_len);
}

uint8_t eeprom_get_blk_data(uint8_t* readData)
{
	uint16_t address;
  uint8_t writeData[3];
  OS_ERR err;
 
  if( g_at24xx_blk_loc.blk_read == g_at24xx_blk_loc.blk_write )
  {  
    //flash have no data
    DBG_INFO("eeprom is empty!!\n");
    return M_FALSE;
  }
  
  //read AT24XX_BLK_SIZE data in eeprom
  address = g_at24xx_blk_loc.blk_read*AT24XX_BLK_SIZE;
  AT24CXX_Read(address,readData,AT24XX_BLK_SIZE);
  
  //need update read blk head
  writeData[0] = 0;
  OSTimeDlyHMSM(0, 0, 0, 25,OS_OPT_TIME_HMSM_STRICT,&err);
  AT24CXX_Write(address,writeData,1);
  g_at24xx_blk_loc.blk_read++;
  writeData[0] = EEPROM_MAGIC_FLAG;
  writeData[1] = g_at24xx_blk_loc.blk_write;
  writeData[2] = g_at24xx_blk_loc.write_len;
  //DBG_INFO("write data address %d ,blk_write %d, write_len %d\n",g_at24xx_blk_loc.blk_read*AT24XX_BLK_SIZE,g_at24xx_blk_loc.blk_write,g_at24xx_blk_loc.write_len);
  OSTimeDlyHMSM(0, 0, 0, 25,OS_OPT_TIME_HMSM_STRICT,&err);
  AT24CXX_Write(g_at24xx_blk_loc.blk_read*AT24XX_BLK_SIZE,writeData,3);

  return M_TRUE;
}

void eeprom_save_data(void)
{
  OS_ERR err;
  uint8_t i,loc;
  uint8_t writeData[3];
  
  for(i = 0; i < record_rfid_open_len; i++)
  {
    //this block have space
    if(g_at24xx_blk_loc.write_len < EEPROM_BLK_SIZE)
    {
      loc = g_at24xx_blk_loc.blk_write*AT24XX_BLK_SIZE + 3 +  g_at24xx_blk_loc.write_len*12;
      DBG_INFO("1 write data to %d\n",loc);
      
      AT24CXX_Write(loc,(uint8_t *)(g_record_rfid_open+i),12);
      
      g_at24xx_blk_loc.write_len++;
      //save len to write blk
      OSTimeDlyHMSM(0, 0, 0, 25,OS_OPT_TIME_HMSM_STRICT,&err);
      AT24CXX_Write(g_at24xx_blk_loc.blk_write*AT24XX_BLK_SIZE+2,&(g_at24xx_blk_loc.write_len),1);
      //save len to read blk
      //AT24CXX_Write(g_at24xx_blk_loc.blk_read*AT24XX_BLK_SIZE+2,&(g_at24xx_blk_loc.write_len),1);
    } else {
      //this block don't have space
      //g_at24xx_blk_loc.blk_write++;
      if(g_at24xx_blk_loc.blk_write < EEPROM_BLK_NUM - 1)
      {
        g_at24xx_blk_loc.write_len = 0;
        g_at24xx_blk_loc.blk_write++;
        
        loc = g_at24xx_blk_loc.blk_write*AT24XX_BLK_SIZE + 3 +  g_at24xx_blk_loc.write_len*12;
        DBG_INFO("2 write data to %d\n",loc);
        AT24CXX_Write(loc,(uint8_t *)(g_record_rfid_open+i),12);
        g_at24xx_blk_loc.write_len++;
        
        writeData[0] = EEPROM_MAGIC_FLAG;
        writeData[1] = g_at24xx_blk_loc.blk_write;
        writeData[2] = g_at24xx_blk_loc.write_len;
        OSTimeDlyHMSM(0, 0, 0, 25,OS_OPT_TIME_HMSM_STRICT,&err);
        AT24CXX_Write(g_at24xx_blk_loc.blk_write*AT24XX_BLK_SIZE,writeData,3);
        OSTimeDlyHMSM(0, 0, 0, 25,OS_OPT_TIME_HMSM_STRICT,&err);
        //save to read blk
        AT24CXX_Write(g_at24xx_blk_loc.blk_read*AT24XX_BLK_SIZE,writeData,3);
      } 
    }
    
    if(g_at24xx_blk_loc.blk_read != 0 && g_at24xx_blk_loc.blk_write >= (EEPROM_BLK_NUM - 1) && g_at24xx_blk_loc.write_len >= EEPROM_BLK_SIZE)
    {
      uint8_t tmp = g_at24xx_blk_loc.blk_write - g_at24xx_blk_loc.blk_read;
      uint8_t data_len = (tmp + 1) * AT24XX_BLK_SIZE;
      uint8_t data_buf[256];
      AT24CXX_Read(g_at24xx_blk_loc.blk_read*AT24XX_BLK_SIZE,data_buf,data_len);
      g_at24xx_blk_loc.blk_read = 0;
      g_at24xx_blk_loc.blk_write = tmp;
      data_buf[0] = EEPROM_MAGIC_FLAG;
      data_buf[1] = g_at24xx_blk_loc.blk_write;
      data_buf[2] = g_at24xx_blk_loc.write_len;
      DBG_INFO("3 write data to %d\n",0);
      OSTimeDlyHMSM(0, 0, 0, 25,OS_OPT_TIME_HMSM_STRICT,&err);
      AT24CXX_Write(0,data_buf,data_len);
    }
  }
  
  if(g_at24xx_blk_loc.blk_read == 0 && g_at24xx_blk_loc.blk_write >= (EEPROM_BLK_NUM -1) && g_at24xx_blk_loc.write_len == EEPROM_BLK_SIZE) 
  {
    uint8_t i;
    //save data to flash
    DBG_INFO("write data full in eeprom,write to flash\n");
    AT24CXX_Read(0,flash_data_buf,EEPROM_ALL_SIZE);
    
    #if 0
    {
      uint8_t j,i;
      for(j = 0; j < 4; j++)
      {  
        DBG_INFO("%02x %02x %02x",flash_data_buf[j*63],flash_data_buf[j*63 + 1],flash_data_buf[j*63 + 2]);
        DBG_INFO("\n");
        for(i = 3; i < 63; i++)
        {
          DBG_INFO("%02x ",flash_data_buf[j*63 + i]);
          if((i - 2)%12 == 0 && i != 0)
            DBG_INFO("\n");
        }
      }  
      DBG_INFO("\n");
    }
    #endif
    
    OSSemPost(&Flash_Sem,OS_OPT_POST_ALL,&err);
          
    //earse data
    for(i = 1; i < EEPROM_BLK_NUM; i++)
    {
      uint16_t address = i*AT24XX_BLK_SIZE;
      uint8_t k = 0;
      OSTimeDlyHMSM(0, 0, 0, 25,OS_OPT_TIME_HMSM_STRICT,&err);
      AT24CXX_Write(address,&k,1);
    }
    g_at24xx_blk_loc.blk_read = 0;
    g_at24xx_blk_loc.blk_write = 0;
    g_at24xx_blk_loc.write_len = 0;
    
    writeData[0] = EEPROM_MAGIC_FLAG;
    writeData[1] = g_at24xx_blk_loc.blk_write;
    writeData[2] = g_at24xx_blk_loc.write_len;
    OSTimeDlyHMSM(0, 0, 0, 25,OS_OPT_TIME_HMSM_STRICT,&err);
    AT24CXX_Write(0,writeData,3);
  }
  DBG_INFO("blk_read = %d, blk_write = %d, write_len = %d\n",g_at24xx_blk_loc.blk_read,g_at24xx_blk_loc.blk_write,g_at24xx_blk_loc.write_len);
  record_rfid_open_len = 0;
  //eeprom_debug_all_data();
}

