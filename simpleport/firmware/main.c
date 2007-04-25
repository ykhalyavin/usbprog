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

#include "../../usbprog_base/firmwarelib/avrupdate.h"
#include "usbn2mc.h"

#define BIT(x)            (1 << (x))
#define SETBIT(addr,x)    (addr |= BIT(x))
#define CLEARBIT(addr,x)  (addr &= ~BIT(x))


#define JTAG_PORT_INIT          DDRB
#define JTAG_PORT_WRITE         PORTB
#define JTAG_PORT_READ          PINB

#define TDI	    PB5
#define TDO	    PB6
#define TMS	    PB0
#define TCK	    PB7

#define LED_PIN     PA4
#define LED_PORT    PORTA

#define LED_ON()     (LED_PORT   |=  (1 << LED_PIN))   // red led
#define LED_OFF()    (LED_PORT   &= ~(1 << LED_PIN))


// check if tdo == 1
#define JTAG_IS_TDO_SET()		    (JTAG_PORT_READ & BIT(TDO))

#define JTAG_SET_TCK()                       SETBIT( JTAG_PORT_WRITE, TCK )
#define JTAG_CLEAR_TCK()                     CLEARBIT( JTAG_PORT_WRITE, TCK )
//
#define JTAG_SET_TMS()                       SETBIT( JTAG_PORT_WRITE, TMS )
#define JTAG_CLEAR_TMS()                     CLEARBIT( JTAG_PORT_WRITE, TMS )

#define JTAG_SET_TDI()                       SETBIT( JTAG_PORT_WRITE, TDI )
#define JTAG_CLEAR_TDI()                     CLEARBIT( JTAG_PORT_WRITE, TDI )

#define JTAG_SET_TCK()                       SETBIT( JTAG_PORT_WRITE, TCK )
#define JTAG_CLEAR_TCK()                     CLEARBIT( JTAG_PORT_WRITE, TCK )


SIGNAL(SIG_UART_RECV)
{
 //Terminal(UARTGetChar());
 //UARTWrite("usbn>");
}


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

void USBToglAndSend(void)
{
  if(usbprog.datatogl == 1) {
    USBNWrite(TXC1, TX_LAST+TX_EN+TX_TOGL);
    usbprog.datatogl = 0;
  } else {
    USBNWrite(TXC1, TX_LAST+TX_EN);
    usbprog.datatogl = 1;
  }
}

void CommandAnswer(int length)
{
  int i;

  USBNWrite(TXC1, FLUSH);
  for(i = 0; i < length; i++)
    USBNWrite(TXD1, answer[i]);

  /* control togl bit */
  USBToglAndSend();
}

/* central command parser */
void USB2Port(char *buf)
{
  LED_ON();
  
  JTAG_PORT_WRITE = buf[1];
  
  if(JTAG_IS_TDO_SET())
    answer[0] = 0x01;
  else
    answer[0] = 0x00;
  
  CommandAnswer(1);

  LED_OFF();
}


int main(void)
{
    int conf, interf;

    USBNInit();

    usbprog.datatogl = 0;   // 1MHz

    DDRA = (1 << PA4);

    USBNDeviceVendorID(0x1781);	//atmel ids
    USBNDeviceProductID(0x0c62); // atmel ids

    USBNDeviceBCDDevice(0x0200);

    char lang[]={0x09,0x04};
    _USBNAddStringDescriptor(lang); // language descriptor

    USBNDeviceManufacture("B.Sauter");
    USBNDeviceProduct("SimplePort");
    USBNDeviceSerialNumber("GNU/GPL2");

    conf = USBNAddConfiguration();

    USBNConfigurationPower(conf,50);

    interf = USBNAddInterface(conf,0);
    USBNAlternateSetting(conf,interf,0);

    USBNAddInEndpoint(conf,interf,1,0x03,BULK,64,0,NULL);
    USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&USB2Port);

    USBNInitMC();
    sei();

    // start usb chip
    USBNStart();

    // led
    DDRA = (1 << PA4);


    // use as output
    JTAG_PORT_INIT |= (1<<TCK)|(1<<TMS)|(1<<TDI);
    // use as input
    JTAG_PORT_INIT &=~(1<<TDO);
    // pullup
    JTAG_PORT_WRITE |= (1<<TDO);


    while(1);
}


