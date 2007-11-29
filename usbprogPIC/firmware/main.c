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

//#include "wait.h"
#include "uart.h"

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
    usbprog.datatogl = 0;
int i;
for(i=0;i<64;i++)
  answer[i]=i;
CommandAnswer(64);
}


int main(void)
{
    int conf, interf;

    USBNInit();
    UARTInit();

    usbprog.datatogl = 0;

    USBNDeviceVendorID(0x04d8);
    USBNDeviceProductID(0x0033);

    USBNDeviceBCDDevice(0x0200);

    char lang[]={0x09,0x04};
    _USBNAddStringDescriptor(lang); // language descriptor

    USBNDeviceManufacture ("USBprog EmbeddedProjects");
    USBNDeviceProduct("usbprogPIC Clone");
    USBNDeviceSerialNumber("GNU/GPL2");
    //USBNDeviceManufacture("Microchip Technology Inc."); 
    //USBNDeviceProduct("PICkit 2 Microcontroller Programmer");
    //USBNDeviceSerialNumber("PIC18F2550");
    
    conf = USBNAddConfiguration();

    USBNConfigurationPower(conf,50);

    interf = USBNAddInterface(conf,0);
    USBNAlternateSetting(conf,interf,0);

    USBNAddInEndpoint(conf,interf,1,0x01,BULK,64,0,NULL);
    USBNAddOutEndpoint(conf,interf,1,0x01,BULK,64,0,&Commands);
    
    USBNInitMC();
    sei();

    // start usb chip
    USBNStart();
	


    while(1);
}


