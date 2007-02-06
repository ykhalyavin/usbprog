/*
 * usbprog - A Downloader/Uploader for AVR device programmers
 * Copyright (C) 2006 Benedikt Sauter 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#define F_CPU 16000000
#include <util/delay.h>

#include "wait.h"

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

volatile struct usbprog_t {
	char lastcmd;
	int longpackage;
	int cmdpackage;
	int datatogl;
} usbprog;

volatile char answer[300];

struct pgmmode_t {
	unsigned short numbytes;
	uint8_t mode;
	uint8_t delay;
	uint8_t cmd1;
	uint8_t cmd2;
	uint8_t cmd3;
	uint8_t poll1;
	uint8_t poll2;
	uint32_t address;
} pgmmode;


SIGNAL(SIG_UART_RECV)
{
 //Terminal(UARTGetChar());
 //UARTWrite("usbn>");
}


SIGNAL(SIG_INTERRUPT0)
{
  USBNInterrupt();
}

/* id need for live update of firmware */

void USBNDecodeVendorRequest(DeviceRequest *req)
{
	//UARTWrite("vendor request check ");
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
	
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1)|(1<<SPR0);	//128tel
	//SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|3;
  //SPSR = (0<<SPI2X);
}

void spi_out(char data)
{
  	SPDR = data;
    while ( !(SPSR & (1<<SPIF)) ) ;
}


char spi_in()
{
	SPDR = 0;
	while(!(SPSR & (1<<SPIF)));
	return SPDR;
}


void spi_write_program_memory(char data)
{


}

void spi_write_program_memory_page(unsigned short wordaddr,char wordl,char wordh)
{

	char addrh,addrl;

	addrl = (char)wordaddr;
	addrh = (char)(wordaddr>>8);
/*
	UARTWrite("\r\n write data low ");
	SendHex(wordl);
	UARTWrite(" high ");
	SendHex(wordh);
	UARTWrite(" addr ");
	SendHex(addrh);
	SendHex(addrl);
	UARTWrite("\r\n");
*/


	// write low
	spi_out(0x40);
	spi_out(addrh);
	spi_out(addrl);
	spi_out(wordl);

	// write high
	spi_out(0x48);
	spi_out(addrh);
	spi_out(addrl);
	spi_out(wordh);

}

/* programm finite state machine 
 *
 */
void flash_program_fsm(char * buf)
{
	int bufindex=0,bytes=0;
	
	//if package is first one subtract 10 (control bytes overhead)
	if (usbprog.cmdpackage == 1) {
		bufindex = 10;
		//usbprog.cmdpackage = 0;
	}

	// if page mode
	if(pgmmode.mode && 0x01) {
		if(usbprog.cmdpackage==1) {
			//UARTWrite("\r\ncmd");
			// darf man nur max 54 bytes schreiben
			if(pgmmode.numbytes > 54) {
				bytes = 64;
				pgmmode.numbytes = pgmmode.numbytes -54;
			}
			else {
				bytes = pgmmode.numbytes;
				pgmmode.numbytes =0;
			}
			
			for(bufindex;bufindex < bytes;bufindex=bufindex+2){
				// load low word
				// load high word
				//SendHex(pgmmode.address>>8);
				//SendHex(pgmmode.address);
				spi_write_program_memory_page(pgmmode.address,buf[bufindex],buf[bufindex+1]);
				pgmmode.address++;
				// inc pgmmode.address
			}
			usbprog.cmdpackage = 0;
		}
		else {
			//UARTWrite("\r\ndata");
			// sonst max 64
			if(pgmmode.numbytes > 64) {
				bytes = 64;
				pgmmode.numbytes = pgmmode.numbytes -64;
			}
			else {
				bytes = pgmmode.numbytes;
				pgmmode.numbytes =0;
			}

			for(bufindex=0;bufindex < bytes;bufindex=bufindex+2){
				// load low word
				// load high word
				//SendHex(pgmmode.address>>8);
				//SendHex(pgmmode.address);
				spi_write_program_memory_page(pgmmode.address,buf[bufindex],buf[bufindex+1]);
				// inc pgmmode.address
				pgmmode.address++;
			}

		}

		// falls numbytes ==0
		if(pgmmode.numbytes==0) {
			// write page
			//UARTWrite("\r\n write page at addr ");
			uint16_t pageno;

			// calculate page number
			pageno = ((pgmmode.address/0x40)-1)*0x40;
			if((pgmmode.address % 0x40) > 0)
				pageno = pageno + 0x40;

			//SendHex(pageno);
			//UARTWrite("\r\n");
	
			pgmmode.address - 0x40;
			spi_out(0x4c);
			spi_out((char)(pageno>>8));	//high
			spi_out((char)(pageno));		//low
			spi_out(0x00);
			wait_ms(10);
		
			
			// und usb answer
			answer[0] = CMD_PROGRAM_FLASH_ISP;
			answer[1] = STATUS_CMD_OK;
			CommandAnswer(2);
			usbprog.longpackage=0;
		}

	}
	// else word mode
	else {
			// spaeter
	}
}


