#include <stdlib.h>
#include <avr/io.h>
#include <avr/boot.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#define F_CPU 4000000
#include <avr/delay.h>


#include "uart.h"
#include "usbn2mc.h"

//void Terminal(char cmd);

SIGNAL(SIG_UART_RECV)
{
  //Terminal(UARTGetChar());
  UARTGetChar();
  UARTWrite("usbn>");
  //SendHex(0x05);
}

SIGNAL(SIG_INTERRUPT0)
{
  UARTWrite("irq");
  USBNInterrupt();
}


void USBNInterfaceRequests(DeviceRequest *req,EPInfo* ep){}
void USBNDecodeVendorRequest(DeviceRequest *req){}
void USBNDecodeClassRequest(DeviceRequest *req){}


int main(void)
{
  const unsigned char usbprogDevice[] =
  { 0x12,	      // 18 length of device descriptor
    0x01,       // descriptor type = device descriptor 
    0x10,0x01,  // version of usb spec. ( e.g. 1.1) 
    0x00,	      // device class
    0x00,	      // device subclass
    0x00,       // protocol code
    0x08,       // deep of ep0 fifo in byte (e.g. 8)
    0x00,0x04,  // vendor id
    0x12,0x34,  // product id
    0x03,0x01,  // revision id (e.g 1.02)
    0x00,       // index of manuf. string
    0x00,	      // index of product string
    0x00,	      // index of ser. number
    0x01        // number of configs
  };

  // ********************************************************************
  // configuration descriptor          
  // ********************************************************************

  const unsigned char usbprogConf[] =
  { 0x09,	      // 9 length of this descriptor
    0x02,       // descriptor type = configuration descriptor 
    0x20,0x00,  // total length with first interface ... 
    0x01,	      // number of interfaces
    0x01,	      // number if this config. ( arg for setconfig)
    0x00,       // string index for config
    0xA0,       // attrib for this configuration ( bus powerded, remote wakup support)
    0x1A,        // power for this configuration in mA (e.g. 50mA)
    //InterfaceDescriptor
    0x09,	      // 9 length of this descriptor
    0x04,       // descriptor type = interface descriptor 
    0x00,	      // interface number 
    0x00,	      // alternate setting for this interface 
    0x02,	      // number endpoints without 0
    0x00,       // class code 
    0x00,       // sub-class code 
    0x00,       // protocoll code
    0x00,       // string index for interface
    //EP1 Descriptor
    0x07,	      // length of ep descriptor
    0x05,	      // descriptor type= endpoint
    0x81,	      // endpoint address (e.g. in ep1)
    0x02,	      // transfer art ( bulk )
    0x80,0x00,  // fifo size
    0x00,	      // polling intervall in ms
    //EP2 Descriptor
    0x07,	      // length of ep descriptor
    0x05,	      // descriptor type= endpoint
    0x02,	      // endpoint address (e.g. out ep2)
    0x02,	      // transfer art ( bulk )
    0x00,0x08,  // fifo size
    0x00	      // polling intervall in ms
  };
  UARTInit();

  sei(); 
  USBNInit(usbprogDevice,usbprogConf);   

  //USBNCallbackFIFORX1(&BootLoader);

  MCUCR |=  (1 << ISC01); // fallende flanke
  GICR |= (1 << INT0);

  USBNInitMC();

  // start usb chip
  USBNStart();
  
  while(1){
 		//SendHex(USBNRead(0x03)); // revision
  }

}


