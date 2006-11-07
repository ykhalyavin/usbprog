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
#include "usbn2mc_spi.h"
#include "uart.h"

// ********************************************************************
// This subroutine handles the communication with usbn9604          
// ********************************************************************


char USBNSPITransmit(char data)
{
	/* start transmission */
	SPDR = data;
	/* wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
	return SPDR;
}

void USBNSPIMasterInit()
{
	/* set mosi and sck output, all others input */
	DDR_SPI = (1<<DD_MOSI)|(1<<DD_SCK);

	/* enable spi, master set clockrate fck/16*/
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}

#if 0
char USBNSPIReceive()
{
	/* wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));

	/* return data */
	return SPDR ;

}
void USBNSPISlaveInit()
{
	/* set mosi and sck output, all others input */
	DDR_SPI = (1<<DD_MISO);

	/* enable spi*/
	SPCR = (1<<SPE);
}
#endif

// Read data from usbn96x register

void USBNInitMC(void)
{
  	MCUCR |=  (1 << ISC01); // fallende flanke
  	GICR |= (1 << INT0);

	USBNSPIMasterInit();
}




unsigned char USBNBurstRead(void)
{
  	return 0;
}

unsigned char USBNRead(unsigned char Adr)
{
	Adr &= 0x3F;
	return USBNSPITransmit(Adr);
}



// Write data to usbn96x register
void USBNWrite(unsigned char Adr, unsigned char Data)
{
}


inline void USBNBurstWrite(unsigned char Data)
{
}



void USBNDebug(char *msg)
{
  	UARTWrite(msg);
}

