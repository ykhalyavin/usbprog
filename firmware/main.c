#include <stdlib.h>
#include <avr/io.h>
#include <avr/boot.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#define F_CPU 16000000
#include <avr/delay.h>


#include "uart.h"
#include "usbn2mc.h"

void Terminal(char cmd);

SIGNAL(SIG_UART_RECV)
{
  Terminal(UARTGetChar());
  //UARTGetChar();
  UARTWrite("usbn>");
  //SendHex(USBNRead(0x03));
}

SIGNAL(SIG_INTERRUPT0)
{
  UARTWrite("irq");
  //USBNInterrupt();
}


void USBNInterfaceRequests(DeviceRequest *req,EPInfo* ep){}
void USBNDecodeVendorRequest(DeviceRequest *req){}
void USBNDecodeClassRequest(DeviceRequest *req){}


int main(void)
{
  	int conf, interf;
  	UARTInit();
  	USBNInit();

  	USBNDeviceVendorID(0x0400);
	USBNDeviceProductID(0x9876);
	USBNDeviceBCDDevice(0x0201);

	char lang[]={0x09,0x04};
  	_USBNAddStringDescriptor(lang); // language descriptor

	USBNDeviceManufacture ("B.Sauter");
	USBNDeviceProduct	("usbprog");
	USBNDeviceSerialNumber("2006042401");

	conf = USBNAddConfiguration();
	USBNConfigurationName(conf,"StandardKonfiguration");
	USBNConfigurationPower(conf,50);

	interf = USBNAddInterface(conf,0);
	USBNAlternateSetting(conf,interf,0);

	USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,NULL);
	USBNAddInEndpoint(conf,interf,1,0x03,BULK,64,0,NULL);

	USBNInitMC();
  	// start usb chip
  	USBNStart();
  
  	while(1);
}


