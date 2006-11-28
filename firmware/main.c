#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#define F_CPU 16000000
#include <util/delay.h>

#include "../avrupdate/firmwarelib/avrupdate.h"
#include "uart.h"
#include "usbn2mc.h"

/* command descriptions for mk2 */
//#include "avr069.h"

#include "devices/at89.h"


/* send a command back to pc */
void CommandAnswer(char* buf, int length);
volatile int datatogl=0;


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
	UARTWrite("vendor request check ");
	SendHex(req->bRequest);
	switch(req->bRequest)
	{
		case STARTAVRUPDATE:
			avrupdate_start();
		break;
	}
}


void CommandAnswer(char* buf, int length)
{
	int i;
	USBNWrite(TXC1,FLUSH);

	for(i=0;i<length;i++)
		USBNWrite(TXD1,buf[i]);


	if(datatogl==1) {
		USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
		datatogl=0;
	} else {
		USBNWrite(TXC1,TX_LAST+TX_EN);
		datatogl=1;
	}
}


/* central command parser */
void USBFlash(char *buf)
{
	SendHex(buf[0]);
	SendHex(buf[1]);
	SendHex(buf[2]);
}

int main(void)
{
  int conf, interf;
  UARTInit();

  USBNInit();   
  
  // setup your usbn device

  USBNDeviceVendorID(0x03EB);
  USBNDeviceProductID(0x2104);
  USBNDeviceBCDDevice(0x0200);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  
  USBNDeviceManufacture ("Benedikt Sauter");
  USBNDeviceProduct	("usbprog USB Programmer");
  USBNDeviceSerialNumber("3");

  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);


  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&USBFlash);
  USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,NULL);

  
  MCUCR |=  (1 << ISC01); // fallende flanke

  GICR |= (1 << INT0);
  sei();
  
  USBNInitMC();

  // start usb chip
  USBNStart();

  while(1);
}


