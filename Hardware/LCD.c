#include "stm32f10x.h"
#include "Delay.h"
#include "SPI.h"


//我的1.8寸TFT屏幕 x 0~128 y 0~160 

//TFT屏幕 地址自增先横后竖
//DC PA2 CS PA4

extern uint16_t Buff[256];//用于解析数据
static uint8_t line_buf[2][2048], Clear[2048];//存放颜色数据供DMA发送
extern uint8_t DMAtime;//中断标志位用于判断中断是否结束
uint8_t time=0;//line_buf数组切换
extern float cha;//差值
uint8_t MAXV;

void LCD_SendData (uint8_t data ){

   SPI_I2S_SendData(SPI1,data);
	 while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==RESET);

}



void Lcd_WriteIndex(uint8_t Index)//发送指令，DC低
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);  
    GPIO_ResetBits(GPIOA,GPIO_Pin_2);        
     LCD_SendData(Index);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET); // ?????
    GPIO_SetBits(GPIOA, GPIO_Pin_4);   
}

void Lcd_WriteData(uint8_t Index)//发送数据，DC高
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);  
    GPIO_SetBits(GPIOA,GPIO_Pin_2);        
     LCD_SendData(Index);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
    GPIO_SetBits(GPIOA, GPIO_Pin_4);   
}
void LCD_WriteData_16Bit(uint16_t Data)//发送颜色数据
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    GPIO_SetBits(GPIOA,GPIO_Pin_2);
    LCD_SendData(Data >> 8);    
    LCD_SendData(Data & 0xFF);  
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
}


void Lcd_Reset(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_1);
    Delay_ms(100);
    GPIO_SetBits(GPIOB, GPIO_Pin_1);
    Delay_ms(50);
}


void Lcd_Init(void)
{
    Lcd_Reset();               // 1. ????,???

    Lcd_WriteIndex(0x11);     // 2. ??????(??????,???)
    Delay_ms(120);

    Lcd_WriteIndex(0x3A);     // 3. ??????:16?RGB565
    Lcd_WriteData(0x05);

    Lcd_WriteIndex(0x36);     // 4. ??????,???????
    Lcd_WriteData(0xC0);

    Lcd_WriteIndex(0x2A);     // 5. ?????(????128)
    Lcd_WriteData(0x00);
    Lcd_WriteData(0x00);      // ???0
    Lcd_WriteData(0x00);
    Lcd_WriteData(0x7F);      // ???127

    Lcd_WriteIndex(0x2B);     // ?????(????160)
    Lcd_WriteData(0x00);
    Lcd_WriteData(0x00);      // ???0
    Lcd_WriteData(0x00);
    Lcd_WriteData(0x9F);      // ???159

    Lcd_WriteIndex(0x29);     // 6. ??????
    GPIO_SetBits(GPIOB, GPIO_Pin_10); // ???
}



//批量窗口 效率最高 后面的所有函数都用这个最好
void LCD_SetRegion(uint8_t x_start,uint8_t y_start,uint8_t x_end,uint8_t y_end){


	 GPIO_ResetBits(GPIOA, GPIO_Pin_4);  //拉低CS持续通信
    GPIO_ResetBits(GPIOA,GPIO_Pin_2);      //DC低发送指令  
     LCD_SendData(0x2a);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);  
	//x轴起始位	
    GPIO_SetBits(GPIOA,GPIO_Pin_2);        //DC高发送数据
     LCD_SendData(0x00);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
			 LCD_SendData(x_start);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET){};
			//x轴结尾
     LCD_SendData(0x00);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
			 LCD_SendData(x_end);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	
	
	
	//发送y轴数据
	  GPIO_ResetBits(GPIOA,GPIO_Pin_2);      //DC低发送指令  
     LCD_SendData(0x2b);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);  
		//y轴起始位	
    GPIO_SetBits(GPIOA,GPIO_Pin_2);        //DC高发送数据
     LCD_SendData(0x00);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
			 LCD_SendData(y_start);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET){} ;
			//y轴结尾
     LCD_SendData(0x00);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
			 LCD_SendData(y_end);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	
			GPIO_ResetBits(GPIOA,GPIO_Pin_2);      //DC低发送指令  
     LCD_SendData(0x2c);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);  
			
			 GPIO_SetBits(GPIOA, GPIO_Pin_4);//结束通信 拉高CS

}

