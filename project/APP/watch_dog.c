#include "includes.h"
#include "string.h"
#include "app_cfg.h"
#include "delay.h"
#include "debug.h"
#include "watch_dog.h"

OS_TCB   WDOG_TASK_TCB;
CPU_STK  WDOG_TASK_stk[WDOG_TASK_STK_SIZE];

/*task function */
void WDOG_Task(void *p_arg)
{
	OS_ERR err;
  
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  /* IWDG counter clock: LSI/32 */
  IWDG_SetPrescaler(IWDG_Prescaler_64);
  /* 
     Counter Reload Value = 250ms/IWDG counter clock period
                          = 250ms / (LSI/32)
                          = 0.25s / (LsiFreq/32)
                          = LsiFreq/(32 * 4)
                          = LsiFreq/128
     Time = (IWDG_Prescaler_64 / LsiFreq) * value;
     1250 = Time / (IWDG_Prescaler_64 / LsiFreq) = 2 / (64 / 40000)
   */
  IWDG_SetReload(1250);
  /* Reload IWDG counter */
  IWDG_ReloadCounter();
  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();

  while(1)
  {  
    OSTimeDlyHMSM(0, 0, 1, 500,OS_OPT_TIME_HMSM_STRICT,&err);
    /* Reload IWDG counter */
    IWDG_ReloadCounter();
  }
}