void CommandAnswer(int length)
{
	int i;

 	USBNWrite(TXC1,FLUSH);
	for(i=0;i<length;i++)
		USBNWrite(TXD1,answer[i]);

	/* control togl bit */

	if(usbprog.datatogl==1) {
		USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
		usbprog.datatogl=0;
	} else {
		USBNWrite(TXC1,TX_LAST+TX_EN);
		usbprog.datatogl=1;
	}
}


/* central command parser */
void USBFlash(char *buf)
{
	int i; 
	char result;
	int count = 32;
	uint16_t numberofbytes;
	if(usbprog.longpackage) {
		if(usbprog.lastcmd == CMD_PROGRAM_FLASH_ISP)
		{
			flash_program_fsm(buf);
		}

		return;
	}
	else {
		usbprog.lastcmd = buf[0];	
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
			// do we like, all commands are successfully
			answer[0] = CMD_SET_PARAMETER;
			answer[1] = STATUS_CMD_OK;
			CommandAnswer(2);
		break;
		case CMD_GET_PARAMETER:
			answer[0] = CMD_GET_PARAMETER;
			answer[1] = STATUS_CMD_OK;
		
			switch(buf[1]){
				case PARAM_STATUS_TGT_CONN:
					answer[2] = STATUS_ISP_READY;
				break;

				case PARAM_SW_MAJOR:	// avrisp mkII special 
					answer[2] = 1;
				break;

				case PARAM_SW_MINOR:	// abrisp mkII special
					answer[2] = 5;
				break;

				case PARAM_HW_VER:
					answer[2] = 0;
				break;

				case PARAM_VTARGET:
					answer[2] = 42;
				break;
	
				case PARAM_SCK_DURATION:
					answer[2] = 3; // 1MHz
				break;	

				default:
					answer[2] = 0x00; // FIXME all is not perfect!
			}
			CommandAnswer(3);

		break;
		case CMD_OSCCAL:

		break;

		case CMD_READ_OSCCAL_ISP:
			spi_out(buf[2]);	
			spi_out(buf[3]);	
			spi_out(buf[4]);	
			result = spi_in();
			
			answer[0] = CMD_READ_OSCCAL_ISP;
			answer[1] = STATUS_CMD_OK;
			answer[2] = result;
			answer[3] = STATUS_CMD_OK;
			CommandAnswer(4);

		break;

		case CMD_LOAD_ADDRESS:
			// save address
			//buf[1],buf[2],buf[3],buf[4]
			// msb first
			SendHex(buf[1]);
			SendHex(buf[2]);
			SendHex(buf[3]);
			SendHex(buf[4]);
			
			//set given address
			pgmmode.address = (buf[1]<<24)|(buf[2]<<16)|(buf[3]<<8)|(buf[4]);
			//loadaddress = 0;
			answer[0] = CMD_LOAD_ADDRESS;
			answer[1] = STATUS_CMD_OK;
			CommandAnswer(2);
		break;
		case CMD_FIRMWARE_UPGRADE:

		break;
		case CMD_RESET_PROTECTION:
			answer[0] = CMD_RESET_PROTECTION;
			answer[1] = STATUS_CMD_OK;	// this command returns always ok!
		break;
		case CMD_ENTER_PROGMODE_ISP:
			PORTA |= (1<<PA4);	//on
			PORTB &= ~(1<<RESET); //on
			//cbi	portb,SCK	; clear SCK
			PORTB &= ~(1<<SCK);
			
			// set_reset		;	set RESET = 1
			//PORTB |= (1<<RESET);	// give reset a positive pulse
			// clr_reset		;	set RESET = 0
			//PORTB &= ~(1<<RESET);
			wait_ms(100);
	
			spi_out(0xac);
			spi_out(0x53);

			answer[1] = STATUS_CMD_FAILED;
			int syncloops = buf[4];
			for (syncloops;syncloops>0;syncloops--) {
				result = spi_in();
				//SendHex(result);
				if (result == buf[6]) {	//0x53 for avr
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
			};

			spi_out(0x00);

			answer[0] = CMD_ENTER_PROGMODE_ISP;
			CommandAnswer(2);
		break;
		case CMD_LEAVE_PROGMODE_ISP:
			PORTA &= ~(1<<PA4); //off
			PORTB |= (1<<RESET);	// give reset a positive pulse off
			answer[0] = CMD_LEAVE_PROGMODE_ISP;
			answer[1] = STATUS_CMD_OK;
			CommandAnswer(2);
			usbprog.datatogl=0;	// to be sure that togl is on next session clear

		break;
		case CMD_CHIP_ERASE_ISP:
			spi_out(buf[3]);		
			spi_out(buf[4]);		
			spi_out(0x00);		
			spi_out(0x00);		
			wait_ms(buf[1]);	// 9ms
			answer[0] = CMD_CHIP_ERASE_ISP;
			answer[1] = STATUS_CMD_OK;
			CommandAnswer(2);
		break;
		case CMD_PROGRAM_FLASH_ISP:
			//UARTWrite("\r\nfla");
			// buf[1] = msb number of bytes
			// buf[2] = lsb number of bytes
			pgmmode.numbytes = (buf[1]<<8)|(buf[2]);

			// 	-> set longpackage = 1 if greate than 54
			if(pgmmode.numbytes>54)
				usbprog.longpackage = 1;

			// buf[3] = mode
			pgmmode.mode = buf[3];
			// buf[4] = delay
			pgmmode.delay = buf[4];

			// buf[5] = spi command for load page and write program memory (one byte at a time)
			pgmmode.cmd1 = buf[5];
			// buf[6] = spi command for write program memory page (one page at a time)
			pgmmode.cmd2 = buf[6];
			// buf[7] = spi command for read program memory
			pgmmode.cmd3 = buf[7];

			usbprog.cmdpackage = 1;
			flash_program_fsm(buf);
			usbprog.cmdpackage = 0;
		break;

		case CMD_READ_FLASH_ISP:
			
			pgmmode.numbytes = (buf[1]<<8)|(buf[2]); // number of bytes
			pgmmode.cmd3 = buf[3];	// read command
			// collect max first 62 bytes
			for(pgmmode.numbytes;pgmmode.numbytes > 0; pgmmode.numbytes--) {
				spi_out(pgmmode.cmd3);
				spi_out(pgmmode.address>>8);
				spi_out(pgmmode.address);
				answer[pgmmode.address+2]=spi_in();
				pgmmode.address++;
			}
			
			// then toggle send next read bytes
			// and finish with status_cmd_ok

			answer[0] = CMD_READ_FLASH_ISP;
			answer[1] = STATUS_CMD_FAILED;
			CommandAnswer(2);
		break;

		case CMD_READ_LOCK_ISP:
			spi_out(buf[2]);	
			spi_out(buf[3]);	
			spi_out(buf[4]);	
			result = spi_in();
			
			answer[0] = CMD_READ_LOCK_ISP;
			answer[1] = STATUS_CMD_OK;
			answer[2] = result;
			answer[3] = STATUS_CMD_OK;
			CommandAnswer(4);
		break;

		case CMD_PROGRAM_EEPROM_ISP:

		break;
		case CMD_READ_EEPROM_ISP:

		break;
		case CMD_PROGRAM_FUSE_ISP:
			spi_out(buf[1]);	
			spi_out(buf[2]);	
			spi_out(buf[3]);	
			spi_out(buf[4]);	
	
			answer[0] = CMD_PROGRAM_FUSE_ISP;
			answer[1] = STATUS_CMD_OK;
			answer[2] = STATUS_CMD_OK;
			CommandAnswer(3);

		break;
		case CMD_READ_FUSE_ISP:
			spi_out(buf[2]);	
			spi_out(buf[3]);	
			spi_out(buf[4]);	
			result = spi_in();
			
			answer[0] = CMD_READ_FUSE_ISP;
			answer[1] = STATUS_CMD_OK;
			answer[2] = result;
			answer[3] = STATUS_CMD_OK;
			CommandAnswer(4);
		break;
		case CMD_READ_SIGNATURE_ISP:
			spi_out(buf[2]);	
			spi_out(buf[3]);	
			spi_out(buf[4]);	
			result = spi_in();
			
			answer[0] = CMD_READ_SIGNATURE_ISP;
			answer[1] = STATUS_CMD_OK;
			answer[2] = result;
			answer[3] = STATUS_CMD_OK;
			CommandAnswer(4);
		break;
		case CMD_SPI_MULTI:
			spi_out(buf[4]);	
			spi_out(buf[5]);	
			spi_out(buf[6]);	
			
			// instruction
			switch(buf[4]) {	
				
				// read low flash byte
				case 0x20:
					result = spi_in();
				break;
				
				// read high flash byte
				case 0x28:
					result = spi_in();
				break;

				// read signature
				case 0x30:
					result = spi_in();
				break;
				
				// read lfuse
				case 0x50:
					result = spi_in();
				break;
					
			  // read lock
				case 0x38:
					result = spi_in();
				break;

				// read hfuse and lock
				case 0x58:
					result = spi_in();
				break;
				
				// read eeprom memory
				case 0xa0:
					result = spi_in();
				break;
				
				//write eeprom
				case 0xc0:
					spi_out(buf[7]);
					result = 0x00;
				break;
			}

			answer[2]=0x00;
			answer[3]=0x00;
			answer[4]=0x00;
			answer[5]=result;

			answer[0] = CMD_SPI_MULTI;
			answer[1] = STATUS_CMD_OK;

			answer[6] = STATUS_CMD_OK;
			CommandAnswer(3+buf[2]);

		break;
		}
	}
}


