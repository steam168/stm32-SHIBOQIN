#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "TIM.h "
#include "ADC.h"
#include "SPI.h"
#include "LCD.h"
#include "key.h"
#include "USART.h"

uint16_t Buff [256];//将采集的ADC值拷贝在另一个数组中，防止画面波形撕裂
uint8_t conut=0; //双缓冲切换变量，在中断中切换
uint16_t Buf [256]={0};//声明和定义不能写在同一个文件下
float cha=0;//两竖光标的差值，未在OLED中显示
uint8_t dan=0,zshu=129,yshu=129;//3个光标的定义接收ADC2的值
uint8_t KeyNum=0;//接收按键值
char USART[2048];//发送上位机的数据数组
extern uint16_t psc;//修改PSC值
uint8_t mode=0;//切换程序
extern unsigned char code_logo;//汉字存储数组
extern uint8_t MAXV;//电压最大值
uint8_t Vpp=0;
uint16_t BuF[512];//拷贝数组
uint8_t Do=0;//按键按住冻结模式
int i=0;//
uint8_t wu=0;//切换光标

int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
  TIM_PWM_Init();
	ADC_DMA_Init();
	SPI1_Init();
	Key_Init();
	Lcd_Init();
	LCD_Clear();
	USART_Init1();
	
	
	
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_2);
	
	
 	OLED_ShowString(1,1,"Vpp");
  OLED_ShowString(2,1,"f");
	OLED_ShowString(3,1,"PSC");
	OLED_ShowString(4,1,"mode:");	
	
		uint16_t x=0, y=0,b=0;//触发冻结模式模式2 3
	while(1)
	{
		
		//基础值显示
     OLED_ShowNum(1,4,Vpp,3);
		OLED_ShowNum(3,4,psc,4);	
	  OLED_ShowNum(2,3,TIM_IC(),4); 
		OLED_ShowNum(4,6,mode,1); 
		
		
		//切换光标
		 if(wu==0){zshu=yshu=130;dan=140;}
		 if(wu==1){zshu=yshu=129;dan=(uint8_t)((ADC_GetConversionValue(ADC2)*122U)/4096);}
		 if(wu==2){dan=140;zshu=(uint8_t)((ADC_GetConversionValue(ADC2)*128U)/4096);}
		 if(wu==3){yshu=(uint8_t)((ADC_GetConversionValue(ADC2)*128U)/4096);}
		  KeyNum=Key_GetNum() ;
		 
		 //修改PSC值变换采样率
			TIM_gai();
		 //模式切换
			if(KeyNum==3){KeyNum=0;mode++;if(mode>4){KeyNum=0;mode=0;LCD_Clear();}}
			
			
			//程序前显示汉字
  if(mode==0){
     LCD_DrawBMP(45,10,&code_logo,30,150,0xFFE0);

  }
			
			//第一个程序
if(mode==1){
			if(KeyNum==1){Do=1-Do;KeyNum=0;}
		if(Do==0){
		    if(conut==1){
			 uint8_t x=0;
		   for(;x<128;x++){
		   Buff[x]=Buf[x];
		   }
		   LCD_ADC_DMA(&Buff,0);
			 Vpp=LCD_Vpp(Buff, zshu,yshu);
		   LCD_DMA(0,0,dan,zshu,yshu,&Buff);
			
			
			 conut = 0;
		   }
	     if(conut==2){
			 uint16_t x=127;
		   for(;x<256;x++){
		   Buff[x]=Buf[x];
		   }
		   LCD_ADC_DMA(&Buff,128);
			  Vpp=LCD_Vpp(Buff,zshu,yshu);
		   LCD_DMA(128,0,dan,zshu,yshu,&Buff);
			 
			
       
		   conut = 0;
		 }
if(KeyNum==2){KeyNum=0;wu++;if(wu>3){wu=0;}}			 
	 }
	 		 
		
		if(Do==1){
			
		
		} 
	 
	}



		 USART_sprintf(Buf);
     
	 
	 
	 //第二个程序
	 if(mode==2){
		   if(KeyNum==2){KeyNum=0;wu++;if(wu>3){wu=0;}}		
		 LCD_Clear();
		 LCD_SetXxian(0,dan,128,0xFFE0);
		 if(KeyNum==1){
			 KeyNum=0;
		 	 if(conut==2){
			if(b==0){
			x++;
			if(x>2){x=0;y=0;i=0;}
			for(;y<=x*255;y++){
			BuF[y]=Buf[y];
			}
			for(;i<=x*128;i++){
			if(BuF[i]<=ADC_GetConversionValue(ADC2)&&BuF[i-10]>2000){
			b=1;
			goto END;
			}
			}
			
		 }
		if(b==1){
		  END:LCD_ADC_DMA(&BuF,i-3);
		  LCD_DMA(i-3,0,dan,zshu,yshu,&BuF);
			while(KeyNum==0){ USART_sprintf(Buf); KeyNum=Key_GetNum() ;}
			b=0;
			x=0;
			i=0;
			y=0;
			KeyNum=1;
		}
		}//下降沿触发
			 
	}
	}
	// 
	
if(mode==3){
	   if(KeyNum==2){KeyNum=0;wu++;if(wu>3){wu=0;}}		
	   LCD_Clear();
		 LCD_SetXxian(0,dan,128,0xFFE0);
	if(KeyNum==1){
		KeyNum=0;
			if(conut==2){
				
			if(b==0){
			x++;
			if(x>2){x=0;y=0;i=0;}
			for(;y<=x*255;y++){
			BuF[y]=Buf[y];
			}
			for(;i<=x*128;i++){
			if(BuF[i]>=dan&&BuF[i-5]<20){
			b=1;
			goto END1;
			}
			}
		
		 }
		if(b==1){
		  END1:LCD_ADC_DMA(&BuF,i-5);
		  LCD_DMA(i-5,0,dan,zshu,yshu,&BuF);
			while(KeyNum==0){KeyNum=Key_GetNum();USART_sprintf(Buf);}
			b=0;
			x=0;
			i=0;
			y=0;
			KeyNum=1;
		}
		}//上升沿触发
	}
	}    
	//	
	
	
	if(mode==4){
	
		if(conut==1){
			for(uint16_t i=0;i<128;i++){
			if(Buf[i]>ADC_GetConversionValue(ADC2)){
			  for(uint16_t x=i;x<128+i;x++){
		     Buff[x]=Buf[x];
		    }
				goto END2;
			}
}
			
       END2: LCD_ADC_DMA(&Buff,0);
		   LCD_DMA(0,0,dan,zshu,yshu,&Buff);
			 Vpp=LCD_Vpp(Buff,zshu,yshu);
					
       conut = 0;
		}//边沿触发
				
		if(conut==2){
			for(uint16_t i=128;i<256;i++){
			if(Buf[i]>ADC_GetConversionValue(ADC2)){
			  for(uint16_t x=i;x<128+i;x++){
					if(x<=255){
		     Buff[x]=Buf[x];
					}
					if(x>255){Buff[x]=0;}
		    }
				goto END3;
			}
}
   	
       END3: LCD_ADC_DMA(&Buff,128);
			 Vpp=LCD_Vpp(Buff,zshu,yshu);
		   LCD_DMA(128,0,dan,zshu,yshu,&Buff);
					
       conut = 0;
   
		}//边沿触发
		if(KeyNum==2){KeyNum=0;wu++;if(wu>3){wu=0;}}
		 
	}

	
	
}


}

