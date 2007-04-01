/*
 * jtagice - A Downloader/Uploader for AVR device programmers
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

#include "../usbprog_base/firmwarelib/avrupdate.h"
#include "uart.h"
#include "usbn2mc.h"

#include "jtag.h"
#include "jtagice2.h"
#include "crc.h"

/*** prototypes and global vars ***/
/* send a command back to pc */
void CommandAnswer(int length);


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
	//SendHex(req->bRequest);
	switch(req->bRequest)
	{
		case STARTAVRUPDATE:
			avrupdate_start();
		break;
	}
}

volatile char answer[300];

void CommandAnswer(int length)
{
	int i;
	USBNWrite(TXC1,FLUSH);
	for(i=0;i<length;i++){
		USBNWrite(TXD1,answer[i]);
	}

	//SendHex(0x11);
	/* control togl bit */

	if(jtagice.datatogl==1) {
		USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
		jtagice.datatogl=0;
	} else {
		USBNWrite(TXC1,TX_LAST+TX_EN);
		jtagice.datatogl=1;
	}
}

/* called after data where send to pc */
void USBSend()
{

}

/* is called when received data from pc */
void USBReceive(char *buf)
{
#if 0	
	if(jtagice.longpackage) {

		// recalculate message size
		jtagice.size = jtagice.size -64;
		if(jtagice.size==0)
			jtagice.longpackage = 0;

	} else {
#endif	

		// check if package is a cmdpackage
		if(buf[0]==MESSAGE_START)
			jtagice.cmdpackage=1;
		else
			jtagice.cmdpackage=0;
		
		if(jtagice.cmdpackage==1) {
			jtagice.seq1=buf[1];		// save sequence number
 			jtagice.seq2=buf[2];		// save sequence number
		}
/*
		// check if package is a longpackage
		jtagice.size = buf[3]+(255*buf[4])+(512*buf[5])+(1024*buf[6]);
		//jtagice.size = buf[3]+(buf[4]<<8)+(buf[5]<<16)+(buf[6]<<24);
*/
//		if(jtagice.size>54)
//			jtagice.longpackage = 1;
	
		int cmdlength=0;

		switch(buf[8]) {

			case CMND_GET_SIGN_ON:
				cmdlength = cmd_get_sign_on(&buf,&answer);
			break;

			case CMND_GET_SIGN_OFF:
				cmdlength = cmd_sign_off(&buf,&answer);
			break;

			case CMND_SET_PARAMETER:
				cmdlength = cmd_set_parameter(&buf,&answer);
			break;
		
			case CMND_READ_MEMORY:
				cmdlength = cmd_read_memory(&buf,&answer);
			break;
			
			case CMND_GET_PARAMETER:
				cmdlength = cmd_get_parameter(&buf,&answer);
			break;

			case CMND_FORCED_STOP:
				cmdlength = cmd_forced_stop(&buf,&answer);
			break;

			case CMND_SET_DEVICE_DESCRIPTOR:
				cmdlength = cmd_set_device_descriptor(&buf,&answer);
			break;
	
			case CMND_GO:
				cmdlength = cmd_go(&buf,&answer);
			break;
			
			case CMND_RESTORE_TARGET:
				cmdlength = cmd_restore_target(&buf,&answer);
			break;

			case CMND_LEAVE_PROGMODE:
				cmdlength = cmd_leave_progmode(&buf,&answer);
			break;

			case CMND_ENTER_PROGMODE:
				cmdlength = cmd_enter_progmode(&buf,&answer);
			break;

			case CMND_RESET:
				cmdlength = cmd_reset(&buf,&answer);
			break;
			
			case CMND_READ_PC:
				cmdlength = cmd_read_pc(&buf,&answer);
			break;



			default:
				answer[0]=RSP_FAILED;
				cmdlength=0;
		}
		if(cmdlength!=0)
			CommandAnswer(cmdlength);
		// recalculate size
		jtagice.size = jtagice.size -54;
	//}
}



int main(void)
{
  int conf, interf;
	// only for testing
  UARTInit();
  
	USBNInit();   
  
  jtagice.longpackage=0;
  jtagice.datatogl=1;

	jtag_init();

	DDRA = (1 << DDA4);
	PORTA &= ~(1<<PA4); //off

  USBNDeviceVendorID(0x03eb);	//atmel ids
  USBNDeviceProductID(0x2103); // atmel ids
  
  USBNDeviceBCDDevice(0x0200);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  
  USBNDeviceManufacture ("B.Sauter");
  USBNDeviceProduct	("JTAGICE mkII Klon");
  USBNDeviceSerialNumber("A000000D3F");

  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,USBSend);
  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&USBReceive);

  USBNInitMC();
  sei();

  // start usb chip
  USBNStart();

