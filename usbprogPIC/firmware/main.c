#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#define  F_CPU   16000000

#include "uart.h"
#include "usbn2mc.h"
#include "usbn2mc/fifo.h"
#include "../../usbprog_base/firmwarelib/avrupdate.h"

void CommandAnswer(int length);
volatile char answer[64];

volatile struct usbprog_t {
  int datatogl;
} usbprog;



void interrupt_ep_send();
void rs232_send();

volatile int tx1togl=0; 		// inital value of togl bit


char toUSBBuf[100];
char toRS232Buf[100];

fifo_t* toRS232FIFO;
fifo_t* toUSBFIFO;

int togl3=0;
int togl1=0;


struct {
	char		dwDTERrate[4];   //data terminal rate, in bits per second
	char    bCharFormat;  //num of stop bits (0=1, 1=1.5, 2=2)
	char    bParityType;  //parity (0=none, 1=odd, 2=even, 3=mark, 4=space)
	char    bDataBits;    //data bits (5,6,7,8 or 16)
} usb_cdc_line_coding;

enum {
	SEND_ENCAPSULATED_COMMAND = 0,
	GET_ENCAPSULATED_RESPONSE,
	SET_COMM_FEATURE,
	GET_COMM_FEATURE,
	CLEAR_COMM_FEATURE,
	SET_LINE_CODING = 0x20,
	GET_LINE_CODING,
	SET_CONTROL_LINE_STATE,
	SEND_BREAK
};


/* Device Descriptor */

const unsigned char usbrs232[] =
{ 
	0x12,             // 18 length of device descriptor
    	0x01,       // descriptor type = device descriptor
    	0x10,0x01,  // version of usb spec. ( e.g. 1.1)
    	0x00,             // device class
    	0x00,             // device subclass
    	0x00,       // protocol code
    	0x08,       // deep of ep0 fifo in byte (e.g. 8)
    	0xD8,0x04,  // vendor id
    	0x33,0x00,  // product id
    	0x01,0x00,  // revision id (e.g 1.02)
    	0x01,       // index of manuf. string
    	0x02,       // index of product string
    	0x03,       // index of ser. number
    	0x02        // number of configs
};

/* Configuration descriptor */

const unsigned char usbrs232Conf[] =
{ 
	0x09,       // 9 length of this descriptor
    	0x02,       // descriptor type = configuration descriptor
    	0x29,0x00,  // total length with first interface ...
    	0x01,       // number of interfaces //bene 01
    	0x01,       // number if this config. ( arg for setconfig)
    	0x00,       // string index for config
    	0xA0,       // attrib for this configuration ( bus powerded, remote wakup support)
    	0x32,       // power for this configuration in mA (e.g. 50mA)
		    //InterfaceDescriptor
    	0x09,       // 9 length of this descriptor
    	0x04,       // descriptor type = interface descriptor
    	0x00,       // interface number
    	0x00,       // alternate setting for this interface
    	0x02,       // number endpoints without 0
    	0x03,       // class code
    	0x00,       // sub-class code
    	0x00,       // protocoll code
    	0x00,       // string index for interface
	0x09,
	0x21,
	0x01,
	0x00,
	0x00,
	0x01,
	0x22,0x1D,0x00,0x07,0x05,0x81,0x03,0x40,0x00,0x01,0x07,0x05,0x01,0x03,0x40,0x00,0x01
};

char ReportDescriptorKeyboard[] =
{
  0x06,0x00,0xff,0x09,0x01,0xa1,0x01,0x19,
  0x01,0x29,0x40,0x15,0x00,0x26,0xff,0x00,
  0x75,0x08,0x95,0x40,0x81,0x02,0x19,0x01,
  0x29,0x40,0x91,0x02,0xc0
};


/* uart interrupt (only for debugging) */

SIGNAL(SIG_UART_RECV)
{
	//UARTWrite("tipp");
	//fifo_put (toUSBFIFO,UARTGetChar());
	USBNWrite(TXC2,FLUSH);
	USBNWrite(TXD2,UARTGetChar());
	rs232_send();	
}

