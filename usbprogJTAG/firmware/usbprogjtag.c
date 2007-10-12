/*
 * Copyright (C) 2006 Benedikt Sauter 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "usbprogjtag.h"

void write_and_read(char * buf, uint16_t size)
{
  uint16_t i,j;
  // until byte 3 (0=cmd,1,2=size,3... data)
  uint16_t bit_cnt;
  CLEARPIN(PIN_WRITE,TMS);
  for (bit_cnt = 0; bit_cnt < size; bit_cnt++) {
    // write tdi
    
    // control tdi
    if ((buf[(bit_cnt+24)/8] >> ((bit_cnt+24) % 8)) & 0x1) //tdi 1
      SETPIN(PIN_WRITE,TDI);
    else // tdi 0
      CLEARPIN(PIN_WRITE,TDI);
   /* 
    // control tms line - goes to high at last bit
    if(size != 488){
      if(bit_cnt==(size-1))
	SETPIN(PIN_WRITE,TMS);
      else
	CLEARPIN(PIN_WRITE,TMS);
    }
   */ 
    // clock
    wait_ms(1);
    CLEARPIN(PIN_WRITE,TCK);
    //asm("nop");
    //for(i=0;i<0xFF;i++)asm("nop");
    asm("nop");
    wait_ms(1);
    SETPIN(PIN_WRITE,TCK);

    // read tdo
    if(IS_PIN8_SET())
      buf[(bit_cnt+24)/8] |= 1 << ((bit_cnt+24) % 8);
    else
      buf[(bit_cnt+24)/8] &= ~(1 << ((bit_cnt+24) % 8));
  }
}


void write_tdi(char * buf, uint16_t size)
{

  uint16_t i,j;
  // until byte 3 (0=cmd,1,2=size,3... data)
  uint16_t bit_cnt;
  
  for (bit_cnt = 0; bit_cnt < size; bit_cnt++) {
    // write tdi
    
    // control tdi
    if ((buf[(bit_cnt+24)/8] >> ((bit_cnt+24) % 8)) & 0x1) //tdi 1
      SETPIN(PIN_WRITE,TDI);
    else // tdi 0
      CLEARPIN(PIN_WRITE,TDI);
    // control tms line - goes to high at last bit
    if(size != 488){
      if(bit_cnt==(size-1))
	SETPIN(PIN_WRITE,TMS);
      else
	CLEARPIN(PIN_WRITE,TMS);
    }
    // clock
    CLEARPIN(PIN_WRITE,TCK);
    asm("nop");
    //for(i=0;i<0xFF;i++)asm("nop");
    //asm("nop");
    SETPIN(PIN_WRITE,TCK);
  }
}

void write_tms(uint8_t buf)
{

  uint8_t i;
  CLEARPIN(PIN_WRITE,TDI);
  // until byte 3 (0=cmd,1,2=size,3... data)
  uint8_t tms; 
  for (i = 0; i < 7; i++) {
    // write tdi
    
    // control tdi
    tms = ((buf >> i) & 1); //tdi 1
    if (tms) //tdi 1
      SETPIN(PIN_WRITE,TMS);
    else // tdi 0
      CLEARPIN(PIN_WRITE,TMS);
    
    // clock
    CLEARPIN(PIN_WRITE,TCK);
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    SETPIN(PIN_WRITE,TCK);
  }
 
  // from openocd moved to here
  CLEARPIN(PIN_WRITE,TCK);
  if (tms) //tdi 1
    SETPIN(PIN_WRITE,TMS);
  else // tdi 0
    CLEARPIN(PIN_WRITE,TMS);

}

