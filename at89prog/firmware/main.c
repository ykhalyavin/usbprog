#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
//#include <avr/signal.h>
#include <inttypes.h>

#define F_CPU 16000000
#include <util/delay.h>

#include "uart.h"
#include "../../usbn2mc/main/usbnapi.h"
#include "../../usbprog_base/firmwarelib/avrupdate.h"
#include "usbn2mc.h"

#include "wait.h"

#include "at89.h"

//void Terminal(char cmd);

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
  switch(req->bRequest)
  {
    case STARTAVRUPDATE:
      avrupdate_start();
    break;
  }
}



void USBFlash(char *buf)
{

  if(buf[0]==0x01)
    At89FlashErase();
  else if(buf[0]==0x02)
    At89FlashWrite(buf);
  else {}
}

int main(void)
{
  int conf, interf;
  UARTInit();
  USBNInit();   
  
  // setup your usbn device

  USBNDeviceVendorID(0x1781);
  USBNDeviceProductID(0x0c64);
  USBNDeviceBCDDevice(0x0201);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  
  USBNDeviceManufacture ("B.Sauter");
  USBNDeviceProduct	("at89prog");
  USBNDeviceSerialNumber("2007-08-11");

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


