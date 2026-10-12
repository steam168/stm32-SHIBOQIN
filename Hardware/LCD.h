#ifndef __LCD_H
#define __LCD_H


void LCD_SendData();
void Lcd_WriteIndex();
void Lcd_WriteData();
void LCD_WriteData_16Bit();
void Lcd_Reset();
void Lcd_Init();
void LCD_Clear();
void LCD_SetRegion1();
void LCD_SetXY();
void LCD_DrawPoint();
void LCD_DrawLine();
void LCD_SetYxian();
void LCD_SetXxian();
void LCD_ADC_DMA();
void LCD_DMA();
void LCD_Bo();
void LCD_DrawBMP();
uint16_t LCD_Vpp();
#endif