/* interrupt signael from usb controller */

SIGNAL(SIG_INTERRUPT0)
{
	USBNInterrupt();
}

void CommandAnswer(int length)
{
  int i;

  USBNWrite(TXC1, FLUSH);
  for(i = 0; i < length; i++)
    USBNWrite(TXD1, answer[i]);

  /* control togl bit */
  USBToglAndSend();
}


void USBToglAndSend(void)
{
  if(usbprog.datatogl == 1) {
    USBNWrite(TXC1, TX_LAST+TX_EN+TX_TOGL);
    usbprog.datatogl = 0;
  } else {
    USBNWrite(TXC1, TX_LAST+TX_EN);
    usbprog.datatogl = 1;
  }
}


/*************** usb class HID requests  **************/

// reponse for requests on interface
void USBNDecodeClassRequest(DeviceRequest *req,EPInfo* ep)
{
  //UARTWrite("Class\r\n");
  // uncompleted 
  USBNWrite(TXC0,FLUSH);
  USBNWrite(TXD0,0x29);
  USBNWrite(TXD0,0x40);
  USBNWrite(TXD0,0x91);
  USBNWrite(TXD0,0x02);
  USBNWrite(TXD0,0xC0);
  USBNWrite(TXC0,TX_TOGL+TX_EN);
  
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

// class requests
void USBNInterfaceRequests(DeviceRequest *req,EPInfo* ep)
{
        // 81 06 22 get report descriptor
	switch(req->bRequest)
	{
	  case GET_DESCRIPTOR:
	    ep->Index=0;
	    ep->DataPid=1;
	    ep->Size=29;
	    ep->Buf=ReportDescriptorKeyboard;
	  break;
	  default:
		;
	}
}


// used
void FromPC(char * buf)
{
      usbprog.datatogl=0;

      SendHex(buf[0]);
      SendHex(buf[1]);
      SendHex(buf[2]);


      int i;
      for(i=0;i<64;i++)
	answer[i]=0;
      answer[0]=0x02;
      answer[1]=0x0A;
      CommandAnswer(64);
}


// togl pid for in endpoint
void interrupt_ep_send()
{
	if(togl3==1) {
		togl3=0;
		USBNWrite(TXC1,TX_LAST+TX_EN);
	} else {
		togl3=1;
		USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
	}
}

// togl pid for in endpoint
void rs232_send()
{
	if(togl1==1) {
		togl1=0;
		USBNWrite(TXC2,TX_LAST+TX_EN);
	} else {
		togl1=1;
		USBNWrite(TXC2,TX_LAST+TX_EN+TX_TOGL);
	}
}


/*************** main function  **************/

int main(void)
{
	// init fifos
	fifo_init (toRS232FIFO, toRS232Buf, 100);
	fifo_init (toUSBFIFO, toUSBBuf, 100);
	
	USBNCallbackFIFORX1(&FromPC);
	//USBNCallbackFIFOTX2Ready(&USBtoRS232);

	sei();			// activate global interrupts
	UARTInit();		// only for debugging

	// setup usbstack with your descriptors
	USBNInit(usbrs232,usbrs232Conf);

	_USBNAddStringDescriptor(""); //pseudo lang
	_USBNAddStringDescriptor("Microchip Technology Inc.");
	//_USBNAddStringDescriptor("abcdefgh");
	_USBNAddStringDescriptor("PICkit 2 Microcontroller Programmer     ");
	//_USBNAddStringDescriptor("PICkit 2 Microcontroller Programmer");
	//_USBNAddStringDescriptor("ijklmnop");
	_USBNAddStringDescriptor("PIC18F2550");
	//_USBNAddStringDescriptor("12345678");
	_USBNCreateStringField();


	USBNInitMC();		// start usb controller
	USBNStart();		// start device stack

	while(1);
}






