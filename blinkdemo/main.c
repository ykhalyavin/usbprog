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
#include "usbn2mc.h"

/* id need for live update of firmware */

void USBNDecodeVendorRequest(DeviceRequest *req)
{
	//UARTWrite("vendor request check ");
	switch(req->bRequest)
	{
		case STARTAVRUPDATE:
			avrupdate_start();
		break;
	}
}


int main(void)
{
  int conf, interf;
  //UARTInit();

  USBNInit();   
  
  USBNDeviceVendorID(0x1781);
  USBNDeviceProductID(0x0c62);
  USBNDeviceBCDDevice(0x0001);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  
  USBNDeviceManufacture ("Benedikt Sauter - www.ixbat.de  ");
  USBNDeviceProduct	("Blink Demo");
  USBNDeviceSerialNumber("200612261");
 
	DDRA = (1 << DDA4);
  PORTA |= (1<<PA4);	//on
	PORTA &= ~(1<<PA4); //off

  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,NULL);
  
  USBNInitMC();
  sei();

  // start usb chip
  USBNStart();
  
	while(1){
		PORTA |= (1<<PA4);	//on
		wait_ms(500);
		PORTA &= ~(1<<PA4); //off
		wait_ms(500);

	}
}


