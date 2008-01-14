#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "uart.h"
#include "usbn2mc.h"
#include "../../usbprog_base/firmwarelib/avrupdate.h"


#include "jtagcmd.h"
#include "usbprog.h"


SIGNAL(SIG_INTERRUPT0)
{
  USBNInterrupt();
}

void USBNDecodeVendorRequest(DeviceRequest *req)
{
  switch(req->bRequest) {
    case STARTAVRUPDATE:
      avrupdate_start();
    break;
    case SET_SPEED:

    break;
    case GET_SPEED:

    break;
    case USER_INTERFACE:
      vendorrequest[0] = 0;
      if(req->wValue==LED_ON)
	LED_on;
      else if (req->wValue==LED_OFF)
	LED_off;
      else if (req->wValue==GET_JUMPER)
	if ( !(PINA & (1<<PINA7)) ) 
	  vendorrequest[0] = 1;
      else
	;
      VendorRequestAnswer(1);
    break;
    case GET_VERSION:
      vendorrequest[0] = 0x11;
      vendorrequest[1] = 0x22;
      vendorrequest[2] = 0x33;
      vendorrequest[3] = 0x44;
      VendorRequestAnswer(4);
    break;
    default:
      ;
  }
}

void VendorRequestAnswer(int size){
  int i;
  for(i=0;i<size;i++)
    USBNWrite(TXD0,vendorrequest[i]);

  USBNWrite(TXC0,TX_TOGL+TX_EN);
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
    USBNWrite(TXD1, jtagcmd.jtagcmdbuf_tx[usbprog.long_index+i]);

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
  /* collect complete packet */

  /* start packet if first part of packet is here */
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

 
  // setup your usbn device

  USBNDeviceVendorID(0x1786);
  USBNDeviceProductID(0x0c62);
  USBNDeviceBCDDevice(0x0007);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor
  
  USBNDeviceManufacture ("EmbeddedProjects");
  USBNDeviceProduct	("usbprogOpenOCD  ");

  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,&CommandAnswerRest);
  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&Commands);

  
  USBNInitMC();
  sei();
  USBNStart();
 
  DDRA |= (1 << PA4); // status led
  PORTA |= (1<<PA7); //switch on internal pullup


  LED_on;
  
  //PORTA &= ~(1<<PA7);
  //CommandAnswer(320);
  //PORTA |= (1<<PA7);
  //PORTA &= ~(1<<PA7);

  while(1);
}