int main(void)
{
  int conf, interf;
  //UARTInit();

  spi_init();
  USBNInit();   
  
  usbprog.longpackage=0;

/* usbprog ids 
  USBNDeviceVendorID(0x1781);
  USBNDeviceProductID(0x0c62);
  USBNDeviceBCDDevice(0x0200);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  
  USBNDeviceManufacture ("Benedikt Sauter - www.ixbat.de  ");
  USBNDeviceProduct	("usbprog AVR Programmer");
  USBNDeviceSerialNumber("200612261");
 
  USBNAddInEndpoint(conf,interf,1,0x03,BULK,64,0,NULL);
  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&USBFlash);

*/

	DDRA = (1 << DDA4);
  //PORTA |= (1<<PA4);	//on
	PORTA &= ~(1<<PA4); //off

  USBNDeviceVendorID(0x03eb);	//atmel ids
  USBNDeviceProductID(0x2104); // atmel ids
  
  USBNDeviceBCDDevice(0x0200);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  
  USBNDeviceManufacture ("B.Sauter");
  USBNDeviceProduct	("usbprog ");
  USBNDeviceSerialNumber("0000A00128255");

	//0000A0016461 (aktuelle)
	//0000A0019647
	//0000A0000252

  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,NULL);
  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&USBFlash);
  
  USBNInitMC();
  sei();

  // start usb chip
  USBNStart();
  while(1);
}


