/**

void TIM_gai(){
	
	uint8_t dan=0;
	uint16_t adc2=ADC_GetConversionValue(ADC2),psc;
	if(adc1-adc2<30) return;//软件销抖
	for(uint8_t i=1;i<=10;i++){
	  uint16_t Max=400;
	if(Max*i>=adc2){ dan=i;goto END;}
	}
	END:if(Current!=dan){
	psc=(uint16_t)(dan*400*0.92*2);
		if(psc>7199)psc=7199;
		if(psc<72)psc=72;
		Current=dan;
		TIM3->PSC=psc;
	}
	
	
	adc1=adc2;
}

void LCD_QIN(uint8_t n, uint8_t y_start, uint16_t* Arrdy) {
   // LCD_Bo(n);
    //LCD_Clear();
   
    uint8_t block;//每8行一浚共16次传输
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
             
                else if(x < 127) {  
                    if((curr < y && next >= y) || (curr > y && next <= y)) {
                        is_wave = 1;//前后有边沿要画线置1
                    }
                }
                
                
                if(is_wave) {
                    line_buf[time][idx++] = 0x00;  
                    line_buf[time][idx++] = 0x00;  //置1后画绿线
                } 
							
                }
            }
        }
        
        SPI_DMA(2048, line_buf[time]);//调用DMA
        time = 1 - time;//切换缓冲数组
        DMAtime = 1;//中断标志位置1
    }
    
  
}
//绘制波形函数，传入数组  和0或128代表开始的数组起点
//帧缓冲数组用于填下一帧屏幕
void LCD_DMA(uint8_t n,uint8_t y,uint16_t* Arrdy){

	   LCD_Bo(n);
	   uint8_t i=0;
	   uint32_t Color=0;
     GPIO_ResetBits(GPIOA, GPIO_Pin_4); // cs低 开始通信
	  LCD_SetRegion1(0, 0, 127, 127);
	    GPIO_SetBits(GPIOA,GPIO_Pin_2);   //DC高发送数据
	LCD_Clear();
    for(;y<128;y++){
		  uint8_t x=0;
			if(Color>=2047)Color=0;
			if(i<=7){
		  for(;x<128;x++){
		    if(Arrdy[n+x]==y){
			    line_buf[time][Color++]=0x07;
			    line_buf[time][Color++]=0xE0;
			   }
			  else{
				  if(Arrdy[n+x+1]-Arrdy[n+x]>30&&Arrdy[n+x+1]>=y){
          line_buf[time][Color++]=0x07;
          line_buf[time][Color++]=0xE0;
          }
          else if(Arrdy[n+x+1]-Arrdy[n+x]<-30&&Arrdy[n+x]>=y){
          line_buf[time][Color++]=0x07;
          line_buf[time][Color++]=0xE0;
          }
          else if(Arrdy[n+x]!=y){
          line_buf[time][Color++]=0x00;
          line_buf[time][Color++]=0x00;
					}
		      }
		}
			i++;
	}
			
			if(i==8){
			while(DMAtime==1);
			SPI_DMA(2048,line_buf[time]);
			time=1-time;
			DMAtime=1;
				i=0;
			}
	    
} 
     GPIO_SetBits(GPIOA, GPIO_Pin_4);//拉高CS 结束通信		
}







void Lcd_Clear(uint16_t color)//清屏函数
{
    uint16_t i,j;
    Lcd_WriteIndex(0x2C); // ?????
    for(i = 0; i < 160; i++)
    {
        for(j = 0; j < 128; j++)
        {
            LCD_WriteData_16Bit(color);
        }
    }
}

		  for(;x<126;x++){
		    if(Arrdy[n+x]==y){
			    line_buf[time][Color++]=0x07;
			    line_buf[time][Color++]=0xE0;
			   }
			  else{
				  if(Arrdy[n+x+1]-Arrdy[n+x]>=0&&Arrdy[n+x+1]>=y){
          line_buf[time][Color++]=0x07;
          line_buf[time][Color++]=0xE0;
          }
          else if(Arrdy[n+x+1]-Arrdy[n+x]<=0&&Arrdy[n+x]>=y){
          line_buf[time][Color++]=0x07;
          line_buf[time][Color++]=0xE0;
          }
          else if(Arrdy[n+x]!=y){
          line_buf[time][Color++]=0x00;
          line_buf[time][Color++]=0x00;
					}
		      }
		}  
	
void LCD_SPI_DMA(uint8_t n){
     uint8_t y=0;
	   uint8_t next_buf[256];
	  
	   GPIO_ResetBits(GPIOA, GPIO_Pin_4); // cs低 开始通信
	   LCD_SetRegion1(0, 0, 127, 128);
	   GPIO_SetBits(GPIOA,GPIO_Pin_2);   //DC高发送数据
	   LCD_DMA(n,y);
	   for(;y<128;y++){
			  uint8_t x=0;
			  for(;x<255;x++){
		    next_buf[x]=line_buf[x];
		    }
	     LCD_DMA(n,y+1);
       
	      while(DMAtime==1);
			  DMAtime=0;
				SPI_DMA(256,next_buf);
				
		 }
	while(DMAtime==1);
				DMAtime=0;
	GPIO_SetBits(GPIOA, GPIO_Pin_4);//拉高CS 结束通信
}


void LCD_SPI_DMA(uint8_t n)
{
    uint8_t y;
    uint8_t next_buf[256];

    GPIO_ResetBits(GPIOA, GPIO_Pin_4);  
    LCD_SetRegion1(0, 0, 127, 127);
    GPIO_SetBits(GPIOA, GPIO_Pin_2);     

    LCD_DMA(n, 0);
    for (int i = 0; i < 256; i++) next_buf[i] = line_buf[i];
	  SPI_DMA(256, next_buf);
    while (DMAtime == 1);   
    DMAtime = 0;
    


    for (y = 1; y < 127; y++)
    {
        LCD_DMA(n, y);    
        for (int i = 0; i < 256; i++) next_buf[i] = line_buf[i];
        while (DMAtime == 1);
        DMAtime = 0;
        SPI_DMA(256, next_buf);
    }

    LCD_DMA(n, 127);
    for (int i = 0; i < 256; i++) next_buf[i] = line_buf[i];
    while (DMAtime == 1);
    DMAtime = 0;
    SPI_DMA(256, next_buf);

    while (DMAtime == 1);
    DMAtime = 0;   
    GPIO_SetBits(GPIOA, GPIO_Pin_4); 
}
	x=0;
  for(;x<128;x++){
	if(Buff[x]>2000){
	LCD_ADC_DMA(&Buff,x);
		   LCD_DMA(x);
	}
	
	}



//把二维数组拆成一维的用于发送 第y行
void LCD_SentDMA(uint8_t y){
   
  
	 uint8_t x=0;
	 uint16_t Color;
	 uint8_t len=0;
	 for(;x<128;x++){
	 
		 Color=Buff[y][x];
		 line_buf[len++]=Color>>8;
		 line_buf[len++]=Color&0xFF;
	 }
}



//绘制波形函数，传入数组  和0或128代表开始的数组起点
//帧缓冲数组用于填下一帧屏幕
void LCD_DMA(uint8_t n){
	 uint8_t y=0;
   for(;y<60;y++){
	   uint8_t x=0;
     for(;x<128;x++){
			 if(Buf[n+x]==y){Buff[y][x]=0x07E0;}
			 else {Buff[y][x]=0x0000;}
		 } 
	} 

	 uint8_t x=0;
	 for(;x<128;x++){
	  if(Buf[n+x+1]-Buf[n+x]>=10)
		{
			 uint8_t y0 = Buf[n + x];
       uint8_t y1 = Buf[n + x + 1];
		 for(;y0<y1&&y0<60;y0++)
		{
		 Buff[y0][x-1]=0x07E0;
		 }
			 
		}
	 if(Buf[n+x+1]-Buf[n+x]<=-10)
		{
			uint8_t y0 = Buf[n + x];
      uint8_t y1 = Buf[n + x + 1];
		 for(;y1<y0&&y1<60;y1++)
		{
		 Buff[y1][x+1]=0x07E0;
		 }
			 
		}
	 }
	 	GPIO_ResetBits(GPIOA, GPIO_Pin_4); // cs低 开始通信
	LCD_SetRegion1(0, 0, 127, 60);

	GPIO_SetBits(GPIOA,GPIO_Pin_2);   
   y=0;
  for(;y<60;y++){
	  LCD_SentDMA(y);
	  SPI_DMA(256,line_buf);
	
	}

	 	GPIO_SetBits(GPIOA, GPIO_Pin_4);//拉高CS 结束通信
}
 











 //画线函数，使用Bresenham 画线算法
//x0 起始x坐标 y0 起始y x1 终坐标 y1 终 color 颜色
void LCD_DrawLine(u16 x0, u16 y0,u16 x1, u16 y1,u16 Color)   
{
int dx,             // difference in x's
    dy,             // difference in y's
    dx2,            // dx,dy * 2
    dy2, 
    x_inc,          // amount in pixel space to move during drawing
    y_inc,          // amount in pixel space to move during drawing
    error,          // the discriminant i.e. error i.e. decision variable
    index;          // used for looping	


	
	dx = x1-x0;//计算x距离
	dy = y1-y0;//计算y距离

	if (dx>=0)
	{
		x_inc = 1;
	}
	else
	{
		x_inc = -1;
		dx    = -dx;  
	} 
	
	if (dy>=0)
	{
		y_inc = 1;
	} 
	else
	{
		y_inc = -1;
		dy    = -dy; 
	} 

	dx2 = dx << 1;
	dy2 = dy << 1;

	if (dx > dy)//x距离大于y距离，那么每个x轴上只有一个点，每个y轴上有若干个点
	{//且线的点数等于x距离，以x轴递增画点
		// initialize error term
		error = dy2 - dx; 

		// draw the line
		for (index=0; index <= dx; index++)//要画的点数不会超过x距离
		{
			//画点
			LCD_DrawPoint(x0,y0,Color);
			
			// test if error has overflowed
			if (error >= 0) //是否需要增加y坐标值
			{
				error-=dx2;

				// move to next line
				y0+=y_inc;//增加y坐标值
			} // end if error overflowed

			// adjust the error term
			error+=dy2;

			// move to the next pixel
			x0+=x_inc;//x坐标值每次画点后都递增1
		} // end for
	} // end if |slope| <= 1
	else//y轴大于x轴，则每个y轴上只有一个点，x轴若干个点
	{//以y轴为递增画点
		// initialize error term
		error = dx2 - dy; 

		// draw the line
		for (index=0; index <= dy; index++)
		{
			// set the pixel
			LCD_DrawPoint(x0,y0,Color);

			// test if error overflowed
			if (error >= 0)
			{
				error-=dy2;

				// move to next line
				x0+=x_inc;
			} // end if error overflowed

			// adjust the error term
			error+=dx2;

			// move to the next pixel
			y0+=y_inc;
		} // end for
	} // end else |slope| > 1
}




SPI_I2S_SendData(SPI1, 0x00);
	  while(DMA_GetFlagStatus(DMA1_FLAG_TC3)==RESET);
	  DMA_ClearFlag(DMA1_FLAG_TC3); 
    DMA_Cmd(DMA1_Channel3,DISABLE);


 uint8_t x=0;
   for(;x<=118;){
	 DMA_Buf[x]=0x07;
	 DMA_Buf[x+1]=0xE0;
	 x=x+2;
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
   Lcd_WriteIndex(0x2a);
   Lcd_WriteData(0x00);
	 Lcd_WriteData(x_start);
	 Lcd_WriteData(0x00);
	 Lcd_WriteData(x_end);
	
   Lcd_WriteIndex(0x2b);
   Lcd_WriteData(0x00);
	 Lcd_WriteData(y_start);
	 Lcd_WriteData(0x00);
	 Lcd_WriteData(y_end);

   Lcd_WriteIndex(0x2c); //发送颜色指令的，记得在调用函数后再发送颜色数据16位的


}
颜色填充数组
void DMA_Buff(){
	 uint8_t x=0;
   for(;x<=118;){
	 DMA_Buf[x]=0x07;
	 DMA_Buf[x+1]=0xE0;
	 x=x+2;
	 }

}

//发送第x列的竖线数据
void LCD_SHU(uint8_t n)
{
	 uint8_t DMA_Buf[120];
	 uint8_t x=0;
	 DMA_Buff();
	 uint8_t line_buf[128];//一列几个像素
   for(;x<128;x++){
	  if(Buf[n+x+1]-Buf[n+x]>=1)
		{
		line_buf[x]=Buf[n+x+1]-Buf[n+x];
		}
		else if(Buf[n+x+1]-Buf[n+x]<=-1)
		{
		line_buf[x]=-(Buf[n+x+1]-Buf[n+x]);
		}
		else if(Buf[n+x+1]==Buf[n+x])
		{
		line_buf[x]=0;
		}
	 }
   x=0;
	 GPIO_ResetBits(GPIOA, GPIO_Pin_4); //拉低cs 开始通信
	 for(;x<127;x++){
		 if(line_buf[x]>0){
			 if(Buf[n+x]>Buf[n+x+1]){
			 LCD_SetRegion1(x+1,0,x+1,Buf[n+x]);
			 //GPIO_ResetBits(GPIOA, GPIO_Pin_4); 
	     GPIO_SetBits(GPIOA,GPIO_Pin_2);   
			 SPI_DMA(line_buf[x]*2,DMA_Buf);
			 }
			 else{
			 LCD_SetRegion1(x-1,0,x-1,Buf[n+x+1]);
			 //GPIO_ResetBits(GPIOA, GPIO_Pin_4); 
	     GPIO_SetBits(GPIOA,GPIO_Pin_2);   
			 SPI_DMA(line_buf[x]*2,DMA_Buf);
			 }
			 	
		 }
	 }
   GPIO_SetBits(GPIOA, GPIO_Pin_4);//拉高CS 结束通信
}
**/