void read_tdo(char * buf, uint16_t size)
{
  uint16_t i,j;
  // until byte 3 (0=cmd,1,2=size,3... data)
  uint16_t bit_cnt;
  for (bit_cnt = 0; bit_cnt < size; bit_cnt++) {
    
    // control tms line - goes to high at last bit
    if(size != 488){
      if(bit_cnt==(size-1))
	SETPIN(PIN_WRITE,TMS);
      else
	CLEARPIN(PIN_WRITE,TMS);
    }
    
    // clock
    CLEARPIN(PIN_WRITE,TCK);
    asm("nop");
    //for(i=0;i<0xFF;i++)asm("nop");
    //asm("nop");
    SETPIN(PIN_WRITE,TCK);

    // read tdo
    if(IS_PIN8_SET())
      buf[(bit_cnt+24)/8] |= 1 << ((bit_cnt+24) % 8);
    else
      buf[(bit_cnt+24)/8] &= ~(1 << ((bit_cnt+24) % 8));
  }

}





void set_direction(uint8_t direction)
{
  // 0 = input, 1 = output
  if(direction & 0x80) SETPIN(PIN_DDR,PIN1); else CLEARPIN(PIN_DDR,PIN1);
  if(direction & 0x40) SETPIN(PIN_DDR,PIN2); else CLEARPIN(PIN_DDR,PIN2);
  if(direction & 0x20) SETPIN(PIN_DDR,PIN3); else CLEARPIN(PIN_DDR,PIN3);
  if(direction & 0x10) SETPIN(PIN_DDR,PIN4); else CLEARPIN(PIN_DDR,PIN4);
  if(direction & 0x08) SETPIN(PIN_DDR,PIN5); else CLEARPIN(PIN_DDR,PIN5);
  if(direction & 0x04) SETPIN(PIN_DDR,PIN6); else CLEARPIN(PIN_DDR,PIN6);
  if(direction & 0x02) SETPIN(PIN_DDR,PIN7); else CLEARPIN(PIN_DDR,PIN7);
  if(direction & 0x01) SETPIN(PIN_DDR,PIN8); else CLEARPIN(PIN_DDR,PIN8);
}


void set_port(uint8_t value)
{
  // PIN0 - PIN 3
  //PORTB hinbauen	
  if(value & 0x80) SETPIN(PIN_WRITE,PIN1); else CLEARPIN(PIN_WRITE,PIN1);
  if(value & 0x40) SETPIN(PIN_WRITE,PIN2); else CLEARPIN(PIN_WRITE,PIN2);
  if(value & 0x20) SETPIN(PIN_WRITE,PIN3); else CLEARPIN(PIN_WRITE,PIN3);
  if(value & 0x10) SETPIN(PIN_WRITE,PIN4); else CLEARPIN(PIN_WRITE,PIN4);
  if(value & 0x08) SETPIN(PIN_WRITE,PIN5); else CLEARPIN(PIN_WRITE,PIN5);
  if(value & 0x04) SETPIN(PIN_WRITE,PIN6); else CLEARPIN(PIN_WRITE,PIN6);
  if(value & 0x02) SETPIN(PIN_WRITE,PIN7); else CLEARPIN(PIN_WRITE,PIN7);
  if(value & 0x01) SETPIN(PIN_WRITE,PIN8); else CLEARPIN(PIN_WRITE,PIN8);
}

uint8_t get_port()
{
  uint8_t result=0x00; 
  if(IS_PIN1_SET()) result |= 0x80;
  if(IS_PIN2_SET()) result |= 0x40;
  if(IS_PIN3_SET()) result |= 0x20;
  if(IS_PIN4_SET()) result |= 0x10;
  if(IS_PIN5_SET()) result |= 0x08;
  if(IS_PIN6_SET()) result |= 0x04;
  if(IS_PIN7_SET()) result |= 0x02;
  if(IS_PIN8_SET()) result |= 0x01;
  return result;
}


