#ifndef RTC_CONFIG_H
#define RTC_CONFIG_H

typedef struct _deviceTime
{
	uint8_t	init;/* 0 没有初始化 ；1 rtc 初始化；2 服务器已初始化*/
	uint32_t	set_s;/* 系统设置时间，单位s  */
	uint32_t	local_s;/*设置时系统上电到当前的运行时间，单位s  */
} deviceTime;

int rtc_config_init(void);
int get_local_time(uint32_t *time);
int set_local_time(uint8_t *date);
int get_rtc_time(unsigned int *rtc);
void get_rtc_to_local_time(void);
int set_rtc_time_second(uint32_t rtc);
void rtc_init_error(void);

#endif

