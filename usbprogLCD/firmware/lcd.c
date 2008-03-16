#define  F_CPU   16000000

#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>
#include "simpleport.h"

#include "lcd.h"


#define  P_LCD_RW   PB1
#define  DP_LCD_RW  DDB1
#define  P_LCD_RS   PB2
#define  DP_LCD_RS  DDB2
#define  P_LCD_E    PB3
#define  DP_LCD_E   DDB3
#define  P_LCD_DATA PORTB
#define  DP_LCD_DATA DDRB
#define  LCD_DATA(x) (P_LCD_DATA = (P_LCD_DATA & (~0xF0)) | ((x) << 4))
#define  LCD_DATA_DIR(x) (DP_LCD_DATA = (DP_LCD_DATA & (~0xF0)) | ((x) << 4))


#define SET_LCD_E  SETBIT(PORTB, P_LCD_E)
#define CLR_LCD_E  CLEARBIT(PORTB, P_LCD_E)
#define SET_LCD_RS SETBIT(PORTB, P_LCD_RS)
#define CLR_LCD_RS CLEARBIT(PORTB, P_LCD_RS)
#define SET_LCD_RW SETBIT(PORTB, P_LCD_RW)
#define CLR_LCD_RW CLEARBIT(PORTB, P_LCD_RW)


void LCD_data(unsigned char dat);
void LCD_cmd(unsigned char cmd);
inline void LCD_clock(void);



void LCD_init(void)
{
   unsigned char i;

   CLR_LCD_E;
   SETBIT(DDRB, DP_LCD_E);
   CLR_LCD_RS;
   SETBIT(DDRB, DP_LCD_RS);
   CLR_LCD_RW;
   SETBIT(DDRB, DP_LCD_RW);
   LCD_DATA_DIR(0x0F);

   for(i = 0; i < 10; i++)
   {
      _delay_ms(10);
   }
   LCD_DATA(0x03);     // enable 4-bit LCD mode
   LCD_clock();
   _delay_ms(5);
   LCD_DATA(0x03);
   LCD_clock();
   _delay_ms(5);
   LCD_DATA(0x03);
   LCD_clock();
   _delay_ms(5);
   LCD_DATA(0x02);
   LCD_clock();
   _delay_ms(5);

   if(LCD_COLS > 1)    // configuration
      LCD_cmd(0x28);
   else
      LCD_cmd(0x20);
   _delay_us(40);

   LCD_cmd(0x04);      // entry mode
   _delay_us(40);

   LCD_cmd(0x01);      // clear display
   _delay_ms(2);

   LCD_cmd(0x0C);      // Display ON
   _delay_us(40);

   LCD_cursor(0, 0);   // goto position (0,0)
}


void LCD_text(char *buf)
{
   while(*buf)
   {
      LCD_data(*buf);
      buf++;
   }
}


void LCD_char(char c)
{
   LCD_data(c);
}


void LCD_cursor(unsigned char line, unsigned char column)
{
   unsigned char l = 0x00;
   unsigned char offset = column;

   if((line == 1) || (line == 3))
   {
      l = 0x40;
   }
   if((line == 2) || (line == 3))
   {
      offset += LCD_COLS;
   }
   LCD_cmd(0x80 | l | offset);
}



void LCD_specialchars(void)
{
   unsigned char i, k;

   for(i = 0; i < 8; i++)
   { 
      LCD_cmd(0x40 | (i*8));
      for(k = 0; k < (7-i); k++)
      {
         LCD_data(0x00);
      }
      for(k; k < 8; k++)
      {
         LCD_data(0x1F);
      }
   }
}


void LCD_hex(unsigned char value)
{
   unsigned char temp;

   temp = value >> 4;
   if(temp > 9)
      temp += 55;
   else
      temp += '0';
   LCD_char(temp);

   temp = value & 0x0F;
   if(temp > 9)
      temp += 55;
   else
      temp += '0';
   LCD_char(temp);
}



inline void LCD_clock(void)
{
   _delay_us(40);
   SET_LCD_E;
   _delay_ms(1);
   CLR_LCD_E;
   _delay_us(40);
}


void LCD_data(unsigned char dat)
{
   SET_LCD_RS;
   LCD_DATA(dat >> 4);
   LCD_clock();
   LCD_DATA(dat & 0x0F);
   LCD_clock();
   _delay_us(40);
}


void LCD_cmd(unsigned char cmd)
{
   CLR_LCD_RS;
   LCD_DATA(cmd >> 4);
   LCD_clock();
   LCD_DATA(cmd & 0x0F);
   LCD_clock();
   _delay_ms(1);
}