//批量窗口 效率最高 后面的所有函数都用这个最好
void LCD_SetRegion1(uint8_t x_start,uint8_t y_start,uint8_t x_end,uint8_t y_end){


	 //GPIO_ResetBits(GPIOA, GPIO_Pin_4);  //拉低CS持续通信
    GPIO_ResetBits(GPIOA,GPIO_Pin_2);      //DC低发送指令  
     LCD_SendData(0x2a);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);  
	//x轴起始位	
    GPIO_SetBits(GPIOA,GPIO_Pin_2);        //DC高发送数据
     LCD_SendData(0x00);
   while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET){};
			 LCD_SendData(x_start);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET){};
			//x轴结尾
     LCD_SendData(0x00);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
			 LCD_SendData(x_end);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	
	
	
	//发送y轴数据
	  GPIO_ResetBits(GPIOA,GPIO_Pin_2);      //DC低发送指令  
     LCD_SendData(0x2b);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);  
		//y轴起始位	
    GPIO_SetBits(GPIOA,GPIO_Pin_2);        //DC高发送数据
     LCD_SendData(0x00);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
			 LCD_SendData(y_start);
   while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET){} ;
			//y轴结尾
     LCD_SendData(0x00);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
			 LCD_SendData(y_end);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	
			GPIO_ResetBits(GPIOA,GPIO_Pin_2);      //DC低发送指令  
     LCD_SendData(0x2c);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);  
			
		

}
//这个函数是DMA清屏函数，效率更高
void LCD_Clear(){
  
   
	 GPIO_ResetBits(GPIOA, GPIO_Pin_4);//拉低CS 开始通信
    LCD_SetRegion1(0, 0, 127, 160);  //设置显示窗口
    GPIO_SetBits(GPIOA, GPIO_Pin_2);//拉高DC 开始数据传输
	int i=0;
   for(;i<2048;i++){
		 Clear[i]=0x00;
	 }
	 i=0;
	 DMAtime = 0;
   for(;i<=20;i++){
	    while(DMAtime ==1);
		SPI_DMA(2048,Clear);
		DMAtime =1;
	 
 }
	   while (DMAtime == 1); 
	 GPIO_SetBits(GPIOA, GPIO_Pin_4);

}

void LCD_SetXY(uint8_t x,uint8_t y ){

   LCD_SetRegion(x,y,x,y);
}//调用此函数也会直接触发颜色指令，记得在后面补发一个颜色。

//画点函数
void LCD_DrawPoint(uint8_t x,uint8_t y,uint16_t Data){

   LCD_SetXY(x,y);
   LCD_WriteData_16Bit(Data);//颜色命令

}

//画行线 需要传入y的位置，x的起始和结束位置 还有颜色
void LCD_SetXxian(uint8_t x_start,uint8_t y_start,uint8_t x_end,uint16_t Color){

	 uint8_t i=x_start;
	 LCD_SetRegion(x_start,y_start,x_end,y_start);
   for(;i<=x_end;i++)
	{
     LCD_WriteData_16Bit(Color);
      //硬件特性，每次设置好窗口后发送颜色地址自增 显存自增
  }
}




//画列线 需要传入x的位置，y的起始和结束位置 还有颜色
void LCD_SetYxian(uint8_t x_start,uint8_t y_start,uint8_t y_end,uint16_t Color){

	 uint8_t i=y_start;
	 LCD_SetRegion(x_start,y_start,x_start,y_end);
   for(;i<=y_end;i++)
	{
     LCD_WriteData_16Bit(Color);
      //硬件特性，每次设置好窗口后发送颜色地址自增 显存自增
  }
}

//滤波算法，中值滤波法，前后共5个点去掉最高最低，取平均后赋值给该点
void LCD_Bo(uint8_t n){

  uint8_t x=0,Max=0,Min=0,pin=0;
	uint16_t he;
	uint8_t LBuff[128];
	for(;x<125;x++){
	 Max=Buff[n+x];
	 Min=127;
	 uint8_t i=0;
	 he=0,pin=0;
	 if(x>1){
	 for(;i<=4;i++){
	 if(Buff[x-2+i+n]>=Max){
	 Max=Buff[x-2+i+n];
	 }
	 if(Buff[x-2+i+n]<=Min){
	 Min=Buff[x-2+i+n];
	 }
	 he=he+Buff[x-2+i+n];
	 }
	 pin=(uint8_t)((he-Max-Min)/3);
	 LBuff[x]=pin;
	}
	else{
	for(;i<=2;i++){
	 if(Buff[x+i]>=Max){
	 Max=Buff[x+i];
	 }
	 if(Buff[x+i+n]<=Min){
	 Min=Buff[x+i+n];
	 }
	 he=he+Buff[x+i+n];
	 }
	 pin=(uint8_t)((he-Max-Min+(2-x)*Buff[x+n])/3);
	 LBuff[x]=pin;
	}
 }
uint8_t a=0;
for(;a<=127;a++){
    Buff[a+n]=LBuff[a];
}


}


//传入数组和数组起始位置 转换函数使ADC采样值符合屏幕的点
void LCD_ADC_DMA(uint16_t* Arrdy,uint8_t n){
	 uint8_t i=0;
   for(;i<128;i++){
	 Arrdy[n+i]=(uint8_t)((Arrdy[n+i]*120U)/4096);
	 }
}

