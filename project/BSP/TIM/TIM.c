/*-----------------------------------------------------------------*/
/*********************定时器初始化与中断函数************************/
/*-----------------------------------------------------------------*/  
#include "TIM.h" 
#include "stm32f0xx.h"

extern uint8_t DATA,DATA1;  //DATA定时器暂时存储数据  DATA1主函数中用于输出的
extern __IO uint8_t receivedFlag; //接受完成标志位

/***********************************************************************************
*     注意：个人在调试期间发现发送时间要小于接受时间
* 9600波特率时    SendingDelay=104    TIME3_init(108,72c)
* 115200波特率时    SendingDelay=8    TIME3_init(10,72c)
************************************************************************************/
void TIM3_init(u16 arr,u16 psc)
{
 
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr -1;			//从0~（arr-1）计数，定时周期为arr次
	TIM_TimeBaseStructure.TIM_Prescaler = psc-1;		//频率48mhz/psc,一般是1Mhz，1us吧
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//时钟不分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//向上计数
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_ClearITPendingBit(TIM3, TIM_FLAG_Update);
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);     

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;      //初始化了中断但没有使能
	NVIC_InitStructure.NVIC_IRQChannelPriority = 1;   
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM3_IRQHandler(void)
{  
	uint8_t tmp;
	static uint8_t i;
	if(TIM_GetFlagStatus(TIM3, TIM_FLAG_Update) != RESET)
	{
		tmp = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10);
		if(tmp == 1)
		DATA |= (1 << i); 
		i++;
		if(i >= 8)
		{
			i = 0;
			DATA1=DATA; 
			DATA=0;
			receivedFlag = 1;
			EXTI->IMR |= 1<<10;   	//开启外部中断
			TIM_Cmd(TIM3,DISABLE); 	//关闭TIM3
		}
		TIM_ClearITPendingBit(TIM3, TIM_FLAG_Update);
	}
}
