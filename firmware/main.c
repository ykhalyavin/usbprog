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

//#include "spi.h"

#include "devices/at89.h"

/*** prototypes and global vars ***/
/* send a command back to pc */
void CommandAnswer(int length);
volatile int datatogl=0;
volatile int longpackage=0;

volatile char answer[64];


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


#define DDR_SPI DDRB
#define MOSI PB5
#define MISO PB6
#define SCK PB7
#define RESET PB0


void spi_init()
{
	
	DDR_SPI &=~(1<<MISO);
	//PORTB = (1<<MISO);

	DDR_SPI = (1<<MOSI)|(1<<SCK)|(1<<RESET);
	
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|3;
    SPSR = (0<<SPI2X);
}

void spi_out(char data)
{
  	SPDR = data;
    while ( !(SPSR & (1<<SPIF)) ) ;
}


void spi_in_init()
{
	//SPCR = (1<<SPE);
}

char spi_in()
{
	SPDR = 0;
	while(!(SPSR & (1<<SPIF)));
	return SPDR;
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
	int i; 
	char result;
	if(longpackage) {

	}
	else {
		
		switch(buf[0]) {
		case CMD_SIGN_ON:
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
			//cbi	portb,SCK	; clear SCK
			PORTB &= ~(1<<SCK);
			
			// set_reset		;	set RESET = 1
			PORTB |= (1<<RESET);	// give reset a positive pulse
			wait_ms(1);
			// clr_reset		;	set RESET = 0
			PORTB &= ~(1<<RESET);
			wait_ms(20);
	
			spi_out(0xac);
			spi_out(0x53);

			int count = 32;
			answer[1] = STATUS_CMD_FAILED;
			do {
				result = spi_in();
				//SendHex(result);
				if (result == 0x53) {
					answer[1] = STATUS_CMD_OK;
					break;
				}
				spi_out(0x00);
				PORTB |= (1<<SCK);
				asm("nop");
				asm("nop");
				asm("nop");
				PORTB &= ~(1<<SCK);
				spi_out(0xac);
				spi_out(0x53);
			} while(--count);

			spi_out(0x00);

			#if 0
			// the kommt das wirklich vom target oder liegt das noch am register?
			result = spi_in();	
			spi_out(0x00);

			answer[1] = result;

			/*
			//spi_in_init();
			if(result == 0x53)
			else
				answer[1] = STATUS_CMD_FAILED;
			*/	
			//answer[1] = STATUS_CMD_OK;
			#endif
/*
		spi_out(0x30);	
		spi_out(0x00);	
		spi_out(0x00);	
		result = spi_in();
		SendHex(result);
*/

			answer[0] = CMD_ENTER_PROGMODE_ISP;
			CommandAnswer(2);
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
		case CMD_SPI_MULTI:

		spi_out(0x30);	
		spi_out(0x00);	
		spi_out(0x01);	
		result = spi_in();
		SendHex(result);


		#if 0
			/* send command */
			for(i=0;i<4;i++)
				spi_out(buf[4+i]);

			/* create answer */
			answer[0] = CMD_SPI_MULTI;
			answer[1] = STATUS_CMD_OK;

			/* read answer */	
			// 2, 3 oder 4 da aendert sich was
			for(i=0;i<4;i++){
					if(i==3) {
						//answer[2+i] = spi_in();
/*
if((int)buf[6]==0)
	answer[2+i] =  spi_in();
if((int)buf[6]==1)
	answer[2+i] =  0x95;
if((int)buf[6]==2)
	answer[2+i] =  0x02;
*/
						//answer[2+i] = (int)buf[6];
					}
					else {
						//answer[2+i] = spi_in();
						spi_in();
						answer[2+i] = 0;
					}
			}

			answer[2+i] = STATUS_CMD_OK;
			CommandAnswer(2+i);
			#endif
		break;
		}
	}
}

int main(void)
{
  int conf, interf;
  UARTInit();

  spi_init();
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

  USBNAddInEndpoint(conf,interf,1,0x03,BULK,64,0,NULL);
  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&USBFlash);
  
  MCUCR |=  (1 << ISC01); // fallende flanke

  GICR |= (1 << INT0);
  sei();
  USBNInitMC();

  // start usb chip
  USBNStart();

  while(1);
}


