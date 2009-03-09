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


#define I2C_PORT PORTB
#define I2C_DDR  DDRB
#define I2C_SDA  PB5
#define I2C_SCL  PB6

/*** prototypes and global vars ***/
/* send a command back to pc */

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
void USBFlash(char *buf)
{
}


int main(void)
{
  int conf, interf;
  UARTInit();

  USBNInit();   
  
  DDRA = (1 << DDA4);
  //PORTA |= (1<<PA4);	//on
  PORTA &= ~(1<<PA4); //off

  USBNDeviceVendorID(0x1781);	//atmel ids
  USBNDeviceProductID(0x0c62); // atmel ids
  
  USBNDeviceBCDDevice(0x0001);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  
  USBNDeviceManufacture ("B.Sauter");
  USBNDeviceProduct	("I2CSNIFFER");

	//0000A0016461 (aktuelle)
	//0000A0019647
	//0000A0000252

  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,NULL);
  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&USBFlash);
  
  USBNInitMC();
  // start usb chip
  USBNStart();
  sei();

  wait_ms(1000);
	PORTA |= (1<<PA4);  //on
	wait_ms(100);
	PORTA &= ~(1<<PA4); //off


  #define ACTIVE   1
  #define INACTIVE 0

	#define BIT_LOW     0
	#define BIT_HIGH    1
  #define BIT_INVALID 2

	// SDA, SCL Input
	I2C_DDR &= (1 << I2C_SDA | 1 << I2C_SCL);

  char i2c = I2C_PORT;

	struct {
    char sda;
    char scl;
    char sda_last;
    char scl_last;
		char status;
		char i;
		char cur_bit;
		char byte;
	} state;

	state.byte     = 0;
	state.cur_bit  = BIT_INVALID;
	state.sda_last = (1 << I2C_SDA) & i2c;
	state.scl_last = (1 << I2C_SCL) & i2c;

  while(1)
	{
		// get current levels of SDA and SCL
    i2c = I2C_PORT; 
		state.sda = (1 << I2C_SDA) & i2c;
		state.scl = (1 << I2C_SCL) & i2c;

		// transmission in progress
		if(ACTIVE)
		{
			// SCL is currently High
      if(state.scl)
			{
				// SCL was High last time
        if(state.scl_last)
				{
					// test, if SDA has changed during clock and BIT is invalid
					if(state.cur_bit != BIT_INVALID && state.cur_bit != state.sda)
				    state.cur_bit = BIT_INVALID;
				}
				// SCL was not High last time
				else
				{
					// handle last bit
				  if(state.cur_bit != BIT_INVALID)
					{
						state.byte |= state.cur_bit << (8-state.i); // MSB first
						if(state.i == 8)
						{
							// byte ausgeben
						}
						else if(state.i == 9)
						{
							state.byte = 0;
							state.i    = 0;
							// ACK: lo; NACK: hi
						}
						else
							state.i++;
					}

					state.cur_bit = I2C_SDA;
				}
			}
		}

		if(state.scl && state.scl_last)
		{
			if(state.scl_last && !state.scl)
			{
        // start: SCL High, SDA von High auf Low
				state.status = ACTIVE;
				state.i = 0;
			}
			else if(!state.scl_last && state.scl)
        // stop: SCL High, SDA von Low auf High
				state.status = INACTIVE;
		}

		state.sda_last = state.sda;
		state.scl_last = state.scl;
	}
}
