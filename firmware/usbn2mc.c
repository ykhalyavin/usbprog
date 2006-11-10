/* usbn960x.c
* Copyright (C) 2005  Benedikt Sauter
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

#include <avr/io.h>
#include "usbn2mc.h"
#include "uart.h"

// ********************************************************************
// This subroutine handles the communication with usbn9604          
// ********************************************************************


// Read data from usbn96x register

void USBNInitMC(void)
{
	MCUCR |=  (1 << ISC01); // fallende flanke
   	GICR |= (1 << INT0);

  
  	//USB_CTRL_DDR = 0xf8;
	/* data directions */
	DDRB |= (1<<PF_CS)|(1<<PF_RD);	
	DDRD |= (1<<PF_WR)|(1<<PF_A0);	

	/* inital values  (PORTB RD 1, CS 1), (PORTD = WR 1, A0 0)*/
  	PORTD |= (PF_WR) & ~(PF_A0);
  	PORTB |= (PF_RD | PF_CS);
}



unsigned char USBNBurstRead(void)
{
  //unsigned char result;
                                                                            
  PORTB ^= (PF_CS | PF_RD);
  asm("nop");              // pause for data to get to bus
  asm("nop"); 
  PORTB ^= (PF_CS | PF_RD);

  /* collect result */
  return (PINC)|((PIND & 0x18) << 3);
}


unsigned char USBNRead(unsigned char Adr)
{
  
  	// set as output
  	DDRC |= 0x3f;        // set for output D0 - D5 pin 0 -5 on C
  	DDRD |= 0x18;        // set for output D0 - D5 pin3 and c on D

	// load address
	// alle alten werte muessen bleiben nur pin 0 bis 5 muessen geaendert werden
 	PORTC |= (0x3f & Adr); 
	PORTD |= ((0xC0 & Adr) >> 3);	/* move complete term 3 steps to left */


  	PORTB ^= (PF_CS);  // strobe the CS, WR, and A0 pins
  	PORTD ^= (PF_WR | PF_A0);  // strobe the CS, WR, and A0 pins
 
  	PORTD ^= (PF_WR | PF_A0);  // strobe the CS, WR, and A0 pins
  	PORTB ^= (PF_CS);  // strobe the CS, WR, and A0 pins
  	
  	asm("nop");              // pause for data to get to bus
  	
	// set as input
	DDRC ^= 0x3f;        // set for output D0 - D5 pin 0 -5 on C
  	DDRD ^= 0x18;        // set for output D0 - D5 pin3 and c on D

  	return (USBNBurstRead());// get data off the bus
}



// Write data to usbn96x register
void USBNWrite(unsigned char Adr, unsigned char Data)
{
  	//USB_DATA_OUT = Adr;        // put the address on the bus
  	//USB_DATA_DDR = 0xff;         // set for output
  	// set as output
  	DDRC |= 0x3f;        // set for output D0 - D5 pin 0 -5 on C
  	DDRD |= 0x18;        // set for output D0 - D5 pin3 and c on D

	// load address
	// alle alten werte muessen bleiben nur pin 0 bis 5 muessen geaendert werden
 	PORTC |= (0x3f & Adr); 
	PORTD |= ((0xC0 & Adr) >> 3);	/* move complete term 3 steps to left */

  	//USB_CTRL_PORT ^= (PF_CS | PF_WR | PF_A0);
  	//USB_CTRL_PORT ^= (PF_CS | PF_WR | PF_A0);

  	PORTB ^= (PF_CS);  // strobe the CS, WR, and A0 pins
  	PORTD ^= (PF_WR | PF_A0);  // strobe the CS, WR, and A0 pins
 
  	PORTD ^= (PF_WR | PF_A0);  // strobe the CS, WR, and A0 pins
  	PORTB ^= (PF_CS);  // strobe the CS, WR, and A0 pins
  	

  	USBNBurstWrite(Data);
}


inline void USBNBurstWrite(unsigned char Data)
{
    // load address
	// alle alten werte muessen bleiben nur pin 0 bis 5 muessen geaendert werden
 	PORTC |= (0x3f & Data); 
	PORTD |= ((0xC0 & Data) >> 3);	/* move complete term 3 steps to left */

	//USB_DATA_OUT = Data;       // put data on the bus
   	//USB_CTRL_PORT ^= (PF_CS | PF_WR);
   	//USB_CTRL_PORT ^= (PF_CS | PF_WR);

  	PORTB ^= (PF_CS);  // strobe the CS, WR, and A0 pins
  	PORTD ^= (PF_WR);  // strobe the CS, WR, and A0 pins
 
  	PORTD ^= (PF_WR);  // strobe the CS, WR, and A0 pins
  	PORTB ^= (PF_CS);  // strobe the CS, WR, and A0 pins
  	
}



void USBNDebug(char *msg)
{
  UARTWrite(msg);
}

