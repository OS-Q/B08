#include "includes.h"
#include "string.h"
#include "app_cfg.h"
#include "delay.h"
#include "debug.h"
#include "24cxx.h"
#include "rc522.h"
#include "rc522_task.h"
#include "spi_flash.h"
#include "flash_task.h"

OS_TCB   FLASH_TASK_TCB;
CPU_STK  FLASH_TASK_stk[FLASH_TASK_STK_SIZE];

OS_SEM Flash_Sem;

flash_blk_loc g_flash_blk_loc;
uint8_t flash_data_buf[256];

static void flash_get_data(void)
{
	uint32_t address;
  uint32_t readData[5],writeData[5];
  
  uint32_t i;
  //DBG_INFO("flash blk_read=%d,blk_write=%d,write_len=%d\n",g_flash_blk_loc.blk_read,g_flash_blk_loc.blk_write,g_flash_blk_loc.write_len);
  for(i = 0; i < 1; i++)
  {
    address = i*FLASH_BLK_SIZE;
    SPI_Flash_Read((uint8_t *)readData,address,FLASH_BLK_HEAD_SIZE);
    if(readData[0] == FLASH_MAGIC_FLAG)
    {
      g_flash_blk_loc.blk_read = readData[1];
      g_flash_blk_loc.blk_write = readData[3];
      address = g_flash_blk_loc.blk_read*FLASH_BLK_SIZE + 8;
      SPI_Flash_Read((uint8_t *)readData,address,4);
      g_flash_blk_loc.read_len = readData[0];
      address = g_flash_blk_loc.blk_write*FLASH_BLK_SIZE + 16;
      SPI_Flash_Read((uint8_t *)readData,address,4);
      g_flash_blk_loc.write_len = readData[0];
      //DBG_INFO("find flash FLASH_MAGIC_FLAG i = %d\n",i);
      break;
    } 
  }
  
  if(i >= 1)
  {
    g_flash_blk_loc.blk_read = 0;
    g_flash_blk_loc.read_len = 0;
    g_flash_blk_loc.blk_write = 0;
    g_flash_blk_loc.write_len = 0;
    writeData[0] = FLASH_MAGIC_FLAG;
    writeData[1] = g_flash_blk_loc.blk_read;
    writeData[2] = g_flash_blk_loc.read_len;
    writeData[3] = g_flash_blk_loc.blk_write;
    writeData[4] = g_flash_blk_loc.write_len;
    SPI_Flash_Write((uint8_t *)writeData,0,FLASH_BLK_HEAD_SIZE);
  }
  DBG_INFO("flash blk_read=%d,read_len=%d,blk_write=%d,write_len=%d\n",
            g_flash_blk_loc.blk_read,g_flash_blk_loc.read_len,g_flash_blk_loc.blk_write,g_flash_blk_loc.write_len);
  
  address = 1021*FLASH_BLK_SIZE;
  SPI_Flash_Read((uint8_t *)readData,address,8);
  if(readData[0] == FLASH_MAGIC_FLAG)
  {
    blackNameID[0] = readData[1];
    SPI_Flash_Read((uint8_t *)(blackNameID + 1),address + 8,blackNameID[0]*4);
    DBG_INFO("have black id in flash: %d\n",blackNameID[0]);
    #if 1
    {
      uint8_t i;
      for(i = 1; i <= blackNameID[0]; i++)
      {
        DBG_INFO("0x%08x ",blackNameID[i]);
        if(i%5 == 0)
          DBG_INFO("\n");
      }
    }
    #endif
  } else {
    blackNameID[0] = 0;
    writeData[0] = FLASH_MAGIC_FLAG;
    writeData[1] = blackNameID[0];
    SPI_Flash_Write((uint8_t *)writeData,address,8);
  }
}

