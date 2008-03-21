#define  F_CPU   16000000

#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>


#define UNKOWN_COMMAND	'u'
#define PORT_DIRECTION	'r'
#define PORT_SET	's'
#define PORT_GET	'g'
#define PORT_SETPIN	'p'
#define PORT_GETPIN	'i'
#define PORT_SETPINDIR	'd'


#include "uart.h"
#include "usbn2mc.h"
#include "usbn2mc/fifo.h"
#include "../../usbprog_base/firmwarelib/avrupdate.h"
#include "simpleport.h"
#include "lcd.h"

volatile unsigned char bBlink = 1;
unsigned char lcdBuf[256];
unsigned char cmdbuf[5];



void interrupt_ep_send();
void rs232_send();

volatile int tx1togl=0; 		// inital value of togl bit


char toUSBBuf[10];
int USBBuf_i=0;
char toRS232Buf[100];
char RS232_i=0;

fifo_t* toRS232FIFO;
fifo_t* toUSBFIFO;

int togl3=0;
volatile int togl1=0;


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

volatile struct usbprog_t 
{
  volatile int datatogl;
}usbprog;


/* Device Descriptor */

const unsigned char usbrs232[] =
{ 
	0x12,             // 18 length of device descriptor
    	0x01,       // descriptor type = device descriptor
    	0x10,0x01,  // version of usb spec. ( e.g. 1.1)
    	0x02,             // device class
    	0x00,             // device subclass
    	0x00,       // protocol code
    	0x08,       // deep of ep0 fifo in byte (e.g. 8)
    	0x81,0x17,  // vendor id
    	0x64,0x0c,  // product id
    	0x00,0x01,  // revision id (e.g 1.02)
    	0x00,       // index of manuf. string
    	0x00,             // index of product string
    	0x00,             // index of ser. number
    	0x01        // number of configs
};

/* Configuration descriptor */

const unsigned char usbrs232Conf[] =
{ 
	0x09,       // 9 length of this descriptor
    	0x02,       // descriptor type = configuration descriptor
    	0x48,0x00,  // total length with first interface ...
    	0x02,       // number of interfaces //bene 01
    	0x01,       // number if this config. ( arg for setconfig)
    	0x00,       // string index for config
    	0xA0,       // attrib for this configuration ( bus powerded, remote wakup support)
    	0x1A,       // power for this configuration in mA (e.g. 50mA)
		    //InterfaceDescriptor
    	0x09,       // 9 length of this descriptor
    	0x04,       // descriptor type = interface descriptor
    	0x00,       // interface number
    	0x00,       // alternate setting for this interface
    	0x01,       // number endpoints without 0
    	0x02,       // class code
    	0x02,       // sub-class code
    	0x01,       // protocoll code
    	0x00,       // string index for interface

    /* CDC Class-Specific descriptor */
    5,           /* sizeof(usbDescrCDC_HeaderFn): length of descriptor in bytes */
    0x24,        /* descriptor type */
    0,           /* header functional descriptor */
    0x10, 0x01,

    4,           /* sizeof(usbDescrCDC_AcmFn): length of descriptor in bytes */
    0x24,        /* descriptor type */
    2,           /* abstract control management functional descriptor */
    0x02,        /* SET_LINE_CODING, GET_LINE_CODING, SET_CONTROL_LINE_STATE    */

    5,           /* sizeof(usbDescrCDC_UnionFn): length of descriptor in bytes */
    0x24,        /* descriptor type */
    6,           /* union functional descriptor */
    0,           /* CDC_COMM_INTF_ID */
    1,           /* CDC_DATA_INTF_ID */

    5,           /* sizeof(usbDescrCDC_CallMgtFn): length of descriptor in bytes */
    0x24,        /* descriptor type */
    1,           /* call management functional descriptor */
    3,           /* allow management on data interface, handles call management by itself */
    1,           /* CDC_DATA_INTF_ID */

    /* Endpoint Descriptor */
    7,           /* sizeof(usbDescrEndpoint) */
    5,  /* descriptor type = endpoint */
    0x83,        /* IN endpoint number 3 */
    0x03,        /* attrib: Interrupt endpoint */
    8, 0,        /* maximum packet size */
    100,         /* in ms */

    /* Interface Descriptor  */
    9,           /* sizeof(usbDescrInterface): length of descriptor in bytes */
    4,           /* descriptor type */
    1,           /* index of this interface */
    0,           /* alternate setting for this interface */
    2,           /* endpoints excl 0: number of endpoint descriptors to follow */
    0x0A,        /* Data Interface Class Codes */
    0,
    0,           /* Data Interface Class Protocol Codes */
    0,           /* string index for interface */

    /* Endpoint Descriptor */
    7,           /* sizeof(usbDescrEndpoint) */
    5,  /* descriptor type = endpoint */
    0x01,        /* OUT endpoint number 1 */
    0x02,        /* attrib: Bulk endpoint */
#if UART_CFG_HAVE_USART
    6, 0,        /* maximum packet size 8->6 */
#else
    1, 0,        /* maximum packet size */
#endif
    0,           /* in ms */

    /* Endpoint Descriptor */
    7,           /* sizeof(usbDescrEndpoint) */
    5,  /* descriptor type = endpoint */
    0x81,        /* IN endpoint number 1 */
    0x02,        /* attrib: Bulk endpoint */
    9, 0,        /* maximum packet size */
    0,           /* in ms */

};



/* uart interrupt (only for debugging) */

