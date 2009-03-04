#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "uart.h"
#include "usbn2mc.h"
#include "../../usbprog_base/firmwarelib/avrupdate.h"

#define F_CPU 16000000UL
#include <util/delay.h>

#include "ring.h"

#include "logic.h"

SIGNAL(SIG_UART_RECV)
{
}


SIGNAL(SIG_INTERRUPT0)
{
  USBNInterrupt();
}


SIGNAL(SIG_OUTPUT_COMPARE1A)
{
  // activate signal for next measure
  uint8_t port=PINB; 
  int old,new;
  if(logic.trigger!=TRIGGER_OFF)
  {
    if(logic.trigger==TRIGGER_EDGE)
    {
      old = logic.trigger_last & (1<<(logic.trigger_channel-1)) ?1:0;
      new = port & (1<<(logic.trigger_channel-1)) ?1:0;
      
      if(logic.trigger_value==1)
      {
	if(new==1 && old==0)
	  logic.trigger=TRIGGER_OFF;

      }else
      {
	if(new==0 && old==1)
	  logic.trigger=TRIGGER_OFF;
      }
	
      logic.trigger_last=port;
    }
    if(logic.trigger==TRIGGER_PATTERN)
    {
      // ignore pattern
      if( (port | logic.trigger_ignore)  == (logic.trigger_value|logic.trigger_ignore))
      {
	logic.trigger=TRIGGER_OFF;
      }
    }

  }
  else
  {
    ring_put (&logic.ring, port);
  }

  if(togl==1)
  {
    PORTA = 0xFF;
    togl=0;
  }
  else {
    PORTA = 0x00;
    togl=1;
  }
}

void USBNDecodeVendorRequest(DeviceRequest *req)
{
  switch(req->bRequest)
  {
    case STARTAVRUPDATE:
      avrupdate_start();
    break;
  }
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
  
  USBNDeviceManufacture ("EmbeddedProjects");
  USBNDeviceProduct	("usbprogLogic Analyzer 5V");

  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  
  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&LogicCommand); // scope commands
  USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,&LogicPingPongTX1); // scope data

  
  USBNInitMC();
  sei();

  // init ring
  ring_init(&logic.ring, ringbuffer, BUFFER_SIZE);   


  //setup logic state and mode
  logic.state=STATE_DONOTHING;
  logic.mode=MODE_NONE;
  logic.samplerate=SAMPLERATE_1MS;
  logic.trigger=TRIGGER_OFF;
  logic.trigger_last=0xff;

  // start usb chip
  USBNStart();

  //DDRB=0xff;
  DDRA=0xff;
  DDRB=0x00; //in port
  PORTB = 0xff; //internal pull up resistors

  int datatogl=0;

  int ringstate=1;
  int internstate=1;
  int i,j;

  UARTWrite("ready...\r\n");

  while(1){

    if(logic.ring.count>=BUFFER_SIZE)
    {
	if(logic.mode==MODE_LOGICINTERN)
	{
	  logic.state=STATE_DONOTHING;
          TCCR1B = (1 << WGM12) | (0 << CS12) | (0 << CS11) | (0 << CS10);
	    if(internstate)
	    {
	      UARTWrite("intern stoped");
	      internstate=0;
	    }
	  logic.mode=MODE_LOGIC;
	}
    }

/*
  if(logic.ring.count>0 &&logic.tx==1 && logic.mode==MODE_LOGIC)
  {
    logic.tx=0;
    USBNWrite(TXC1,FLUSH);

    USBNWrite(TXD1,ring_get_nowait(&logic.ring));

    if(logic.ring.count<63)
      j=logic.ring.count;
    else j=63;
    for(i=0;i<j;i++)
      USBNBurstWrite(ring_get_nowait(&logic.ring));

    if(datatogl==1)
    {
      USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
      datatogl=0;
    }else
    {
      USBNWrite(TXC1,TX_LAST+TX_EN);
      datatogl=1;
    }
  }
*/
  }
}