void set_bit(uint8_t bit, uint8_t value)
{
  switch(bit) {
    case 1: if(value==1) SETPIN(PIN_WRITE,PIN1); else CLEARPIN(PIN_WRITE,PIN1); break;
    case 2: if(value==1) SETPIN(PIN_WRITE,PIN2); else CLEARPIN(PIN_WRITE,PIN2); break;
    case 3: if(value==1) SETPIN(PIN_WRITE,PIN3); else CLEARPIN(PIN_WRITE,PIN3); break;
    case 4: if(value==1) SETPIN(PIN_WRITE,PIN4); else CLEARPIN(PIN_WRITE,PIN4); break;
    case 5: if(value==1) SETPIN(PIN_WRITE,PIN5); else CLEARPIN(PIN_WRITE,PIN5); break;
    case 6: if(value==1) SETPIN(PIN_WRITE,PIN6); else CLEARPIN(PIN_WRITE,PIN6); break;
    case 7: if(value==1) SETPIN(PIN_WRITE,PIN7); else CLEARPIN(PIN_WRITE,PIN7); break;
    case 8: if(value==1) SETPIN(PIN_WRITE,PIN8); else CLEARPIN(PIN_WRITE,PIN8); break;
  }
}


uint8_t get_bit(uint8_t bit)
{
  switch(bit) {
    case 1: if(IS_PIN1_SET())return 1; else return 0; break;
    case 2: if(IS_PIN2_SET())return 1; else return 0; break;
    case 3: if(IS_PIN3_SET())return 1; else return 0; break;
    case 4: if(IS_PIN4_SET())return 1; else return 0; break;
    case 5: if(IS_PIN5_SET())return 1; else return 0; break;
    case 6: if(IS_PIN6_SET())return 1; else return 0; break;
    case 7: if(IS_PIN7_SET())return 1; else return 0; break;
    case 8: if(IS_PIN8_SET())return 1; else return 0; break;
  }
}

void tap_shift(char * buf, uint8_t size)
{
  char tmp;
  // until byte 3 (0=cmd,1,2=size,3... data)
  uint8_t bit_cnt=2;
  CLEARPIN(PIN_WRITE,TMS);
  CLEARPIN(PIN_WRITE,TCK);
  for (bit_cnt = 2; bit_cnt < size+2; bit_cnt++) {
    // write tdi
    
    tmp = buf[bit_cnt];
    // read tdo

    if(IS_PIN8_SET())
      buf[bit_cnt] = 0x01; 
    else
      buf[bit_cnt] = 0x00;

    // control tdi
    if (tmp==0x01) //tdi 1
      SETPIN(PIN_WRITE,TDI);
    else // tdi 0
      CLEARPIN(PIN_WRITE,TDI);
   
    // clock
    //wait_ms(1);
    CLEARPIN(PIN_WRITE,TCK);
    SETPIN(PIN_WRITE,TCK);
    //wait_ms(1);
    //asm("nop");
    //asm("nop");
    asm("nop");
    CLEARPIN(PIN_WRITE,TCK);
  }

}


void tap_shift_final(char * buf,uint8_t size)
{
  uint16_t i,j;
  char tmp;
  // until byte 3 (0=cmd,1,2=size,3... data)
  uint16_t bit_cnt;
  for (bit_cnt = 2; bit_cnt < size+2; bit_cnt++) {
    // write tdi
    
    tmp = buf[bit_cnt];
    // read tdo

    if(IS_PIN8_SET())
      buf[bit_cnt] = 1; 
    else
      buf[bit_cnt] = 0;


    // control tdi
    if (tmp==0x01) //tdi 1
      SETPIN(PIN_WRITE,TDI);
    else // tdi 0
      CLEARPIN(PIN_WRITE,TDI);
    
    // control tms line - goes to high at last bit
    if(bit_cnt==(size-1))
      SETPIN(PIN_WRITE,TMS);

    // clock
    wait_ms(1);
    CLEARPIN(PIN_WRITE,TCK);
    SETPIN(PIN_WRITE,TCK);
    wait_ms(1);
    CLEARPIN(PIN_WRITE,TCK);

  }

}

