#include "stm32f10x.h"

extern uint16_t Buf[256];
extern uint16_t Buff[128];
extern uint8_t conut; 
extern uint8_t KeyNum;
uint16_t psc=72;
uint16_t x=0;

void ADC_DMA_Init(){

   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_ADC1|RCC_APB2Periph_ADC2,ENABLE);
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE); 
	 RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM3,ENABLE);
	
	 GPIO_InitTypeDef GPIO_InitStructure;
	 GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AIN;//模拟输入模式
	 GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
	 GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	 GPIO_Init(GPIOB,&GPIO_InitStructure);
	 
	 
	 TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	 TIM_TimeBaseInitStructure.TIM_Period=720-1;//ARR
	 TIM_TimeBaseInitStructure.TIM_Prescaler=100-1; //PSC
	 TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;//选择计数模式上升计数
	 TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1 ;//不分频
	 TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	 
	 TIM_SelectOutputTrigger(TIM3,TIM_TRGOSource_Update);//选择内部时钟源作为触发输入源
	  
	 TIM_Cmd(TIM3,ENABLE);
	
   ADC_InitTypeDef ADC_InitStructure;
	 ADC_InitStructure.ADC_Mode=ADC_Mode_Independent;//ADC独立工作模式
	 ADC_InitStructure.ADC_NbrOfChannel=1 ;//规则通道序列长度
   ADC_InitStructure.ADC_ScanConvMode=DISABLE;//扫描模式关闭
   ADC_InitStructure.ADC_ContinuousConvMode=DISABLE;//连续模式关闭
   ADC_InitStructure.ADC_DataAlign=ADC_DataAlign_Right ;//数据右对齐
   ADC_InitStructure.ADC_ExternalTrigConv= ADC_ExternalTrigConv_T3_TRGO;//触发模式为TIM3时钟	
   ADC_Init(ADC1,&ADC_InitStructure);//ADC1初始化 测波形主力
   ADC_DMACmd(ADC1,ENABLE); 
	 
	 ADC_InitStructure.ADC_Mode=ADC_Mode_Independent;
	 ADC_InitStructure.ADC_NbrOfChannel=1 ;
   ADC_InitStructure.ADC_ScanConvMode=DISABLE;
   ADC_InitStructure.ADC_ContinuousConvMode=DISABLE;
   ADC_InitStructure.ADC_DataAlign=ADC_DataAlign_Right ;
   ADC_InitStructure.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;//不选择为外部触发模式为软件触发	
   ADC_Init(ADC2,&ADC_InitStructure);
	 
	 
	 
   
   ADC_ExternalTrigConvCmd(ADC1, ENABLE); 	//外部触发模式使能 

	 ADC_Cmd(ADC1,ENABLE);//ADC1
	 ADC_RegularChannelConfig (ADC1,ADC_Channel_8,1,ADC_SampleTime_55Cycles5);//开启哪个通道和采样速率 
	 
	 ADC_Cmd(ADC2,ENABLE);//ADC2
	 ADC_RegularChannelConfig (ADC2,ADC_Channel_1,1,ADC_SampleTime_55Cycles5);  
  
	 
    ADC_ResetCalibration(ADC1);//开始校准
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
		
		 ADC_ResetCalibration(ADC2);//开始校准
    while(ADC_GetResetCalibrationStatus(ADC2));
    ADC_StartCalibration(ADC2);
    while(ADC_GetCalibrationStatus(ADC2));


   DMA_InitTypeDef DMA_InitStructure;
	 DMA_InitStructure.DMA_BufferSize=256;//几个数据
	 DMA_InitStructure.DMA_DIR=DMA_DIR_PeripheralSRC;//传输方向为外设到内存
	 DMA_InitStructure.DMA_M2M=DMA_M2M_Disable;//数据从内存到内存关闭
	 DMA_InitStructure.DMA_MemoryBaseAddr=(uint32_t)Buf;//开始地址
	 DMA_InitStructure.DMA_MemoryDataSize= DMA_MemoryDataSize_HalfWord;//数据位数（8位或更多）这里为16位
   DMA_InitStructure.DMA_MemoryInc= DMA_MemoryInc_Enable;//内存地址是否自增
	 DMA_InitStructure.DMA_Mode=DMA_Mode_Circular;//循环模式 自动持续的搬运数据
	 DMA_InitStructure.DMA_PeripheralBaseAddr=(uint32_t)&ADC1->DR;//外设地址
	 DMA_InitStructure.DMA_PeripheralDataSize=DMA_PeripheralDataSize_HalfWord;//外设数据宽度16位
	 DMA_InitStructure.DMA_PeripheralInc=DMA_PeripheralInc_Disable;//外设地址不自增 （只有ADC的扫描模式用得到）
	 DMA_InitStructure.DMA_Priority=DMA_Priority_Medium;//DMA通道优先级 中等
	 DMA_Init(DMA1_Channel1,&DMA_InitStructure);
	  
	 
   DMA_ITConfig(DMA1_Channel1, DMA_IT_HT | DMA_IT_TC, ENABLE);//中断使能 只有写了这个才会开启中断
	 //并且要选对中断标志位才能正确进中断
	 
	 DMA_Cmd(DMA1_Channel1, ENABLE); 
	 
	 NVIC_InitTypeDef NVIC_InitStructure;
	 NVIC_InitStructure.NVIC_IRQChannel=DMA1_Channel1_IRQn;;
	 NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0; 		
   NVIC_Init(&NVIC_InitStructure);

}

void DMA1_Channel1_IRQHandler(void){

	if(DMA_GetITStatus(DMA1_FLAG_HT1)==SET){
		
	  conut=1;
		
	 DMA_ClearITPendingBit(DMA1_FLAG_HT1);//清除中断标志位
	 //一定要清理中断标志位不然会反复进入中断导致程序卡死
	}
	//中断标志位会进中断，普通标志位不会
	//有进中断的函数和检查标志位的函数 注意区分
	if(DMA_GetITStatus(DMA1_FLAG_TC1)==SET){
		
	   conut=2;
		 DMA_ClearITPendingBit(DMA1_FLAG_TC1);
	}

}
//修改PSC值
void TIM_gai(){
  
  if(KeyNum==4&&x==0) {psc=71; TIM3->PSC=psc;x++;KeyNum=0;}
  if(KeyNum==4&&x==1) {psc=143; TIM3->PSC=psc;x++;KeyNum=0;}
  if(KeyNum==4&&x==2) {psc=719; TIM3->PSC=psc;x++;KeyNum=0;}
  if(KeyNum==4&&x==3) {psc=7199; TIM3->PSC=psc;x++;KeyNum=0;}
	if(KeyNum==4&&x==4) {psc=9; TIM3->PSC=psc;KeyNum=0;x=0;}
}
 /**void ADC_sofa(){
 	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	
 
 }**/
 
 
 