//SIGNAL(SIG_UART_RECV)
void sendUSB(char c)
{
	TOGGLE_IO11();
	//UARTWrite("tipp");
	//fifo_put (toUSBFIFO,UARTGetChar());
	USBNWrite(TXC2,FLUSH);
	USBNWrite(TXD2,c);
	rs232_send();	
//		USBNWrite(TXC1,FLUSH);
//		USBNWrite(TXD1,0x44);
}

void sendUSBString(char *s)
{
	TOGGLE_IO11();
	USBNWrite(TXC2,FLUSH);
	while(*s != '\0')
		USBNWrite(TXD2,*s++);
	rs232_send();
}

/* interrupt signael from usb controller */

SIGNAL(SIG_INTERRUPT0)
{
	USBNInterrupt();
}


/*************** usb class HID requests  **************/

// reponse for requests on interface
void USBNInterfaceRequests(DeviceRequest *req,EPInfo* ep)
{
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
void USBNDecodeClassRequest(DeviceRequest *req,EPInfo* ep)
{
	UARTWrite("class");
	static unsigned char serialNotification[10] = {0xa1,0x20,0,0,0,0,2,0,3,0};
	int loop;
	switch(req->bRequest)
	{
		case 0x20:	//SET_LINE_CODING:
			UARTWrite("set line\r\n");
			USBNWrite(RXC0,RX_EN);
			
			USBNRead(RXD0);
			USBNRead(RXD0);
			USBNRead(RXD0);
			USBNRead(RXD0);
			USBNRead(RXD0);
			USBNRead(RXD0);
			USBNRead(RXD0);
			
			//USBNWrite(RXC0,RX_EN);
			//USBNWrite(RXC0,FLUSH);
		
			USBNWrite(TXC0,FLUSH);
			USBNWrite(TXC0,TX_TOGL+TX_EN);
		break;
		case 0x21:	// GET_LINE_CODING:
			//UARTWrite("get line coding");
			USBNWrite(TXC0,FLUSH);

			// baud rate
			USBNWrite(TXD0,0x80);
			USBNWrite(TXD0,0x25);
			USBNWrite(TXD0,0);
			USBNWrite(TXD0,0);

			USBNWrite(TXD0,0); //stopbit
			USBNWrite(TXD0,0); // parity
			USBNWrite(TXD0,8); // databits

			interrupt_ep_send();

			USBNWrite(TXC0,TX_TOGL+TX_EN);
			
		break;
		case 0x22:	//SET_CONTROL_LINE_STATE:
			//UARTWrite("set ctrl line state");
			USBNWrite(TXC1,FLUSH);
			// fill endpoint fifo
			for(loop=0;loop<8;loop++)
				USBNWrite(TXD1,serialNotification[loop]);

			//send and control togl bit
			interrupt_ep_send();
			USBNWrite(TXC0,TX_TOGL+TX_EN);
		break;
	}
}



unsigned char bCmd = 0;
unsigned char lcdIndex = 0;
unsigned char lcdCmdIndex = 0;


// usb zu rs232
void USBtoRS232(char * buf)
{
   unsigned char c;

   c = buf[0];
   if(bCmd != 0)
   {
      cmdbuf[lcdCmdIndex] = c;
      lcdCmdIndex++;
      bCmd--;
      if(bCmd == 0)
      {
         lcdIndex = cmdbuf[1] * LCD_COLS + cmdbuf[2];
         lcdCmdIndex = 0;
      }
   }
   else
   {
         if(c == 0x1B)
         {
            cmdbuf[lcdCmdIndex] = c;
            lcdCmdIndex++;
            bCmd = 2;
         }
         else
         {
            lcdBuf[lcdIndex] = c;
            lcdIndex++;
         }
   }

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
    unsigned char c, i, j, index;
    unsigned int blinkcnt = 0;

    usbprog.datatogl = 0;   // 1MHz

	// init fifos
//	fifo_init (toRS232FIFO, toRS232Buf, 100);
//	fifo_init (toUSBFIFO, toUSBBuf, 100);
	
	USBNCallbackFIFORX1(&USBtoRS232);
	//USBNCallbackFIFOTX2Ready(&USBtoRS232);

	sei();			// activate global interrupts
//	UARTInit();		// only for debugging

	// setup usbstack with your descriptors
	USBNInit(usbrs232,usbrs232Conf);


	USBNInitMC();		// start usb controller
	USBNStart();		// start device stack


//	sendUSBString("Hello World!");

      PORTA |= (1 << 4);
      DDRA |= (1 << 4);

      LCD_init();
      LCD_cursor(0,0);
      LCD_text("USBprog with LCD");
      sprintf(lcdBuf, "USBprog with LCD");
      LCD_specialchars();


	while(1){
	  #if 0
		// wenn cpu zeit vorhanden fifos weiterverteilen
		// usb -> rs232
	
		// rs232 -> usb
		if( USBBuf_i>2){
		USBNWrite(TXC2,FLUSH);
		USBNWrite(TXD2,toUSBBuf[0]);
		USBNWrite(TXD2,toUSBBuf[1]);
		//USBNWrite(TXD2,0x46);
		//send_toggle();	
		rs232_send();
		USBBuf_i=0;
		}
		_delay_ms(1);
	  #endif


index = 0;
for(i = 0; i < LCD_ROWS; i++)
{
   LCD_cursor(i,0);
   for(j = 0; j < LCD_COLS; j++)
   {
      LCD_char(lcdBuf[index]);
      index++;
   }
}


        if(bBlink)
        {
           blinkcnt++;
           if(blinkcnt == 20000)
           {
              blinkcnt = 0;
              PORTA ^= (1 << 4);
           }
        }
    }
}






