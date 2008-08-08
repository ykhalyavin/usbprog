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
#include "fifo.h"

#define DEBUG 0

#define BUF_SIZE 400
uint8_t buffer[BUF_SIZE];
fifo_t fifo;


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
  jtagcmd.tx_index=0;
  PORTA ^= (1<<PA7);

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

  PORTA ^= (1<<PA7);
  if(usbprog.long_running==1){
    //PORTA ^= (1<<PA7);
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
  int i; 
  /* collect all data */
  for(i=0;i<64;i++) {
    fifo_put (&fifo, buf[i]);
    //SendHex(buf[i]);
  }
  //UARTWrite("RX\r\n");
  //SendHex(fifo.count);

  PORTA ^= (1<<PA7);
}


int main(void)
{
  int conf, interf;
  
  UARTInit();
  
  USBNInit();   
  usbprog.long_running=0;

  fifo_init (&fifo, buffer, BUF_SIZE);       

 
  // setup your usbn device

  USBNDeviceVendorID(0x1786);
  USBNDeviceProductID(0x0c62);
  USBNDeviceBCDDevice(0x0007);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor
  
  USBNDeviceManufacture ("EmbeddedProjects");
  USBNDeviceProduct	("USBprog JTAG v.1");

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
  DDRA |= (1 << PA7); // debug
  // PORTA |= (1<<PA7); //switch on internal pullup

  LED_off;
  
  //PORTA &= ~(1<<PA7);
  //CommandAnswer(320);
  //PORTA |= (1<<PA7);
  //PORTA &= ~(1<<PA7);
  
  uint8_t cmd, cs;
  uint8_t length;
  uint8_t bit_length;

  //fifo_t *f = &fifo;

  while(1){
    
    //SendHex(fifo.count);
    cmd = fifo_get_wait (&fifo);
    //PORTA ^= (1<<PA7);

    cs = cmd >> 5;
    switch(cs){
      case 0x01: // SCAN
	
	length = fifo_get_wait (&fifo);
		
	#if DEBUG
	UARTWrite("scan ");
	#endif

	if(SCAN_BYTE) {bit_length=8;} else  { bit_length=length; length=1;}

	for(length;length>0;length--) {
	  
	  if(SCAN_READ && SCAN_WRITE) {
	    
	    #if DEBUG 
	    if(SCAN_TDI) UARTWrite("TDI rw ");
	    else UARTWrite("TMS rw ");
	    #endif

	    if(SCAN_TDI) bit_out_in( fifo_get_wait (&fifo), bit_length,NEXT_ANSWER_ADDR);
	    else bit_out_in_tms(fifo_get_wait (&fifo), bit_length, NEXT_ANSWER_ADDR, SCAN_VALUE?1:0);

	    //ANSWER_ADD = 0x66; /* also possible solutiion for results */

	  } else if (SCAN_READ) {
	    
	    #if DEBUG 
	    if(SCAN_TDI) UARTWrite("TDI r ");
	    else UARTWrite("TMS r ");
	    #endif

	    if(SCAN_TDI) bit_in( fifo_get_wait (&fifo), bit_length, NEXT_ANSWER_ADDR);
	    else bit_in_tms(fifo_get_wait (&fifo), bit_length,NEXT_ANSWER_ADDR, SCAN_VALUE?1:0);

	  } else if (SCAN_WRITE) {
  
	    #if DEBUG 
	    if(SCAN_TDI) UARTWrite("TDI w ");
	    else UARTWrite("TMS w ");
	    #endif

	    if(SCAN_TDI) bit_out( fifo_get_wait (&fifo), bit_length);
	    else bit_out_tms(fifo_get_wait (&fifo), bit_length, SCAN_VALUE?1:0);

	  } else {
	    // unkown
	  }

	}
	#if DEBUG
	UARTWrite("scan end\r\n");
	#endif
      break;
      case 0x02: // GPIO
	#if DEBUG
	UARTWrite("scan\r\n");
	#endif
      break;
      default:

	#if DEBUG
	//UARTWrite("unkown\r\n");
	#endif
	;

	/* send answer if some bytes are available */	
	if(jtagcmd.tx_index>0) {
	  //usbprog.datatogl = 0;
	  CommandAnswer(jtagcmd.tx_index+1);
	  if(usbprog.datatogl==1)  usbprog.datatogl=0; else  usbprog.datatogl=1;
	}
    }
  }
}