uint8_t flash_get_blk_data(uint8_t* readData)
{
	uint32_t loc;
  uint32_t writeData[5];
  
  if( g_flash_blk_loc.blk_read == g_flash_blk_loc.blk_write && g_flash_blk_loc.read_len == g_flash_blk_loc.write_len)
  {  
    //flash have no data
    DBG_INFO("\nflash is empty!!\n");
    return M_FALSE;
  }
  
  if( g_flash_blk_loc.read_len == FLASH_EEPROM_SIZE_NUM)
  {  
    g_flash_blk_loc.read_len = 0;
    if(g_flash_blk_loc.blk_read == (FLASH_BLK_NUM - 1))
    {
      g_flash_blk_loc.blk_read = 0;
    }    
  } 
  
  //read 252 bytes data from flash
  loc = g_flash_blk_loc.blk_read*FLASH_BLK_SIZE + FLASH_BLK_HEAD_SIZE +  g_flash_blk_loc.read_len*EEPROM_DATA_SIZE;
  SPI_Flash_Read(readData,loc,EEPROM_DATA_SIZE);
  DBG_INFO("1 flash blk_read=%d,read_len=%d,blk_write=%d,write_len=%d\n",
            g_flash_blk_loc.blk_read,g_flash_blk_loc.read_len,g_flash_blk_loc.blk_write,g_flash_blk_loc.write_len);
  
  //need updata read blk head
  g_flash_blk_loc.read_len++;
  if(g_flash_blk_loc.read_len < FLASH_EEPROM_SIZE_NUM)
  {
    //update read len
    SPI_Flash_Write((uint8_t *)(&(g_flash_blk_loc.read_len)),g_flash_blk_loc.blk_read*FLASH_BLK_SIZE + 8,4);
  } else {
    if( g_flash_blk_loc.blk_read == g_flash_blk_loc.blk_write && g_flash_blk_loc.read_len == g_flash_blk_loc.write_len)
    {
      //read over of data
      DBG_INFO("flash read over of data!\n");
      SPI_Flash_Write((uint8_t *)(&(g_flash_blk_loc.read_len)),g_flash_blk_loc.blk_read*FLASH_BLK_SIZE + 8,4);
      return M_TRUE;
    }
    
    //earse data
    loc = g_flash_blk_loc.blk_read*FLASH_BLK_SIZE;
    //writeData[0] = 0;
    //SPI_Flash_Write((uint8_t *)(&(writeData[0])),loc,4);
    spiflash_sector4k_erase(g_flash_blk_loc.blk_read);
    
    g_flash_blk_loc.read_len = 0;
    g_flash_blk_loc.blk_read++;
    if( g_flash_blk_loc.blk_read >= FLASH_BLK_NUM)
      g_flash_blk_loc.blk_read = 0;
    
    writeData[0] = FLASH_MAGIC_FLAG;
    writeData[1] = g_flash_blk_loc.blk_read;
    writeData[2] = g_flash_blk_loc.read_len;
    writeData[3] = g_flash_blk_loc.blk_write;
    writeData[4] = g_flash_blk_loc.write_len;
    SPI_Flash_Write((uint8_t *)writeData,g_flash_blk_loc.blk_read*FLASH_BLK_SIZE,FLASH_BLK_HEAD_SIZE);
    
    SPI_Flash_Write((uint8_t *)writeData,0,FLASH_BLK_HEAD_SIZE);
  }
  DBG_INFO("2 flash blk_read=%d,read_len=%d,blk_write=%d,write_len=%d\n",
            g_flash_blk_loc.blk_read,g_flash_blk_loc.read_len,g_flash_blk_loc.blk_write,g_flash_blk_loc.write_len);
  
  return M_TRUE;
}

