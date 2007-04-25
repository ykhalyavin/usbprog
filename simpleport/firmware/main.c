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

#define BIT0	    PB5
#define BIT1  	    PB0
#define BIT2	    PB7
#define BIT3	    PB6

#define BIT4	    PD0
#define BIT5	    PD1

#define BIT6	    PA4


#define BIT0_DDR    DDRB
#define BIT1_DDR    DDRB
#define BIT2_DDR    DDRB
#define BIT3_DDR    DDRB

#define BIT4_DDR    DDRD
#define BIT5_DDR    DDRD
#define BIT6_DDR    DDRA


#define BIT0_READ   PINB
#define BIT1_READ   PINB
#define BIT2_READ   PINB
#define BIT3_READ   PINB

#define BIT4_READ   PIND
#define BIT5_READ   PIND
#define BIT6_READ   PINA


#define BIT0_WRITE   PORTB
#define BIT1_WRITE   PORTB
#define BIT2_WRITE   PORTB
#define BIT3_WRITE   PORTB

#define BIT4_WRITE   PORTD
#define BIT5_WRITE   PORTD
#define BIT6_WRITE   PORTA

// check if tdo == 1
#define IS_BIT0_SET()			    (BIT0_READ & BIT(BIT0))
#define IS_BIT1_SET()			    (BIT1_READ & BIT(BIT1))
#define IS_BIT2_SET()			    (BIT2_READ & BIT(BIT2))
#define IS_BIT3_SET()			    (BIT3_READ & BIT(BIT3))

#define IS_BIT4_SET()			    (BIT4_READ & BIT(BIT4))
#define IS_BIT5_SET()			    (BIT5_READ & BIT(BIT5))
#define IS_BIT6_SET()			    (BIT6_READ & BIT(BIT6))


#define SET_BIT0()			     SETBIT( BIT0_WRITE, BIT0 )
#define SET_BIT1()			     SETBIT( BIT1_WRITE, BIT1 )
#define SET_BIT2()			     SETBIT( BIT2_WRITE, BIT2 )
#define SET_BIT3()			     SETBIT( BIT3_WRITE, BIT3 )

#define SET_BIT4()			     SETBIT( BIT4_WRITE, BIT4 )
#define SET_BIT5()			     SETBIT( BIT5_WRITE, BIT5 )
#define SET_BIT6()			     SETBIT( BIT6_WRITE, BIT6 )

#define CLEAR_BIT0()			     CLEARBIT( BIT0_WRITE, BIT0 )
#define CLEAR_BIT1()			     CLEARBIT( BIT1_WRITE, BIT1 )
#define CLEAR_BIT2()			     CLEARBIT( BIT2_WRITE, BIT2 )
#define CLEAR_BIT3()			     CLEARBIT( BIT3_WRITE, BIT3 )

#define CLEAR_BIT4()			     CLEARBIT( BIT4_WRITE, BIT4 )
#define CLEAR_BIT5()			     CLEARBIT( BIT5_WRITE, BIT5 )
#define CLEAR_BIT6()			     CLEARBIT( BIT6_WRITE, BIT6 )




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