#if 0	
	#include "jtag_avr_defines.h"
	unsigned char jtagbuf[10];
	char recvbuf[10];

	// READ IDCODE
	jtag_goto_state(SHIFT_IR);
	jtagbuf[0]=AVR_IDCODE;
	jtag_write(4,jtagbuf);
	
	jtag_goto_state(SHIFT_DR);
	jtag_read(32,jtagbuf);
	sendhex(jtagbuf[0]);
	sendhex(jtagbuf[1]);
	sendhex(jtagbuf[2]);
	sendhex(jtagbuf[3]);

	asm("nop");
	//BYPASS TEST
	jtag_goto_state(SHIFT_IR);
	jtagbuf[0]=BYPASS;
	jtag_write(4,jtagbuf);
	jtag_goto_state(SHIFT_DR);
	jtagbuf[0]=0x11;
	jtagbuf[1]=0x11;
	recvbuf[0]=0x00;
	recvbuf[1]=0x00;
	jtag_write_and_read(16,jtagbuf,recvbuf);
	//SendHex(recvbuf[0]);
	//SendHex(recvbuf[1]);
	
	// READ IDCODE
	jtag_goto_state(SHIFT_IR);
	jtagbuf[0]=AVR_IDCODE;
	jtag_write(4,jtagbuf);
	
	jtag_goto_state(SHIFT_DR);
	jtag_read(32,jtagbuf);
	//SendHex(jtagbuf[0]);
	//SendHex(jtagbuf[1]);
	//SendHex(jtagbuf[2]);
	//SendHex(jtagbuf[3]);
	
	JTAG_CLEAR_TMS();
	JTAG_CLEAR_TDI();
	JTAG_CLEAR_TCK();
	JTAG_SET_TCK();
	JTAG_CLEAR_TCK();

	jtag_reset();

	JTAG_CLEAR_TMS();
	JTAG_CLEAR_TDI();
	JTAG_CLEAR_TCK();
	JTAG_SET_TCK();
	JTAG_CLEAR_TCK();

	// RESET
	jtag_goto_state(SHIFT_IR);
	jtagbuf[0]=AVR_RESET;
	jtag_write(4,jtagbuf);
	jtag_goto_state(SHIFT_DR);
	jtagbuf[0]=0x1;
	jtag_write(1,jtagbuf);
	
	// ENABLE PROG
	jtag_goto_state(SHIFT_IR);
	jtagbuf[0]=AVR_PRG_ENABLE;
	jtag_write(4,jtagbuf);
	jtag_goto_state(SHIFT_DR);
	jtagbuf[0]=0x70;
	jtagbuf[1]=0xA3;
	jtag_write(16,jtagbuf);
	//jtag_goto_state(UPDATE_DR);
	
	// wie kann man herausfinden ob sich der controller im programmiermodus befindet?

	// theoretisch sollte er sich hier im programmiermodus befinden


	// SIGNATURE BYTE
	jtag_goto_state(SHIFT_IR);
	jtagbuf[0]=AVR_PRG_CMDS;
	jtag_write(4,jtagbuf);
	
	jtag_goto_state(SHIFT_DR);
	jtagbuf[0]=0x04;	// enter signature byte read
	jtagbuf[1]=0x23; // 
	jtag_write(15,jtagbuf);
	jtag_goto_state(SHIFT_DR);

	//jtagbuf[0]=0x00;	// load address
	//jtagbuf[1]=0x36;
	jtagbuf[0]=0x00;	// load address
	jtagbuf[1]=0x32;

	jtag_write(15,jtagbuf);
	jtag_goto_state(SHIFT_DR);

	jtagbuf[0]=0x00;	// read signature
	//jtagbuf[1]=0x37;
	jtagbuf[1]=0x33;
	jtag_write_and_read(15,jtagbuf,recvbuf);
	jtag_goto_state(SHIFT_DR);

	//jtag_read(15,recvbuf);
	SendHex(recvbuf[0]);

#endif

	// ask for new events
	// while send an event block usb receive routine
	while(1);
	// end testing
}


