#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "uart.h"
#include "usbn2mc.h"
#include "../usbprog_base/firmwarelib/avrupdate.h"

#define F_CPU 160000000UL
#include <util/delay.h>


#define LED_PIN     PA4
#define LED_PORT    PORTA

#define LED_on     (LED_PORT   |=  (1 << LED_PIN))   // red led
#define LED_off    (LED_PORT   &= ~(1 << LED_PIN))

volatile char answer[320];

volatile struct usbprog_t
{
  int long_index;
  int long_bytes;
  int long_running;
  int datatogl;
} usbprog;

SIGNAL(SIG_INTERRUPT0)
{
  USBNInterrupt();
}

void USBNDecodeVendorRequest(DeviceRequest *req)
{
  if(req->bRequest == STARTAVRUPDATE)
      avrupdate_start();
}

void CommandAnswer(length)
{
  int i;

  // if first packet of a lang message
  if(length>64 && usbprog.long_running==0){
    usbprog.long_index=0;
    usbprog.long_bytes=length;
    usbprog.long_running=1;
    length=64;
  }

  USBNWrite(TXC1, FLUSH);

  for(i = 0; i < length; i++)
    USBNWrite(TXD1, answer[usbprog.long_index+i]);

  /* control togl bit */
  if(usbprog.datatogl == 1) {
    USBNWrite(TXC1, TX_LAST+TX_EN+TX_TOGL);
    usbprog.datatogl = 0;
  } else {
    USBNWrite(TXC1, TX_LAST+TX_EN);
    usbprog.datatogl = 1;
  }
}


void CommandAnswerRest()
{

  if(usbprog.long_running==1){
    PORTA ^= (1<<PA7);
    if(usbprog.long_index < usbprog.long_bytes){
      int dif = usbprog.long_bytes-usbprog.long_index; 
      usbprog.long_index=usbprog.long_index+64;

      if(dif > 64){
	CommandAnswer(64);
      }
      else {
	// last packet
	CommandAnswer(dif);
	usbprog.long_running=0;
      }
    }
  }
}


void Commands(char * buf)
{
  PORTA ^= (1<<PA7);
  if(buf[0]==0x77 && buf[1]==0x88)
    CommandAnswer(320);
}


int main(void)
{
  int conf, interf;
  
  //UARTInit();
  
  USBNInit();   
  usbprog.long_running=0;

  DDRA = (1 << PA4); // status led
  DDRA = (1 << PA7); // switch pin


  // setup your usbn device

  USBNDeviceVendorID(0x1786);
  USBNDeviceProductID(0x0c62);
  USBNDeviceBCDDevice(0x0007);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor
  
  USBNDeviceManufacture ("EmbeddedProjects");
  USBNDeviceProduct	("usbprogSkeleton ");

  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,&CommandAnswerRest);
  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&Commands);

  
  USBNInitMC();
  sei();
  USBNStart();

  //LED_on;
  int i;
  for(i=0;i<320;i++)
    answer[i]=i+2;
  
  PORTA &= ~(1<<PA7);
  //CommandAnswer(320);
  DDRB = 0xFF;

  while(1){
    
    for(i=0;i < 255; i++)
    {
      _delay_us(1);
      PORTB = (unsigned char)i;
    }
    //PORTA |= (1<<PA7);
    //PORTA &= ~(1<<PA7);
  }
}

