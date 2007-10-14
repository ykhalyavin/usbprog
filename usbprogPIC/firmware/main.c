#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "uart.h"
#include "usbn2mc.h"

void usbHIDWrite(char hex);

volatile int tx1togl=0; 		// inital value of togl bit
/* report descriptor keyboard */

char ReportDescriptorKeyboard[] = 
{ 
	5, 1, // Usage_Page (Generic Desktop) 
	9, 6, // Usage (Keyboard) 
	0xA1, 1, // Collection (Application) 
	5, 7, // Usage page (Key Codes) 
	0x19, 224, // Usage_Minimum (224) 
	0x29, 231, // Usage_Maximum (231) 
	0x15, 0, // Logical_Minimum (0) 
	0x25, 1, // Logical_Maximum (1) 
	0x75, 1, // Report_Size (1) 
	0x95, 8, // Report_Count (8) 
	0x81, 2, // Input (Data,Var,Abs) = Modifier Byte 
	0x81, 1, // Input (Constant) = Reserved Byte 
	0x19, 0, // Usage_Minimum (0) 
	0x29, 101, // Usage_Maximum (101) 
	0x15, 0, // Logical_Minimum (0) 
	0x25, 101, // Logical_Maximum (101) 
	0x75, 8, // Report_Size (8) 
	0x95, 6, // Report_Count (6) 
	0x81, 0, // Input (Data,Array) = Keycode Bytes(6) 
	5, 8, // Usage Page (LEDs) 
	0x19, 1, // Usage_Minimum (1) 
	0x29, 5, // Usage_Maximum (5) 
	0x15, 0, // Logical_Minimum (0) 
	0x25, 1, // Logical_Maximum (1) 
	0x75, 1, // Report_Size (1) 
	0x95, 5, // Report_Count (5) 
	0x91, 2, // Output (Data,Var,Abs) = LEDs (5 bits) 
	0x95, 3, // Report_Count (3) 
	0x91, 1, // Output (Constant) = Pad (3 bits) 
	0xC0 // End_Collection 
};


/* Device Descriptor */

const unsigned char usbprogPICDevice[] =
{ 
	0x12,             // 18 length of device descriptor
    	0x01,       // descriptor type = device descriptor
    	0x10,0x01,  // version of usb spec. ( e.g. 1.1)
    	0x00,             // device class
    	0x00,             // device subclass
    	0x00,       // protocol code
    	0x08,       // deep of ep0 fifo in byte (e.g. 8)
    	0xd8,0x04,  // vendor id
    	0x33,0x00,  // product id
    	0x01,0x00,  // revision id (e.g 1.02)
    	0x01,       // index of manuf. string
    	0x02,             // index of product string
    	0x00,             // index of ser. number
    	0x01        // number of configs
};

/* Configuration descriptor */

const unsigned char usbprogPIC[] =
{ 
	0x09,             // 9 length of this descriptor
    	0x02,       // descriptor type = configuration descriptor
    	0x22,0x00,  // total length with first interface ...
    	0x01,             // number of interfaces
    	0x01,             // number if this config. ( arg for setconfig)
    	0x00,       // string index for config
    	0xA0,       // attrib for this configuration ( bus powerded, remote wakup support)
    	0x1A,        // power for this configuration in mA (e.g. 50mA)
    	//InterfaceDescriptor
    	0x09,             // 9 length of this descriptor
    	0x04,       // descriptor type = interface descriptor
    	0x00,             // interface number
    	0x00,             // alternate setting for this interface
    	0x01,             // number endpoints without 0
    	0x03,       	// class code
    	0x01,       // sub-class code
    	0x01,       // protocoll code
    	0x00,       // string index for interface
    	// HID Descriptor Keyboard
    	0x09,	// length ot this descriptor
    	0x21,	// HID Descriptortype
    	0x10,0x01,	// hid class spec
    	0x00,	//country
    	0x01,	// number of hid descriptors to flollow
    	0x22,	// descriptor type
    	0x3b,	// total length of report descriptor
    	0x00,
    	//EP1 Descriptor
    	0x07,             // length of ep descriptor
    	0x05,             // descriptor type= endpoint
    	0x81,             // endpoint address (e.g. in ep1)
    	0x03,             // transfer art ( bulk )
    	0x08,0x00,  // fifo size
    	0x0A,             // polling intervall in ms
};



/* uart interrupt (only for debugging) */

SIGNAL(SIG_UART_RECV)
{
	UARTGetChar();
	
	/*char test[]="Hallo";
	int size = 4;
	usbHIDWrite(test,size,0x05);
	*/
	
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
  	// 81 06 22 get report descriptor
  	switch(req->bRequest)
  	{
    		case GET_DESCRIPTOR:
        		ep->Index=0;
			ep->DataPid=1;
        		ep->Size=59;
			ep->Buf=ReportDescriptorKeyboard;
    		break;
    		default:
      		UARTWrite("unkown interface request");
   	}
}



// vendor requests
void USBNDecodeVendorRequest(DeviceRequest *req)
{
}


// class requests
void USBNDecodeClassRequest(DeviceRequest *req)
{
}




/* function for sending strings over usb hid device 
 * please use max size of 64 in this version
 */

void usbHIDWrite(char hex)
{
  	//int i;

  	USBNWrite(TXC1,FLUSH);  //enable the TX (DATA1)

  	USBNWrite(TXD1,0x00);	// send chars 		bei shift = 02
  	USBNWrite(TXD1,0x00);	
	USBNWrite(TXD1,hex);	
  	USBNWrite(TXD1,0x00);	

  	USBNWrite(TXD1,0x00);	
  	USBNWrite(TXD1,0x00);	
  	USBNWrite(TXD1,0x00);	
  	USBNWrite(TXD1,0x00);	

  	/* control togl bit of EP1 */
  	if(tx1togl)
  	{
  		USBNWrite(TXC1,TX_TOGL+TX_EN+TX_LAST);  //enable the TX (DATA1)
		tx1togl=0;
  	}
  	else
  	{
  		USBNWrite(TXC1,TX_EN+TX_LAST);  //enable the TX (DATA1)
		tx1togl=1;
  	}
}


/*************** main function  **************/

int main(void)
{

  	sei();			// activate global interrupts
  	UARTInit();		// only for debugging

  	// setup usbstack with your descriptors
  	USBNInit(usbprogPICDevice,usbprogPIC);
	_USBNAddStringDescriptor(""); // pseudo langid
	_USBNAddStringDescriptor("USBprog EmbeddedProjects");
	_USBNAddStringDescriptor("usbprogPIC v.0.1");
	_USBNCreateStringField();



  	USBNInitMC();		// start usb controller
  	USBNStart();		// start device stack

	
while(1);
  #if 0
	int j;		 
	char key;
  while(1)
	{
		
		key = atkeyb_getchar();
		usbHIDWrite(key-93);
		//usbHIDWrite(key-93);
		//SendHex(key);

		//test[0]=key;
		//test[1]=0x00;
		//UARTWrite(test);
		



	    for(j=0;j<0xFFFF;j++){}
		usbHIDWrite(0x00);
	}
  #endif
}






