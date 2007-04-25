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

#include "simpleport.h"

void set_direction(uint8_t direction)
{
  // 0 = input, 1 = output
  if(direction && 0x01) SETBIT(BIT0_DDR,BIT0); else CLEARBIT(BIT0_DDR,BIT0);
  if(direction && 0x02) SETBIT(BIT1_DDR,BIT1); else CLEARBIT(BIT1_DDR,BIT1);
  if(direction && 0x04) SETBIT(BIT2_DDR,BIT2); else CLEARBIT(BIT2_DDR,BIT2);
  if(direction && 0x08) SETBIT(BIT3_DDR,BIT3); else CLEARBIT(BIT3_DDR,BIT3);
  if(direction && 0x10) SETBIT(BIT4_DDR,BIT4); else CLEARBIT(BIT4_DDR,BIT4);
  if(direction && 0x20) SETBIT(BIT5_DDR,BIT5); else CLEARBIT(BIT5_DDR,BIT5);
  if(direction && 0x40) SETBIT(BIT6_DDR,BIT6); else CLEARBIT(BIT6_DDR,BIT6);
}


void set_port(uint8_t value)
{
  // BIT0 - BIT 3
	//PORTB hinbauen	
	uint8_t port=0;
  if(value && 0x01) SETBIT(port,BIT0);
  if(value && 0x02) SETBIT(port,BIT1);
  if(value && 0x04) SETBIT(port,BIT2);
  if(value && 0x08) SETBIT(port,BIT3);

	// all together
	PORTB = port;

  //BIT4 - BIT 5
  if(value && 0x10) SETBIT(BIT4_WRITE,BIT4); else CLEARBIT(BIT4_WRITE,BIT4);
  if(value && 0x20) SETBIT(BIT5_WRITE,BIT5); else CLEARBIT(BIT5_WRITE,BIT5);

  // BIT 6
  if(value && 0x40) SETBIT(BIT6_WRITE,BIT6); else CLEARBIT(BIT6_WRITE,BIT6);

}

uint8_t get_port()
{
  uint8_t result=0x00; 
  if(IS_BIT0_SET()) result |= (1<<BIT0);
  if(IS_BIT1_SET()) result |= (1<<BIT1);
  if(IS_BIT2_SET()) result |= (1<<BIT2);
  if(IS_BIT3_SET()) result |= (1<<BIT3);
  if(IS_BIT4_SET()) result |= (1<<BIT4);
  if(IS_BIT5_SET()) result |= (1<<BIT5);
  if(IS_BIT6_SET()) result |= (1<<BIT6);
  return result;
}


void set_bit(uint8_t bit, uint8_t value)
{
  switch(bit) {
    case 0: if(value==1) SETBIT(BIT0_WRITE,BIT0); else CLEARBIT(BIT0_WRITE,BIT0); break;
    case 1: if(value==1) SETBIT(BIT1_WRITE,BIT1); else CLEARBIT(BIT1_WRITE,BIT1); break;
    case 2: if(value==1) SETBIT(BIT2_WRITE,BIT2); else CLEARBIT(BIT2_WRITE,BIT2); break;
    case 3: if(value==1) SETBIT(BIT3_WRITE,BIT3); else CLEARBIT(BIT3_WRITE,BIT3); break;
    case 4: if(value==1) SETBIT(BIT4_WRITE,BIT4); else CLEARBIT(BIT4_WRITE,BIT4); break;
    case 5: if(value==1) SETBIT(BIT5_WRITE,BIT5); else CLEARBIT(BIT5_WRITE,BIT5); break;
    case 6: if(value==1) SETBIT(BIT5_WRITE,BIT6); else CLEARBIT(BIT6_WRITE,BIT6); break;
  }

}


uint8_t get_bit(uint8_t bit)
{
  switch(bit) {
    case 0: return IS_BIT0_SET(); break;
    case 1: return IS_BIT1_SET(); break;
    case 2: return IS_BIT2_SET(); break;
    case 3: return IS_BIT3_SET(); break;
    case 4: return IS_BIT4_SET(); break;
    case 5: return IS_BIT5_SET(); break;
    case 6: return IS_BIT6_SET(); break;
  }
}