static void flash_save_data(uint8_t *data_buf)
{
  uint32_t loc;
  uint32_t readData[5],writeData[5];
  //flash_save_container mflash_save_container[2];
 
  //this block have space
  if(g_flash_blk_loc.write_len < FLASH_EEPROM_SIZE_NUM)
  {
    loc = g_flash_blk_loc.blk_write*FLASH_BLK_SIZE + FLASH_BLK_HEAD_SIZE +  g_flash_blk_loc.write_len*EEPROM_DATA_SIZE;
    DBG_INFO("1 flash write data to %d\n",loc);
    g_flash_blk_loc.write_len++;
 
    //mflash_save_container[0].data_point = (uint8_t *)(&(g_flash_blk_loc.write_len));
    //mflash_save_container[0].loc = g_flash_blk_loc.blk_write*FLASH_BLK_SIZE + 16;
    //mflash_save_container[0].data_len = 4;
    
    //mflash_save_container[1].data_point = data_buf;
    //mflash_save_container[1].loc = loc;
    //mflash_save_container[1].data_len = EEPROM_DATA_SIZE;

    //SPI_Flash_Write_Str(mflash_save_container, 2);
    
    //save write len to write blk
    SPI_Flash_Write((uint8_t *)(&(g_flash_blk_loc.write_len)),g_flash_blk_loc.blk_write*FLASH_BLK_SIZE + 16,4);
    //save data
    SPI_Flash_Write(data_buf,loc,EEPROM_DATA_SIZE);
    
    //save write len to read blk
    //if(g_flash_blk_loc.blk_write != g_flash_blk_loc.blk_read)
    //{  
      //SPI_Flash_Write((uint8_t *)(&(g_flash_blk_loc.write_len)),g_flash_blk_loc.blk_read*FLASH_BLK_SIZE + 16,4);
    //}  
  } else {
    //this block don't have space
    //g_flash_blk_loc.blk_write++;
    if(g_flash_blk_loc.blk_write < FLASH_BLK_NUM - 1)
    {
      g_flash_blk_loc.blk_write++;
      g_flash_blk_loc.write_len = 1;
      
      writeData[0] = FLASH_MAGIC_FLAG;
      writeData[1] = g_flash_blk_loc.blk_read;
      writeData[2] = g_flash_blk_loc.read_len;
      writeData[3] = g_flash_blk_loc.blk_write;
      writeData[4] = g_flash_blk_loc.write_len;
      //write to first head.
      SPI_Flash_Write((uint8_t *)writeData,0,FLASH_BLK_HEAD_SIZE);
      //write to write blk
      SPI_Flash_Write((uint8_t *)writeData,g_flash_blk_loc.blk_write*FLASH_BLK_SIZE,FLASH_BLK_HEAD_SIZE);
      
      loc = g_flash_blk_loc.blk_write*FLASH_BLK_SIZE + FLASH_BLK_HEAD_SIZE;
      DBG_INFO("2 flash write data to %d\n",loc);
      //write data to flash
      SPI_Flash_Write(flash_data_buf,loc,EEPROM_DATA_SIZE);
           
    } else {
      //g_flash_blk_loc.blk_write == FLASH_BLK_NUM
      g_flash_blk_loc.blk_write = 0;
      g_flash_blk_loc.write_len = 1;
      writeData[0] = FLASH_MAGIC_FLAG;
      writeData[1] = g_flash_blk_loc.blk_read;
      writeData[2] = g_flash_blk_loc.read_len;
      writeData[3] = g_flash_blk_loc.blk_write;
      writeData[4] = g_flash_blk_loc.write_len;
      
      //mflash_save_container[0].data_point = (uint8_t *)(writeData);
      //mflash_save_container[0].loc = 0;
      //mflash_save_container[0].data_len = FLASH_BLK_HEAD_SIZE;
      
      SPI_Flash_Write((uint8_t *)writeData,0,FLASH_BLK_HEAD_SIZE);
        
      //need save this time result!
      loc = FLASH_BLK_HEAD_SIZE;
      DBG_INFO("3 flash write data to %d\n",loc);
      //mflash_save_container[1].data_point = data_buf;
      //mflash_save_container[1].loc = loc;
      //mflash_save_container[1].data_len = EEPROM_DATA_SIZE;
      SPI_Flash_Write(data_buf,loc,EEPROM_DATA_SIZE); 
      
      //SPI_Flash_Write_Str(mflash_save_container, 2);
    }
    
    if(g_flash_blk_loc.blk_write == g_flash_blk_loc.blk_read)
    {
      uint32_t tmp = g_flash_blk_loc.blk_read + 1;
      tmp = tmp >= FLASH_BLK_NUM ? 0:tmp;
      loc = tmp*FLASH_BLK_SIZE;
      SPI_Flash_Read((uint8_t *)readData,loc,FLASH_BLK_HEAD_SIZE);
      
      if(readData[0] == FLASH_MAGIC_FLAG)
      {
        g_flash_blk_loc.blk_read = tmp;
        SPI_Flash_Write((uint8_t *)(&tmp),12,4);
      }
    }
  }
  
  DBG_INFO("flash blk_read = %d, read_len = %d, blk_write = %d, write_len = %d\n",g_flash_blk_loc.blk_read,g_flash_blk_loc.read_len,g_flash_blk_loc.blk_write,g_flash_blk_loc.write_len);
}