//绘制波形函数，传入数组  和0或128代表开始的数组起点
//帧缓冲数组用于填下一帧屏幕
void LCD_DMA(uint8_t n, uint8_t y_start,uint8_t chu, uint8_t zshu,uint8_t yshu,uint16_t* Arrdy) {
   // LCD_Bo(n);
    LCD_Clear();
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);//拉低CS 开始通信
    LCD_SetRegion1(0, 0, 127, 129);  //设置显示窗口
    GPIO_SetBits(GPIOA, GPIO_Pin_2);//拉高DC 开始数据传输
    uint8_t block;//每8行一段共16次传输
    for(block = 0; block < 16; block++) {
        uint8_t row_base = block * 8;//上一次结束行
        
        while(DMAtime == 1);  //等待上一个8行传输完毕
        
        uint16_t idx = 0; 
        
        for(uint8_t row = 0; row < 8; row++) //共8行
			{
            uint8_t y = row_base + row;//定义起始行
            
            for(uint8_t x = 0; x < 128; x++) {  
                uint8_t is_wave = 0;//定义是否要画线
                uint8_t curr = (uint8_t)Arrdy[n + x];      
                uint8_t next = (uint8_t)Arrdy[n + x + 1]; //定义前后数组大小
                
              
                if(curr == y) {
                    is_wave = 1;//这个x要画线置1
                }
								
								else if(y==chu){
									is_wave=2;
								}
               else if(zshu==x|yshu==x){
								 is_wave = 3;
								}
								
                else if(x <127) {  
                    if((curr < y && next >= y) || (curr > y && next <= y)) {
                        is_wave = 1;//前后有边沿要画线置1
									
                    }
                }
                
                
                if(is_wave==1) {
                    line_buf[time][idx++] = 0x07;  
                    line_buf[time][idx++] = 0xE0;  //置1后画绿线
                } 
								if(is_wave==2){
								line_buf[time][idx++] = 0xFF;  
                line_buf[time][idx++] = 0xE0;  //置2后画黄线
								}
								if(is_wave==3){
								line_buf[time][idx++] = 0xFF;  
                line_buf[time][idx++] = 0xE0;  //置3后画黄线
								}
								if(is_wave!=1&&is_wave!=2&&is_wave!=3) {
                    line_buf[time][idx++] = 0x00;  
                    line_buf[time][idx++] = 0x00;  //否则画黑点
                }
            }
        }
        
        SPI_DMA(2048, line_buf[time]);//调用DMA
        time = 1 - time;//切换缓冲数组
        DMAtime = 1;//中断标志位置1
    }
    
    GPIO_SetBits(GPIOA, GPIO_Pin_4);//拉高CS 结束通信
}
uint16_t LCD_Vpp(uint16_t* Arrdy,uint8_t zshu,uint8_t yshu){

	 uint16_t i=0,MAX=0,Min=127;
	 float Vpp=0;
   for(;i<=255;i++){
	 if(MAX<=Arrdy[i]) MAX=Arrdy[i];
	 if(Min>=Arrdy[i]) Min=Arrdy[i];
	 }
	  cha=(float)Arrdy[zshu]-(float)Arrdy[yshu];
	 MAXV=MAX/127*3.3f;
   return Vpp=MAX/127*3.3f-Min/127*3.3f;
}


//软件 画图函数，x，y为地址   还有就是数组 w，h是宽高，最后是颜色
void LCD_DrawBMP(uint16_t x,uint16_t y,unsigned char *pic,uint16_t w,uint16_t h,uint16_t color)
{
	uint16_t i,j;
	uint16_t byteCnt=0;
	uint8_t bitMask;
	uint16_t line_bytes = (w + 7)/8;
	for(j=0;j<h;j++)
	{
		uint16_t real_line = h - 1 - j;  
		byteCnt = real_line * line_bytes;

		for(i=0;i<w;i++)
		{
			bitMask = 0x80 >> (i%8); 
			if(pic[byteCnt] & bitMask)
			{
			   
				uint16_t draw_x = x + (w - 1 - i); 
				LCD_DrawPoint(draw_x,y+j,color);
			}
			if(i%8 ==7) byteCnt++;
		}
	}
}


