#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "uart.h"
#include "usbn2mc.h"

void Terminal(char cmd);

SIGNAL(SIG_UART_RECV)
{
  Terminal(UARTGetChar());
  UARTWrite("usbn>");
}



SIGNAL(SIG_INTERRUPT0)
{
  USBNInterrupt();
}


int togl=0;

// testfunction where called when data on ep2, buf is a ptr to a 64 byte field 
void Receive(char *buf)
{
  int i;
  for(i=0;i<64;i++)
    SendHex(buf[i]);

  UARTWrite("\r\nSend Back\r\n");

  USBNWrite(TXC1,FLUSH);
  for(i=0;i<64;i++)
    USBNWrite(TXD1,i);

  USBNWrite(TXC1,TX_LAST+TX_EN);

}

// called at transfer irq
void TransferISR()
{
  //UARTWrite("ready for next\r\n");
  int i;

  USBNWrite(TXC1,FLUSH);
  
  for(i=0;i<56;i++)
    USBNWrite(TXD1,i);


  USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
}


int main(void)
{
  int conf, interf;
  UARTInit();
  USBNInit();   
  // setup your usbn device

  USBNDeviceVendorID(0x0400);
  USBNDeviceProductID(0x9876);
  USBNDeviceBCDDevice(0x0201);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  
  USBNDeviceManufacture ("Benedikt Sauter <sauter@ixbat.de>");
  USBNDeviceProduct	("usbprog USB Programmer");
  USBNDeviceSerialNumber("200611121");

  conf = USBNAddConfiguration();

  //USBNConfigurationName(conf,"StandardKonfiguration");
  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  //USBNInterfaceName(conf,interf,"usbstorage");
  

  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&Receive);
  USBNAddInEndpoint(conf,interf,1,0x03,BULK,64,0,&TransferISR);

  
  USBNInitMC();
  sei();

  // start usb chip
  USBNStart();
  UARTWrite("waiting for enumaration signal...\r\n");

  while(1);
}