void flash_test(void)
{
	uint32_t address=0x0, readData=0, writeData=0;

	SPI_Flash_Read((uint8_t *)&readData,address,4);
	if (readData==0xF0F1F2F3)//0x12345678  0xFFFFEFFF
		DBG_INFO("flash read ok, data match,address=0x%X,readData=0x%X\n", address,readData);
	else
	{
		writeData=0xF0F1F2F3;
		DBG_INFO("flash read err readData=0x%X,eeprom write=0x%X\n",readData,writeData);
		SPI_Flash_Write((uint8_t*)&writeData,address,4);
	}
}

void flash_save_black_nameId(void)
{
	uint32_t address = 1021*FLASH_BLK_SIZE;
	SPI_Flash_Write((uint8_t*)&blackNameID,address + 4,(1 + blackNameID[0])*4);
}

void cleanBlackNameId( void )
{
	uint32_t address=0x0,writeData[2];
  DBG_INFO("\nclean black name!\n");
  blackNameID[0] = 0;
  writeData[0] = FLASH_MAGIC_FLAG;
  writeData[1] = blackNameID[0];
  address = 1021*FLASH_BLK_SIZE;
  SPI_Flash_Write((uint8_t *)writeData,address,8);
}

void reset_flash(void)
{
	uint32_t writeData[5];
  uint32_t address=1021*FLASH_BLK_SIZE;
	OS_ERR err;
	DBG_INFO("Reset Flash!!!\n");
  g_flash_blk_loc.blk_read = 0;
  g_flash_blk_loc.read_len = 0;
  g_flash_blk_loc.blk_write = 0;
  g_flash_blk_loc.write_len = 0;
  
  //spiflash_sector_all_erase();
  
//   writeData[0] = FLASH_MAGIC_FLAG;
//   writeData[1] = g_flash_blk_loc.blk_read;
//   writeData[2] = g_flash_blk_loc.read_len;
//   writeData[3] = g_flash_blk_loc.blk_write;
//   writeData[4] = g_flash_blk_loc.write_len;
//   SPI_Flash_Write((uint8_t *)writeData,0,FLASH_BLK_HEAD_SIZE);
	//erase block 0
	spiflash_sector4k_erase(0);
	OSTimeDlyHMSM(0, 0, 0, 20,OS_OPT_TIME_HMSM_STRICT,&err);
  spiflash_sector4k_erase(1023);
	OSTimeDlyHMSM(0, 0, 0, 20,OS_OPT_TIME_HMSM_STRICT,&err);
  blackNameID[0] = 0;
  writeData[0] = FLASH_MAGIC_FLAG;
  writeData[1] = blackNameID[0];
  SPI_Flash_Write((uint8_t *)writeData,address,8);
}

/* flash task function */
void Flash_Task(void *p_arg)
{
	OS_ERR err;

  flash_get_data();
  OSTimeDlyHMSM(0, 0, 1, 0,OS_OPT_TIME_HMSM_STRICT,&err);
  //flash_test();
  
  OSSemCreate(&Flash_Sem,"Flash_Sem",0,&err);
  
  while(1)
  {  
    OSSemPend (&Flash_Sem,0,OS_OPT_PEND_BLOCKING,NULL,&err);
    flash_save_data(flash_data_buf);
    DBG_INFO("save data to flash!\n");
  }
}
