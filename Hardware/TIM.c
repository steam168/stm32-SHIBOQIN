#include "stm32f10x.h"

void TIM_PWM_Init(){

   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2|RCC_APB1Periph_TIM4,ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB,ENABLE);
	
	 GPIO_InitTypeDef GPIO_InitStructure;
	 GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
	 GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
   GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	 GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	
	 GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6;
	 GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	 GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	 GPIO_Init(GPIOB,&GPIO_InitStructure);
	 
	 TIM_TimeBaseInitTypeDef TIM_TimeStructure;
	 TIM_TimeStructure.TIM_Period=7200-1;
	 TIM_TimeStructure.TIM_Prescaler=10-1;
	 TIM_TimeStructure.TIM_CounterMode=TIM_CounterMode_Up;
	 TIM_TimeStructure.TIM_ClockDivision=TIM_CKD_DIV1 ;
	 TIM_TimeBaseInit(TIM2,&TIM_TimeStructure);
	 
	 
	 
	 TIM_TimeStructure.TIM_Period=65536-1;
	 TIM_TimeStructure.TIM_Prescaler=72-1;
	 TIM_TimeStructure.TIM_CounterMode=TIM_CounterMode_Up;
	 TIM_TimeStructure.TIM_ClockDivision=TIM_CKD_DIV1 ;
	 TIM_TimeBaseInit(TIM4,&TIM_TimeStructure);
	
   TIM_OCInitTypeDef TIM_InitStructure;
   TIM_InitStructure.TIM_OCMode= TIM_OCMode_PWM1 ;
	 TIM_InitStructure.TIM_OutputState=  TIM_OutputState_Enable;//主通道使能引脚嫩输出波形
	 TIM_InitStructure.TIM_OCPolarity=TIM_OCPolarity_High ;
	 TIM_InitStructure.TIM_Pulse=1500;
	 TIM_OC1Init(TIM2,&TIM_InitStructure);
	 
	 
	 TIM_ICInitTypeDef TIM_ICStructInit;
	 TIM_ICStructInit.TIM_Channel=TIM_Channel_1;
	 TIM_ICStructInit.TIM_ICFilter=0x00;
	 TIM_ICStructInit.TIM_ICPolarity=TIM_ICPolarity_Rising;
	 TIM_ICStructInit.TIM_ICPrescaler=TIM_ICPSC_DIV1;
	 TIM_ICStructInit.TIM_ICSelection=TIM_ICSelection_DirectTI;
	 TIM_ICInit(TIM4,&TIM_ICStructInit);
	 
	 TIM_SelectInputTrigger(TIM4, TIM_TS_TI1FP1);					
	 TIM_SelectSlaveMode(TIM4, TIM_SlaveMode_Reset);		
	 
	  TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
		
	 NVIC_InitTypeDef NVIC_InitStructure;
	 NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
	 NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority=3;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3; 		
   NVIC_Init(&NVIC_InitStructure);
		
		
	  TIM_Cmd(TIM2,ENABLE);
		TIM_Cmd(TIM4,ENABLE);
}


void TIM2_IRQHandler(){
 
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)  
    {
       
        ADC_SoftwareStartConvCmd(ADC2,ENABLE);//读取一次旋钮值
        
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  
    }
}


void PWM_SetCompare(uint16_t Compare)
{
	TIM_SetCompare1(TIM2, Compare);	
}
uint16_t TIM_IC(){

   return      1000000 / (TIM_GetCapture1(TIM4) + 1);//测周法求频率

}
