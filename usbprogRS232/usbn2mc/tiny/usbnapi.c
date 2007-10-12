/* usbn960x.c
* Copyright (C) 2006  Benedikt Sauter
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
#include <stdio.h>
#include <string.h>
#include "usbnapi.h"


// setup global datastructure
void USBNInit(unsigned char* _DeviceDescriptor,unsigned char* _ConfigurationDescriptor)
{
  DeviceDescriptor=_DeviceDescriptor;
  ConfigurationDescriptor=_ConfigurationDescriptor;
}

void USBNCallbackFIFORX1(void *fct)
{
  RX1Callback = fct;
}


void USBNStart(void)
{

  _USBNInitEP0();
                                    // usbn starts here

  USBNWrite(MCNTRL,SRST);           // clear all registers
  while(USBNRead(MCNTRL)&SRST);

  //USBNWrite(CCONF, 0x80);           // clock output off, divisor to 4 MHz
  USBNWrite(CCONF, 0x02);           // clock to 16 MHz

  //USBNWrite(NAKMSK,0xFF);
  USBNWrite(NAKMSK,NAK_OUT0);
  USBNWrite(FAR,AD_EN+0x00);            // set default address
  USBNWrite(EPC0,DEF);
  USBNWrite(TXC0,FLUSH);            // FLUSHTX0;

  USBNWrite(RXC0,RX_EN+FLUSH);            // enable EP0 receive
  //USBNWrite(RXC1,RX_EN);            // enable EP0 receive
 
  USBNWrite(RXMSK, RX_FIFO0+RX_FIFO1+RX_FIFO2+RX_FIFO3);            // data incoming EP0
  USBNWrite(TXMSK, TX_FIFO0+TX_FIFO1+TX_FIFO2+TX_FIFO3);            // data incoming EP0
 
  USBNWrite(ALTMSK, ALT_RESET+ALT_SD3);
  USBNWrite(MAMSK, (INTR_E+RX_EV+ALT+TX_EV+NAK) );
 
  
  USBNWrite(NFSR,OPR_ST);
  USBNWrite(MCNTRL, VGE+NAT+INT_L_P);     // VGE, no NAT, interrupt on high
 
  return ;
}


// ********************************************************************
// Interrupt Routine for USBN960x
// ********************************************************************


void USBNInterrupt(void)
{
  unsigned char maev,mask;

 
  maev = USBNRead(MAEV);

  if(maev & RX_EV)  _USBNReceiveEvent();
  else if(maev & TX_EV) _USBNTransmitEvent();
  else if(maev & ALT)   _USBNAlternateEvent();
  else if(maev & NAK)   _USBNNackEvent();

  mask = USBNRead(MAMSK);
  USBNWrite(MAMSK,0x00);                  // disable irq
  USBNWrite(MAMSK,mask);

}






