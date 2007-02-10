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

/* command descriptions for mk2 */
//#include "spi.h"


/*** prototypes and global vars ***/
/* send a command back to pc */
void CommandAnswer(int length);

volatile struct usbprog_t {
	char lastcmd;
	int longpackage;
	int cmdpackage;
	int datatogl;
} usbprog;

volatile char answer[300];

struct pgmmode_t {
	unsigned short numbytes;
	uint8_t mode;
	uint8_t delay;
	uint8_t cmd1;
	uint8_t cmd2;
	uint8_t cmd3;
	uint8_t poll1;
	uint8_t poll2;
	uint32_t address;
} pgmmode;


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


int main(void)
{
  int conf, interf;
  //UARTInit();

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
  USBNDeviceProduct	("Blink Demo");
  USBNDeviceSerialNumber("");

	//0000A0016461 (aktuelle)
	//0000A0019647
	//0000A0000252

  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,NULL);
  //USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&USBFlash);
  
  USBNInitMC();
  sei();

  // start usb chip
  USBNStart();
  while(1);
}


