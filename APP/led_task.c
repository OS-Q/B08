#include "includes.h"
#include "string.h"
#include "app_cfg.h"
#include "delay.h"
#include "debug.h"
#include "led_task.h"

OS_TCB   LED_TASK_TCB;
CPU_STK  LED_TASK_stk[LED_TASK_STK_SIZE];

uint8_t Led_Mode = HAL_LED_MODE_FLASH_SLOW;

void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* GPIOD Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* Configure PC13 in output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	LED1(OFF);
}

void HalLedSet(uint8_t mode)
{
  Led_Mode = mode;
  //DBG_INFO("set Led mode=0x%X\n", Led_Mode);
}

/* led task function */
void Led_Task(void *p_arg)
{
	OS_ERR err;
	uint8_t i;
	OSTimeDlyHMSM(0, 0, 3, 0,OS_OPT_TIME_HMSM_STRICT,&err);
	while(1)
	{
    //DBG_INFO("Led mode=0x%X\n", Led_Mode);
    switch (Led_Mode)
    {
      case HAL_LED_MODE_OFF:
        LED1(OFF);
				for(i = 0; i < 4; i++)
			  {
					if(Led_Mode != HAL_LED_MODE_OFF)
					{
						break;
					}	
					OSTimeDlyHMSM(0, 0, 0, 250,OS_OPT_TIME_HMSM_STRICT,&err);
				}
        break;

      case HAL_LED_MODE_ON:
        LED1(ON);
        for(i = 0; i < 4; i++)
			  {
					if(Led_Mode != HAL_LED_MODE_ON)
					{
						break;
					}	
					OSTimeDlyHMSM(0, 0, 0, 250,OS_OPT_TIME_HMSM_STRICT,&err);
				}
        break;

      case HAL_LED_MODE_FLASH_SLOW:
        LED1(OFF);
				for(i = 0; i < 4; i++)
			  {
					if(Led_Mode != HAL_LED_MODE_FLASH_SLOW)
					{
						break;
					}	
					OSTimeDlyHMSM(0, 0, 0, 250,OS_OPT_TIME_HMSM_STRICT,&err);
				}	
        if(Led_Mode == HAL_LED_MODE_FLASH_SLOW)
        {
          LED1(ON);
          for(i = 0; i < 8; i++)
					{
						if(Led_Mode != HAL_LED_MODE_FLASH_SLOW)
						{
							break;
						}	
						OSTimeDlyHMSM(0, 0, 0, 250,OS_OPT_TIME_HMSM_STRICT,&err);
					}	
        } 
        break;

      case HAL_LED_MODE_FLASH_FAST:
        LED1(OFF);
        OSTimeDlyHMSM(0, 0, 0, 100,OS_OPT_TIME_HMSM_STRICT,&err);
        LED1(ON);
        OSTimeDlyHMSM(0, 0, 0, 100,OS_OPT_TIME_HMSM_STRICT,&err);
        break;      
      
      default:
        break;
    }
	}
}
