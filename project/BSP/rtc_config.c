#include <time.h>
#include "stdio.h"
#include "stm32f10x.h"
#include "rtc_config.h"
#include "delay.h"
#include "debug.h"
#include "app_cfg.h"
#include "timer2.h"

static void delay_us(unsigned int us)
{
	unsigned int i,j;
	
	for(i=0;i<us;i++)
	{
		for(j=0;j<30;j++)
			;
	}
}

int rtc_config_init(void)
{
	//检查是不是第一次配置时钟
	uint32_t tmp=0;
	//uint8 rtc[7]={16,1,26,1,13,27,30};

	//从指定的后备寄存器中读出数据:读出了与写入的指定数据不相乎
	if (BKP_ReadBackupRegister(BKP_DR1) != 0x5050)
	{	 			
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	
		//使能PWR和BKP外设时钟   
		PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问 
		BKP_DeInit();	//复位备份区域 	
		BKP_TamperPinCmd(DISABLE);
		RCC_LSEConfig(RCC_LSE_ON);	//设置外部低速晶振(LSE),使用外设低速晶振

		//检查指定的RCC标志位设置与否,等待低速晶振就绪
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && tmp < 500)
		{
			tmp++;
      delay_us(5000);
		}
		
		if(tmp>=500)
		{
			DBG_ERROR("rtc_config_init error!!!");
			return -1;//初始化时钟失败,晶振有问题
		}
			
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟    
		RCC_RTCCLKCmd(ENABLE);	//使能RTC时钟  
		RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
		RTC_WaitForSynchro();		//等待RTC寄存器同步  
		//RTC_ITConfig(RTC_IT_SEC, ENABLE);		//使能RTC秒中断
		RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
		RTC_EnterConfigMode();/// 允许配置	
		RTC_SetPrescaler(32767); //设置RTC预分频的值
		RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
		//设置时间	
		RTC_ExitConfigMode(); //退出配置模式  
		BKP_WriteBackupRegister(BKP_DR1, 0X5050);	
		//向指定的后备寄存器中写入用户程序数据

		//set_rtc_time(rtc);/*  设置时间 */
	}

	get_rtc_to_local_time();/*  获取时间 */
	
	return 0; 
}

//2000+time[0] 年; 月1-12; 日1-31;星期 1-7;//时0-23;//分0-59;//秒
int set_rtc_time(unsigned char *rtc)
{
	uint32_t seccount;
	struct tm *tm,tm_;

	tm_.tm_year=rtc[0]+2000;
	tm_.tm_mon=rtc[1];
	tm_.tm_mday=rtc[2];
	tm_.tm_hour=rtc[4];
	tm_.tm_min=rtc[5];
	tm_.tm_sec=rtc[6];
	tm_.tm_wday=rtc[3];

	DBG_INFO("set %04d-%02d-%02d %02d:%02d:%02d %d\n",tm_.tm_year,tm_.tm_mon,tm_.tm_mday,tm_.tm_hour,tm_.tm_min,tm_.tm_sec,tm_.tm_wday);

	tm_.tm_year -=1900;
	tm_.tm_mon--;
	seccount=mktime(&tm_);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟  
	PWR_BackupAccessCmd(ENABLE);	//使能RTC和后备寄存器访问 
	RTC_SetCounter(seccount);	//设置RTC计数器的值
	
	tm=localtime(&seccount);
	tm->tm_year +=1900;
	tm->tm_mon++;
	DBG_INFO("get %04d-%02d-%02d %02d:%02d:%02d %d\n",tm->tm_year,tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec,tm->tm_wday);

	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成  	
	return 0;	    
}

int set_rtc_time_second(uint32_t rtc)
{
	struct tm *tm;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟  
	PWR_BackupAccessCmd(ENABLE);	//使能RTC和后备寄存器访问 
	RTC_SetCounter(rtc);	//设置RTC计数器的值
  
  tm=localtime(&rtc);
	tm->tm_year +=1900;
	tm->tm_mon++;
	DBG_INFO("set %04d-%02d-%02d %02d:%02d:%02d %d\n",tm->tm_year,tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec,tm->tm_wday);
	
	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成  	
	return 0;	    
}

int get_rtc_time(unsigned int *rtc)
{
	struct tm *tm;
	
 	*rtc=RTC->CNTH;
	*rtc <<=16;
	*rtc +=RTC->CNTL;

	tm=localtime(rtc);
	tm->tm_year +=1900;
	tm->tm_mon++;
	DBG_INFO("%04d-%02d-%02d %02d:%02d:%02d week=%d\n",tm->tm_year,tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec,tm->tm_wday);
 
	return 0;
}

deviceTime devTime;
void get_rtc_to_local_time(void)
{
	deviceTime *time=&devTime;

	if(get_rtc_time(&time->set_s)==0)
	{
//		time->local_s=get_powerOnToNow_ms()/1000;
		time->init=1;
	}
	else
	{
//		time->local_s=get_powerOnToNow_ms()/1000;
		time->set_s=0;
		time->init=0;
	}
}

int get_local_time(uint32_t *time)
{
  //deviceTime *time_t=&devTime;

  //*time=((get_powerOnToNow_ms()/1000)+time_t->set_s-time_t->local_s);

	return 0;
}

int set_local_time(uint8_t *date)
{
	deviceTime *time=&devTime;

	if(time->init==2)
		return -1;
						
	if((date[1]>12)||(date[1]==0))
		return -2;
	if((date[2]>31)||(date[2]==0))
		return -3;
	if(date[4]>23)
		return -4;
	if(date[5]>59)
		return -5;
	
	set_rtc_time(date);
	get_rtc_to_local_time();
	
	return 0;
}

void rtc_init_error(void)
{
  hw_delay_ms(2000);
  beep_start(2000);
  LED1(ON);
  hw_delay_ms(500);
  LED1(OFF);
  hw_delay_ms(500);
  beep_start(2000);
  LED1(ON);
  hw_delay_ms(500);
  LED1(OFF);
  hw_delay_ms(500);
}  
