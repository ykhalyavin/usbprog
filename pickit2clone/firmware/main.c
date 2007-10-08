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

#define UNKOWN_COMMAND	0x00
#define I2CINIT		0x07  //Initializes the I2C interface pins
#define I2CSTART	0x08  //i2c start condition generator
#define I2CRESTART	0x09  //i2c restart condition generator
#define I2CSENDBYTE	0x0A  //shifts out a byte, msb first, data is latched in on the rising edge
#define I2CRECEIVEBYTE  0x0B  //shifts in a byte, msb first, data is latched in on the rising edge
#define I2CACK		0x0C  //Master Acknowledge generator
#define I2CNACK		0x0D  //Master NO Acknowledge generator
#define I2CRECVACK	0x0E  //Checks for slave Acknowledge
#define I2CSTOP		0x0F  //i2c stop condition generator


#define F_CPU 16000000
#include <util/delay.h>

#include "wait.h"

#include "../../usbprog_base/firmwarelib/avrupdate.h"
#include "usbn2mc.h"

#include "i2ctool.h"

SIGNAL(SIG_UART_RECV)
{
 //Terminal(UARTGetChar());
 //UARTWrite("usbn>");
}


char answer[64];
volatile struct usbprog_t 
{
  volatile int datatogl;
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


  USBNWrite(TXC1, TX_LAST+TX_EN);

  return;
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
  usbprog.datatogl =0 ;
  switch(buf[0]) {
    case I2CINIT:
      //set_direction((uint8_t)buf[1]);
      // call init here
      answer[0] = I2CINIT; 
      answer[1] = 0x00;
    break;
    case I2CSTART:
      //answer[0] = PORT_SET; 
      // call start here
      answer[0] = I2CSTART; 
      answer[1] = 0x00;
    break;
    case I2CSENDBYTE:
      // call send byte
      answer[0] = I2CSENDBYTE; 
      //answer[1] = i2c_sendbye(); we need some response codes
      CommandAnswer(3);
    break;
    default:
      // unkown command
      answer[0] = UNKOWN_COMMAND; 
      answer[1] = 0x00; 
  }


}


int main(void)
{
    int conf, interf;

    USBNInit();

    usbprog.datatogl = 0;   // 1MHz

    //DDRA = (1 << PA4);

    USBNDeviceVendorID(0x1781);	//atmel ids
    USBNDeviceProductID(0x0c62); // atmel ids

    USBNDeviceBCDDevice(0x0200);

    char lang[]={0x09,0x04};
    _USBNAddStringDescriptor(lang); // language descriptor

    USBNDeviceManufacture("USBprog EmbeddedProjects");
    USBNDeviceProduct("I2C Tool");
    USBNDeviceSerialNumber("GNU/GPL2");

    conf = USBNAddConfiguration();

    USBNConfigurationPower(conf,50);

    interf = USBNAddInterface(conf,0);
    USBNAlternateSetting(conf,interf,0);

    USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,NULL);
    USBNAddOutEndpoint(conf,interf,1,0x03,BULK,64,0,&Commands);

    USBNInitMC();
    sei();

    // start usb chip
    USBNStart();
	


    while(1);
}


