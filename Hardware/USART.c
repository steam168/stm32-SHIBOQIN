#include "stm32f10x.h"
#include <stdio.h>
#include"LCD.h"

extern char USART[2048];
extern uint16_t Buf [256];
volatile uint8_t USART_time=0;


void USART_Init1(){
   
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA,ENABLE);
	  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE); 
	
	 GPIO_InitTypeDef  GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	 GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;
	 GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	
	 GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	 USART_InitTypeDef USART_InitStructure;
	 USART_InitStructure.USART_Mode=USART_Mode_Tx;//发送模式
	 USART_InitStructure.USART_Parity=USART_Parity_No;//无校验位
	 USART_InitStructure.USART_BaudRate=921600;//波特率
	 USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;//关闭硬件流控
	 USART_InitStructure.USART_StopBits=USART_StopBits_1 ;//停止位
	 USART_InitStructure.USART_WordLength=USART_WordLength_8b;//数据长度
	 USART_Init(USART1,&USART_InitStructure);
	 
	 USART_Cmd(USART1,ENABLE);
	
	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);
	
	 DMA_InitTypeDef DMA_InitStructure;
	 DMA_InitStructure.DMA_BufferSize=0;//几个数据
	 DMA_InitStructure.DMA_DIR=DMA_DIR_PeripheralDST;
	 DMA_InitStructure.DMA_M2M=DMA_M2M_Disable;
	 DMA_InitStructure.DMA_MemoryBaseAddr=(uint32_t)USART;
	 DMA_InitStructure.DMA_MemoryDataSize= DMA_MemoryDataSize_Byte  ;
   DMA_InitStructure.DMA_MemoryInc= DMA_MemoryInc_Enable;
	 DMA_InitStructure.DMA_Mode= DMA_Mode_Normal ;
	 DMA_InitStructure.DMA_PeripheralBaseAddr=(uint32_t)&USART1->DR;;
	 DMA_InitStructure.DMA_PeripheralDataSize=DMA_PeripheralDataSize_Byte;
	 DMA_InitStructure.DMA_PeripheralInc=DMA_PeripheralInc_Disable;
	 DMA_InitStructure.DMA_Priority=DMA_Priority_Medium;
	 DMA_Init(DMA1_Channel4,&DMA_InitStructure);
	
	
	//DMA_Cmd(DMA1_Channel4,ENABLE);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC|DMA_IT_TE, ENABLE);//中断使能 只有写了这个才会开启中断
	 //并且要选对中断标志位才能正确进中断
	 
	 
	 NVIC_InitTypeDef NVIC_InitStructure;
	 NVIC_InitStructure.NVIC_IRQChannel=DMA1_Channel4_IRQn;;
	 NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority=2;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2; 		
   NVIC_Init(&NVIC_InitStructure);
	 
}


void USART_DMA(char *Arrdy,uint16_t Data){


   DMA_Cmd(DMA1_Channel4, DISABLE);
	
	  DMA1_Channel4->CMAR = (uint32_t)Arrdy;              // ??????
    DMA1_Channel4->CNDTR = Data;   

	
	  DMA_Cmd(DMA1_Channel4, ENABLE);
}

void DMA1_Channel4_IRQHandler(void){

	if(DMA_GetITStatus(DMA1_IT_TC4)==SET){
	USART_time=0;
	 DMA_ClearITPendingBit(DMA1_IT_TC4);//清除中断标志位
	 //一定要清理中断标志位不然会反复进入中断导致程序卡死
	}
	//中断标志位会进中断，普通标志位不会
	//有进中断的函数和检查标志位的函数 注意区分
	if(DMA_GetITStatus(DMA1_IT_TE4)==SET){
	USART_time=0;
	 DMA_ClearITPendingBit(DMA1_IT_TE4);//清除中断标志位
	 //一定要清理中断标志位不然会反复进入中断导致程序卡死
	}
}
//发送数据函数
void USART_sprintf(uint16_t* Arrdy){
 
	  uint16_t pos = 0; 
	LCD_Bo(0);
	LCD_Bo(128);
	for(uint16_t i=0;i<256;i++){
  int len=sprintf(USART+pos,"%.2f\r\n",((float)Arrdy[i]/4096)*3.3f);
  pos += len;
	}
     while(USART_time == 1); 
	 USART_DMA(USART,pos);
	 USART_time=1;
}

