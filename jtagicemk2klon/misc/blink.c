/*
 * usbprog - A Downloader/Uploader for AVR device programmers
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

#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <stdint.h>
#include <inttypes.h>
#include <util/delay.h>

int main(void)
{
	DDRB = (1 << DDB0);
	int i;
	Testfunc();
	while(1){
		PORTB &= ~(1<<PB0); //off
		for(i=0;i<10;i++)
		  _delay_ms(0xff);
		PORTB |= (1<<PB0); //on
		for(i=0;i<10;i++)
		  _delay_ms(0xff);
	}	

}


void Testfunc()
{
  int muh;
  int mampf;

  if(muh > 200)
    mampf = 4;
  else
    mampf = 0;
}



