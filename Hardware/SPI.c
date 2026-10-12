#include "stm32f10x.h"
#include "Delay.h"

DMA_InitTypeDef DMA_InitStructure;//声明结构体，在之后的调用中改变其中一项不会改变其他项的值
volatile uint8_t DMAtime=0;

void SPI1_Init(){


   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_SPI1,ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	 RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	
   GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	 GPIO_InitStructure.GPIO_Pin=GPIO_Pin_4|GPIO_Pin_2;
	 GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	 GPIO_Init(GPIOA,&GPIO_InitStructure);
	 GPIO_SetBits(GPIOA, GPIO_Pin_4); 
	
	 GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	 GPIO_InitStructure.GPIO_Pin=GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	 GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	 GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

	
	 
	 SPI_InitTypeDef SPI_InitStructure;
	 SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2;//SPI时钟2分频 36MHZ
	 SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;//时钟第一个边沿采样数据
	 SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;//空闲状态下SCK时钟为低
   SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;//8位数据
   SPI_InitStructure.SPI_Direction= SPI_Direction_2Lines_FullDuplex ;//模式为双向发送
   SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;//MSB高位先行
   SPI_InitStructure.SPI_Mode=SPI_Mode_Master;//SPI主机
	 SPI_InitStructure.SPI_NSS=SPI_NSS_Soft ;//软件NSS
	 SPI_Init(SPI1,&SPI_InitStructure);
	  SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);//SPIDMA使能
	 SPI_Cmd(SPI1,ENABLE);
	 
	 DMA_InitStructure.DMA_BufferSize=128;//几个数据
	 DMA_InitStructure.DMA_DIR=DMA_DIR_PeripheralDST;
	 DMA_InitStructure.DMA_M2M=DMA_M2M_Disable;
	 DMA_InitStructure.DMA_MemoryDataSize= DMA_MemoryDataSize_Byte  ;
   DMA_InitStructure.DMA_MemoryInc= DMA_MemoryInc_Enable;
	 DMA_InitStructure.DMA_Mode=DMA_Mode_Normal;
	 DMA_InitStructure.DMA_PeripheralBaseAddr=(uint32_t)&SPI1->DR;
	 DMA_InitStructure.DMA_PeripheralDataSize=DMA_PeripheralDataSize_Byte;
	 DMA_InitStructure.DMA_PeripheralInc=DMA_PeripheralInc_Disable;
	 DMA_InitStructure.DMA_Priority=DMA_Priority_High ;
	 DMA_Init(DMA1_Channel3,&DMA_InitStructure);
	 
	
 DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);//中断使能 只有写了这个才会开启中断
	 //并且要选对中断标志位才能正确进中断
	 
	 
	 NVIC_InitTypeDef NVIC_InitStructure;
	 NVIC_InitStructure.NVIC_IRQChannel=DMA1_Channel3_IRQn;;
	 NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1; 		
   NVIC_Init(&NVIC_InitStructure);
	 

	  
}
//DMA发送函数
void SPI_DMA(uint16_t Data,uint8_t *Arrdy) {
 
	  

    DMA_Cmd(DMA1_Channel3, DISABLE);
	
	  DMA1_Channel3->CMAR = (uint32_t)Arrdy;            
    DMA1_Channel3->CNDTR = Data;        
	
    DMA_Cmd(DMA1_Channel3, ENABLE);
	
	
}
//DMA中断
void DMA1_Channel3_IRQHandler(void){

	if(DMA_GetITStatus(DMA1_FLAG_TC3)==SET){
	 DMAtime=0;
	 DMA_ClearITPendingBit(DMA1_FLAG_TC3);//清除中断标志位
	 //一定要清理中断标志位不然会反复进入中断导致程序卡死
		DMA_Cmd(DMA1_Channel3,DISABLE);
	}
	//中断标志位会进中断，普通标志位不会
	//有进中断的函数和检查标志位的函数 注意区分
	
}