unsigned char code_logo[] = {
	
0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x03,0x80,0x70,0x00,0x03,0x04,0x60,0x40,0x07,0xFE,0xFF,0xE0,0x04,0x40,0x88,0x00,0x08,0x61,0x04,0x00,0x10,0x22,0x04,0x00,0x21,0x04,0x00,0x00,0x01,0x8F,0xFF,0xC0,0x00,0xC0,0x00,0xC0,0x0C,0xC0,0x00,0x80,0x06,0x40,0x00,0x80,0x04,0x00,0x00,0x80,0x04,0x1F,0xF0,0x80,0x04,0x10,0x60,0x80,
0x04,0x10,0x60,0x80,0x04,0x10,0x60,0x80,0x04,0x1F,0xE0,0x80,0x04,0x10,0x60,0x80,0x04,0x10,0x60,0x80,0x04,0x10,0x60,0x80,0x04,0x1F,0xE0,0x80,0x04,0x10,0x60,0x80,0x04,0x10,0x00,0x80,0x04,0x00,0x00,0x80,0x04,0x00,0x0F,0x80,0x0C,0x00,0x03,0x80,0x0C,0x00,0x01,0x00,0x00,0x00,0x00,0x00,/*"?",0*/

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0xFF,0xFE,0x00,0x00,0xC0,0x04,0x00,0x00,0xC0,0x04,0x00,0x00,0xC0,0x04,0x00,0x00,0xFF,0xFC,0x00,0x00,0xC0,0x04,0x00,0x00,0xC0,0x04,0x00,0x00,0xC0,0x04,0x00,0x00,0xFF,0xFC,0x00,0x00,0xE0,0x04,0x00,0x00,0x30,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0xFF,0xFF,0xC0,
0x00,0xC6,0x10,0xC0,0x01,0x0C,0x30,0xC0,0x02,0x0C,0x30,0x80,0x0C,0x18,0x60,0x80,0x10,0x30,0x61,0x80,0x00,0x60,0xC1,0x80,0x00,0xC1,0x81,0x80,0x03,0x01,0x81,0x80,0x0C,0x03,0x01,0x00,0x00,0x0C,0x43,0x00,0x00,0x18,0x3F,0x00,0x00,0xE0,0x0E,0x00,0x03,0x00,0x08,0x00,0x00,0x00,0x00,0x00,/*"?",1*/

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x03,0x80,0x07,0xFF,0xFF,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x3F,0xFF,0xFF,0xF0,0x00,0x03,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0xC3,0x10,0x00,
0x00,0xE3,0x08,0x00,0x01,0xC3,0x04,0x00,0x01,0x83,0x06,0x00,0x03,0x03,0x03,0x00,0x03,0x03,0x01,0x80,0x06,0x03,0x00,0xC0,0x0C,0x03,0x00,0xE0,0x18,0x03,0x00,0x60,0x10,0x03,0x00,0x60,0x20,0x43,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x0E,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,/*"?",2*/

0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x08,0x00,0x30,0x00,0x06,0x00,0x30,0x00,0x03,0x00,0x30,0x00,0x03,0x00,0x30,0x00,0x03,0x30,0x30,0x40,0x00,0x3F,0xFF,0xE0,0x00,0x58,0x30,0x60,0x20,0x58,0x30,0x80,0x30,0x58,0x31,0x00,0x18,0x98,0x30,0x00,0x0C,0x98,0x30,0x00,0x09,0x18,0x30,0x80,0x01,0x1F,0xFF,0xC0,0x01,0x19,0x01,0x80,
0x02,0x19,0x01,0x80,0x02,0x18,0x83,0x00,0x06,0x18,0x83,0x00,0x04,0x10,0x46,0x00,0x3C,0x10,0x64,0x00,0x0C,0x30,0x38,0x00,0x0C,0x20,0x18,0x00,0x0C,0x20,0x3C,0x00,0x0C,0x40,0x66,0x00,0x0C,0xC1,0x83,0x80,0x1C,0x83,0x01,0xE0,0x0D,0x0C,0x00,0xE0,0x02,0x70,0x00,0x00,0x00,0x00,0x00,0x00,/*"?",3*/

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x08,0x01,0x00,0x07,0xFC,0x7F,0x80,0x06,0x08,0x41,0x80,0x06,0x08,0x41,0x80,0x06,0x08,0x41,0x80,0x06,0x08,0x41,0x80,0x07,0xF8,0x7F,0x80,0x06,0x0E,0x51,0x80,0x04,0x07,0x0C,0x00,0x00,0x0C,0x04,0x60,0x3F,0xFF,0xFF,0xF0,0x00,0x18,0x40,0x00,0x00,0x30,0x20,0x00,
0x00,0x60,0x18,0x00,0x00,0xC0,0x0E,0x00,0x03,0x08,0x03,0xF0,0x0F,0xFC,0xFF,0xE0,0x32,0x08,0xC1,0xA0,0x02,0x08,0xC1,0x80,0x02,0x08,0xC1,0x80,0x02,0x08,0xC1,0x80,0x02,0x08,0xC1,0x80,0x03,0xF8,0xFF,0x80,0x02,0x08,0xC1,0x80,0x02,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"?",4*/



};
