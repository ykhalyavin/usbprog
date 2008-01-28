/*
 * XSVF Player - A programmer for JTAG enabled devices using XSVF files
 * Copyright (C) 2007 Sven Luetkemeier 
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

#include "defines.h"
#include "wait.h"
#include "xsvfexec/xsvfexec.h"
#include "xsvfexec/host.h"

#include "../../usbprog_base/firmwarelib/avrupdate.h"
#include "usbn2mc.h"

char answer[64];
struct usbprog_t 
{
  int datatogl;
}usbprog;

SIGNAL(SIG_INTERRUPT0)
{
  USBNInterrupt();
}

/* id need for live update of firmware */

void USBNDecodeVendorRequest(DeviceRequest *req)
{
  switch(req->bRequest)
  {
    case STARTAVRUPDATE:
      avrupdate_start();
    break;
  }
}

void CommandAnswer(int length)
{
  int i;

  USBNWrite(TXC1, FLUSH);
  
  for(i = 0; i < length; i++)
    USBNWrite(TXD1, answer[i]);
  
  /* control togl bit */
  if(usbprog.datatogl == 1) {
    USBNWrite(TXC1, TX_LAST+TX_EN+TX_TOGL);
    usbprog.datatogl = 0;
  } else {
    USBNWrite(TXC1, TX_LAST+TX_EN);
    usbprog.datatogl = 1;
  }
}

/* central command parser */
void Commands(char *buf)
{
	answer[1] = buf[0];
	if(buf[0] & 0x80) {
		/* This is a command / non-XSVF packet */
		switch(buf[0]) {
		
		case XSVF_INIT:
			usbprog.datatogl = 0;
			XsvfInit();
			answer[0] = SUCCESS;
			CommandAnswer(2);
			break;
		
		case XSVF_PRGEND:
			XsvfClose();
			answer[0] = SUCCESS;
			CommandAnswer(2);
			break;
		
		default:
			answer[0] = UNKNOWN_COMMAND;
			CommandAnswer(2);
			break;
		}
	} else {
		/* This is an XSVF packet */
		/* TODO: We always assume a buffer size of 64 at the moment.
		 * Can we determine how many bytes we really received?
		 */
		answer[0] = XsvfExec(buf, 64);
		CommandAnswer(2);
	}
}


int main(void)
{
    int conf, interf;
    
    /* set PE as output and to low, other pins to high-Z */
    JTAG_PORT_INIT = (1 << PE);
    JTAG_PORT_WRITE = 0;

    USBNInit();

    usbprog.datatogl = 0;

    USBNDeviceVendorID(VID);
    USBNDeviceProductID(PID);

    USBNDeviceBCDDevice(0x0200);

    char lang[]={0x09,0x04};
    _USBNAddStringDescriptor(lang); // language descriptor

    USBNDeviceManufacture ("USBprog EmbeddedProjects");
    USBNDeviceProduct("usbprogXSVF     ");
    //USBNDeviceSerialNumber("GNU/GPL2");

    conf = USBNAddConfiguration();

    USBNConfigurationPower(conf,50);

    interf = USBNAddInterface(conf,0);
    USBNAlternateSetting(conf,interf,0);

    USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,NULL);
    USBNAddOutEndpoint(conf,interf,1,0x03,BULK,64,0,&Commands);

    USBNInitMC();

    // start usb chip
    USBNStart();
    sei();
	


    while(1);
}


