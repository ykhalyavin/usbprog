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
#include <avr/interrupt.h>
#include <inttypes.h>

#define F_CPU 16000000
#include <util/delay.h>

#include "wait.h"

#include "../usbprog_base/firmwarelib/avrupdate.h"
#include "uart.h"
#include "usbn2mc.h"

/*** prototypes and global vars ***/
/* send a command back to pc */
void CommandAnswer(int length);
volatile char answer[300];

SIGNAL(SIG_UART_RECV)
{
 //Terminal(UARTGetChar());
 //UARTWrite("usbn>");
}


SIGNAL(SIG_INTERRUPT0)
{
  USBNInterrupt();
}

/* id need for live update of firmware */

void USBNDecodeVendorRequest(DeviceRequest *req)
{
	//UARTWrite("vendor request check ");
	//SendHex(req->bRequest);
	switch(req->bRequest)
	{
		case STARTAVRUPDATE:
			avrupdate_start();
		break;
	}
}


#define DDR_SPI DDRB
#define MOSI PB5
#define MISO PB6
#define SCK PB7
#define RESET PB0


void spi_init()
{
	
	DDR_SPI &=~(1<<MISO);
	//PORTB = (1<<MISO);

	DDR_SPI = (1<<MOSI)|(1<<SCK)|(1<<RESET);
	
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);	//128tel
	//SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|3;
  //SPSR = (0<<SPI2X);
}

void spi_out(char data)
{
  	SPDR = data;
    while ( !(SPSR & (1<<SPIF)) ) ;
}


char spi_in()
{
	SPDR = 0;
	int timeout = 1000;
	while(!(SPSR & (1<<SPIF))){
		timeout--;
		if(timeout==0)
			break;
	}
	return SPDR;
}


void spi_write_program_memory(char data)
{


}

void CommandAnswer(int length)
{
	int i;

 	USBNWrite(TXC1,FLUSH);
	for(i=0;i<length;i++)
		USBNWrite(TXD1,answer[i]);

	/* control togl bit */

	if(usbprog.datatogl==1) {
		USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
		usbprog.datatogl=0;
	} else {
		USBNWrite(TXC1,TX_LAST+TX_EN);
		usbprog.datatogl=1;
	}
}


/* central command parser */
void USBFlash(char *buf)
{
	int i; 
  USBNWrite(TXC1,FLUSH);
		
	switch(buf[0]) {
		case 0x00:
		break;
		}
	}
}


int main(void)
{
  int conf, interf;
  //UARTInit();

  spi_init();
  USBNInit();   
  
  usbprog.longpackage=0;

	DDRA = (1 << DDA4);
  //PORTA |= (1<<PA4);	//on
	PORTA &= ~(1<<PA4); //off

  USBNDeviceVendorID(0x03eb);	//atmel ids
  USBNDeviceProductID(0x2104); // atmel ids
  
  USBNDeviceBCDDevice(0x0200);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  
  USBNDeviceManufacture ("B.Sauter");
  USBNDeviceProduct	("JTAGICE mkII Klon");
  USBNDeviceSerialNumber("0000A00128255");

  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,NULL);
  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&USBFlash);
  
  USBNInitMC();
  sei();

  // start usb chip
  USBNStart();
  while(1);
}


