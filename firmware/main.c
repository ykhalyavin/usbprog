#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#define F_CPU 16000000
#include <util/delay.h>

#include "../avrupdate/firmwarelib/avrupdate.h"
#include "uart.h"
#include "usbn2mc.h"

/* command descriptions for mk2 */
#include "avr069.h"

#include "devices/at89.h"

/*** prototypes and global vars ***/
/* send a command back to pc */
void CommandAnswer(int length);
volatile int datatogl=0;
volatile int longpackage=0;

char answer[64];


SIGNAL(SIG_UART_RECV)
{
  //Terminal(UARTGetChar());
  //UARTWrite("usbn>");
}


SIGNAL(SIG_INTERRUPT0)
{
  cli();
  USBNInterrupt();
  sei();
}

/* id need for live update of firmware */

void USBNDecodeVendorRequest(DeviceRequest *req)
{
	UARTWrite("vendor request check ");
	SendHex(req->bRequest);
	switch(req->bRequest)
	{
		case STARTAVRUPDATE:
			avrupdate_start();
		break;
	}
}


void CommandAnswer(int length)
{
	int i;
	USBNWrite(TXC1,FLUSH);

	for(i=0;i<length;i++)
		USBNWrite(TXD1,answer[i]);

	/* control togl bit */
	if(datatogl==1) {
		USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
		datatogl=0;
	} else {
		USBNWrite(TXC1,TX_LAST+TX_EN);
		datatogl=1;
	}
}


/* central command parser */
void USBFlash(char *buf)
{
	if(longpackage) {

	}
	else {
		
		switch(buf[0]) {
		
		case CMD_SIGN_ON:
			UARTWrite("sign\n");
			answer[0] = CMD_SIGN_ON;
			answer[1] = STATUS_CMD_OK;
			answer[2] = 10; // length
			answer[3] = 'A';
			answer[4] = 'V';
			answer[5] = 'R';
			answer[6] = 'I';
			answer[7] = 'S';
			answer[8] = 'P';
			answer[9] = '_';
			answer[10] = 'M';
			answer[11] = 'K';
			answer[12] = '2';
			CommandAnswer(13);
		break;
		case CMD_SET_PARAMETER:

		break;
		case CMD_GET_PARAMETER:

		break;
		case CMD_OSCCAL:

		break;
		case CMD_LOAD_ADDRESS:

		break;
		case CMD_FIRMWARE_UPGRADE:

		break;
		case CMD_RESET_PROTECTION:

		break;
		case CMD_ENTER_PROGMODE_ISP:

		break;
		case CMD_LEAVE_PROGMODE_ISP:

		break;
		case CMD_CHIP_ERASE_ISP:

		break;
		case CMD_PROGRAM_FLASH_ISP:

		break;
		case CMD_READ_FLASH_ISP:

		break;
		case CMD_PROGRAM_EEPROM_ISP:

		break;
		case CMD_READ_EEPROM_ISP:

		break;
		case CMD_PROGRAM_FUSE_ISP:

		break;
		case CMD_READ_FUSE_ISP:

		break;
		}
	}
}

int main(void)
{
  int conf, interf;
  UARTInit();

  USBNInit();   
  
  // setup your usbn device

  USBNDeviceVendorID(0x03EB);
  USBNDeviceProductID(0x2104);
  USBNDeviceBCDDevice(0x0200);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  
  USBNDeviceManufacture ("Benedikt Sauter");
  USBNDeviceProduct	("usbprog USB Programmer");
  USBNDeviceSerialNumber("3");

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


