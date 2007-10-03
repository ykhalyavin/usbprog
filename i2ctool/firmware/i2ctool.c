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

#include "i2ctool.h"

void set_direction(uint8_t direction)
{
  // 0 = input, 1 = output
  if(direction & 0x01) SETBIT(IO1_DDR,IO1); else CLEARBIT(IO1_DDR,IO1);
  if(direction & 0x02) SETBIT(IO2_DDR,IO2); else CLEARBIT(IO2_DDR,IO2);
  if(direction & 0x04) SETBIT(IO3_DDR,IO3); else CLEARBIT(IO3_DDR,IO3);
  if(direction & 0x08) SETBIT(IO4_DDR,IO4); else CLEARBIT(IO4_DDR,IO4);
  if(direction & 0x10) SETBIT(IO5_DDR,IO5); else CLEARBIT(IO5_DDR,IO5);
  if(direction & 0x20) SETBIT(IO6_DDR,IO6); else CLEARBIT(IO6_DDR,IO6);
  if(direction & 0x40) SETBIT(IO7_DDR,IO7); else CLEARBIT(IO7_DDR,IO7);
  if(direction & 0x80) SETBIT(IO8_DDR,IO8); else CLEARBIT(IO8_DDR,IO8);
}


void set_pin_dir(uint8_t pin, uint8_t value)
{
  switch(pin) {
    case 1: if(value==1) SETBIT(IO1_DDR,IO1); else CLEARBIT(IO1_DDR,IO1); break;
    case 2: if(value==1) SETBIT(IO2_DDR,IO2); else CLEARBIT(IO2_DDR,IO2); break;
    case 3: if(value==1) SETBIT(IO3_DDR,IO3); else CLEARBIT(IO3_DDR,IO3); break;
    case 4: if(value==1) SETBIT(IO4_DDR,IO4); else CLEARBIT(IO4_DDR,IO4); break;
    case 5: if(value==1) SETBIT(IO5_DDR,IO5); else CLEARBIT(IO5_DDR,IO5); break;
    case 6: if(value==1) SETBIT(IO6_DDR,IO6); else CLEARBIT(IO6_DDR,IO6); break;
    case 7: if(value==1) SETBIT(IO7_DDR,IO7); else CLEARBIT(IO7_DDR,IO7); break;
    case 8: if(value==1) SETBIT(IO8_DDR,IO8); else CLEARBIT(IO8_DDR,IO8); break;
    case 9: if(value==1) SETBIT(IO9_DDR,IO9); else CLEARBIT(IO9_DDR,IO9); break;
    case 10: if(value==1) SETBIT(IO10_DDR,IO10); else CLEARBIT(IO10_DDR,IO10); break;
    case 11: if(value==1) SETBIT(IO11_DDR,IO11); else CLEARBIT(IO11_DDR,IO11); break;
    default:
      ;
  }
}


void set_port(uint8_t value, uint8_t mask)
{
  // build port B pattern

  // BIT0 - BIT 3
  //PORTB hinbauen	
  uint8_t port=0;
  if(value & 0x80) port |= (1<<IO1);
  if(value & 0x40) port |= (1<<IO2);
  if(value & 0x20) port |= (1<<IO3);
  if(value & 0x10) port |= (1<<IO4);
  if(value & 0x08) port |= (1<<IO5);
  if(value & 0x04) port |= (1<<IO6);
  if(value & 0x02) port |= (1<<IO7);
  if(value & 0x01) port |= (1<<IO8);

  
  // all together
  PORTB = port;
}

uint8_t get_port()
{
  uint8_t result=0x00; 
  if(IS_IO1_SET()) result |= 0x80;
  if(IS_IO2_SET()) result |= 0x40;
  if(IS_IO3_SET()) result |= 0x20;
  if(IS_IO4_SET()) result |= 0x10;
  if(IS_IO5_SET()) result |= 0x08;
  if(IS_IO6_SET()) result |= 0x04;
  if(IS_IO7_SET()) result |= 0x02;
  if(IS_IO8_SET()) result |= 0x01;
  return result;
}


void set_pin(uint8_t pin, uint8_t value)
{
  switch(pin) {
    case 1: if(value==1) SET_IO1(); else CLEAR_IO1(); break;
    case 2: if(value==1) SET_IO2(); else CLEAR_IO2(); break;
    case 3: if(value==1) SET_IO3(); else CLEAR_IO3(); break;
    case 4: if(value==1) SET_IO4(); else CLEAR_IO4(); break;
    case 5: if(value==1) SET_IO5(); else CLEAR_IO5(); break;
    case 6: if(value==1) SET_IO6(); else CLEAR_IO6(); break;
    case 7: if(value==1) SET_IO7(); else CLEAR_IO7(); break;
    case 8: if(value==1) SET_IO8(); else CLEAR_IO8(); break;
    case 9: if(value==1) SET_IO9(); else CLEAR_IO9(); break;
    case 10: if(value==1) SET_IO10(); else CLEAR_IO10(); break;
    case 11: if(value==1) SET_IO11(); else CLEAR_IO11(); break;
    default:
      ;
  }
}


uint8_t get_pin(uint8_t pin)
{
  switch(pin) {
    case 1: if(IS_IO1_SET())return 1; else return 0; break;
    case 2: if(IS_IO2_SET())return 1; else return 0; break;
    case 3: if(IS_IO3_SET())return 1; else return 0; break;
    case 4: if(IS_IO4_SET())return 1; else return 0; break;
    case 5: if(IS_IO5_SET())return 1; else return 0; break;
    case 6: if(IS_IO6_SET())return 1; else return 0; break;
    case 7: if(IS_IO7_SET())return 1; else return 0; break;
    case 8: if(IS_IO8_SET())return 1; else return 0; break;
    case 9: if(IS_IO9_SET())return 1; else return 0; break;
    case 10: if(IS_IO10_SET())return 1; else return 0; break;
    case 11: if(IS_IO11_SET())return 1; else return 0; break;
    default:
      ;
  }
}

