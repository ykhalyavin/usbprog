/*
 * jtagice - A Downloader/Uploader for AVR device programmers
 * Copyright (C) 2006 Benedikt Sauter
 * Copyright (C) 2008 Martin Lang <Martin.Lang@rwth-aachen.de>
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
#include <string.h>

#define F_CPU 16000000
#include <util/delay.h>

#include "wait.h"
#include "../usbprog_base/firmwarelib/avrupdate.h"
#include "uart.h"
#include "usbn2mc.h"

#include "jtag.h"
#include "jtagice2.h"
#include "jtag_avr.h"
#include "jtag_avr_ocd.h"
#include "crc.h"
#include "jtag_avr_prg.h"
#include "jtag_avr_defines.h"

unsigned char forcedStop = 0;

/*** prototypes and global vars ***/
/* send a command back to pc */
void CommandAnswer(int length);
void JTAGICE_ProcessCommand(unsigned char *localbuf);


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
  switch(req->bRequest)
  {
    case STARTAVRUPDATE:
      avrupdate_start();
    break;
  }
}

void USBToglAndSend(void)
{
  if(jtagice.datatogl == 1) {
    USBNWrite(TXC1, TX_LAST+TX_EN+TX_TOGL);
    jtagice.datatogl = 0;
  } else {
    USBNWrite(TXC1, TX_LAST+TX_EN);
    jtagice.datatogl = 1;
  }
}


volatile unsigned char answer[300];
/*
void CommandAnswer(int length)
{
  int i;

  USBNWrite(TXC1, FLUSH);
  for(i = 0; i < length; i++)
    USBNWrite(TXD1, answer[i]);

   // control togl bit
   USBToglAndSend();
} */

void CommandAnswer(int length)
{
	int i,pos;
	unsigned char res;
	pos = 0;
	USBNWrite(TXC1, FLUSH);
	// divide result in fifo sized packets
	while (length)  {
		// wait for tx complete
		do {
			res = USBNRead(TXC1);
		} while (res & TX_EN);
		// put data to FIFO
		USBNWrite(TXD1,answer[pos++]);
		for (i = 1; (i < 64) && (i < length); ++i) {
			USBNBurstWrite(answer[pos++]);
		}
		USBToglAndSend();
		length -= i; // substract data already sent
	}
}

/* called after data where send to pc */
void USBSend(void)
{

}

unsigned char buf[MESSAGE_BUFFER_SIZE];	// Recievebuffer for long packages
// static char recieveCounter = 0;	// not used now

/* is called when received data from pc */
void USBReceive(char *inbuf)
{
	static JTAGICE_STATE current_state = START;
	uint8_t read_pos = 0; // please note that this function get's everytime a 64 byte package out of USBN
	static uint16_t write_pos = 0;
	static uint16_t data_remain = 0;
	uint16_t data_to_read;

	PORTA |= (1<<PA4); // show that we are doing something *gg*

	while (read_pos < 64) {

		switch (current_state) {
			case START:
				if (inbuf[read_pos++] == MESSAGE_START) {
					current_state = GET_SEQUENCE_NUMBER;
					buf[0] = MESSAGE_START;
					write_pos = 1;
#ifdef DEBUG_VERBOSE
					UARTWrite("Start Token found\r\n");
#endif
				}
				break;
			case GET_SEQUENCE_NUMBER:
				if (write_pos++ == 1) {
					buf[1] = inbuf[read_pos++];
					jtagice.seq1 = buf[1];
				}
				else {
					buf[2] = inbuf[read_pos++];
					jtagice.seq2 = buf[2];
					current_state = GET_MESSAGE_SIZE;
#ifdef DEBUG_VERBOSE
					UARTWrite("Seq read:");
					SendHex(jtagice.seq1);
					SendHex(jtagice.seq2);
					UARTWrite("\r\n");
#endif
				}
				break;
			case GET_MESSAGE_SIZE:
#ifdef DEBUG_VERBOSE
				UARTWrite("Read Size\r\n");
#endif
				if (write_pos < 6) {
					buf[write_pos++] = inbuf[read_pos++];
				}
				else if (write_pos == 6) {
					buf[write_pos++] = inbuf[read_pos++];
					data_remain = *(uint16_t*)(buf + 3); // we ignore the upper 2 bytes of the package because our buffer would not be able to handle it
					current_state = GET_TOKEN;
#ifdef DEBUG_ON
					UARTWrite("Data Size ");
					SendHex((char)(data_remain >> 8));
					SendHex((char)data_remain);
					UARTWrite("\r\n");
#endif
				}
				break;
			case GET_TOKEN:
				if (inbuf[read_pos++] != TOKEN) {
					current_state = START;
#ifdef DEBUG_ON
					UARTWrite("Token error\r\n");
#endif
				}
				else {
					current_state = GET_DATA;
					write_pos++; // dont copy the token - no use
				}
				break;
			case GET_DATA:
				// try to get all remaining data from buffer at once
				data_to_read = ((64 - read_pos) < data_remain) ? 64 - read_pos : data_remain;
				if ((write_pos + data_to_read) >= MESSAGE_BUFFER_SIZE) {
#ifdef DEBUG_ON
					UARTWrite("InBuf Overflow\r\n");
#endif
					current_state = START;
				}
				data_remain -= data_to_read;
				while (data_to_read--) {
#ifdef DEBUG_VERBOSE
					UARTWrite("C:");
					SendHex((char)(write_pos >> 8));
					SendHex((char)write_pos);
					UARTWrite(":");
					SendHex(inbuf[read_pos]);
					UARTWrite("\r\n");
#endif
						buf[write_pos++] = inbuf[read_pos++];
				}
				if (!data_remain)
					current_state = GET_CRC;
				break;
			case GET_CRC:
				// ignore rest of packet first
				read_pos = 64;
				current_state = START;
#ifdef DEBUG_VERBOSE
					UARTWrite("Process ->\r\n");
#endif
				JTAGICE_ProcessCommand(buf);
#ifdef DEBUG_VERBOSE
					UARTWrite("-> Completed\r\n");
#endif
		}

	} // while
	PORTA &= ~(1<<PA4); // we do nothing anymore
}

void JTAGICE_ProcessCommand(unsigned char *localbuf) {
	int cmdlength=0;
UARTWrite("Com: ");
SendHex(buf[8]);
UARTWrite("\r\n");
	switch(localbuf[8]) {

			case CMND_GET_SIGN_ON:
				cmdlength = cmd_get_sign_on((char*)localbuf,(char *)answer);
			break;

			case CMND_GET_SIGN_OFF:
				cmdlength = cmd_sign_off((char*)localbuf,(char*)answer);
			break;

			case CMND_SET_PARAMETER:
				cmdlength = cmd_set_parameter((char*)localbuf,(char*)answer);
			break;

			case CMND_READ_MEMORY:
				cmdlength = cmd_read_memory((char*)localbuf,(char*)answer);
			break;

			case CMND_GET_PARAMETER:
				cmdlength = cmd_get_parameter((char*)localbuf,(char*)answer);
			break;
			case CMND_GET_SYNC:
				cmdlength = cmd_get_sync((char*)localbuf,(char*)answer);
			break;
			case CMND_FORCED_STOP:
				cmdlength = cmd_forced_stop((char*)localbuf,(char*)answer);
				forcedStop = 1;
			break;

			case CMND_SET_DEVICE_DESCRIPTOR:
				cmdlength = cmd_set_device_descriptor((char*)localbuf, (char*)answer);
			break;

			case CMND_GO:
				cmdlength = cmd_go((char*)localbuf,(char*)answer);
			break;

			case CMND_RESTORE_TARGET:
				cmdlength = cmd_restore_target((char*)localbuf,(char*)answer);
			break;

			case CMND_LEAVE_PROGMODE:
				cmdlength = cmd_leave_progmode((char*)localbuf,(char*)answer);
			break;

			case CMND_ENTER_PROGMODE:
				cmdlength = cmd_enter_progmode((char*)localbuf,(char*)answer);
			break;

			case CMND_RESET:
				cmdlength = cmd_reset((char*)localbuf,(char*)answer);
			break;

			case CMND_READ_PC:
				cmdlength = cmd_read_pc((char*)localbuf,(char*)answer);
			break;

			case CMND_SET_BREAK:
				cmdlength = cmd_set_break((char*)localbuf,(char*)answer);
			break;

			case CMND_CLR_BREAK:
				cmdlength = cmd_clr_break((char*)localbuf,(char*)answer);
			break;

			case CMND_SINGLE_STEP:
				cmdlength = cmd_single_step((char*)localbuf,(char*)answer);
			break;

			case CMND_SELFTEST:
				cmdlength = cmd_selftest((char*)localbuf, (char*)answer);
			break;

			case CMND_WRITE_MEMORY:
				cmdlength = cmd_write_memory((char*)localbuf, (char*)answer);
			break;

			case CMND_CHIP_ERASE:
				cmdlength = cmd_chip_erase((char*)localbuf, (char*)answer);
			break;

			case CMND_WRITE_PC:
				cmdlength = cmd_write_pc((char*)localbuf,(char*)answer);
			break;

			default:
				cmdlength = rsp_illegal_command((char*)answer);

		}
		if(cmdlength>0){
		  CommandAnswer(cmdlength);
		}
		// recalculate size
//		jtagice.size = jtagice.size -54;
/*
		if(forcedStop)
		{
			forcedStop = 0;
			wait_ms(5);
			answer[0] = MESSAGE_START;
			answer[1] = 0xff;
			answer[2] = 0xff;
			answer[3] = 0x06;
			answer[4] = 0;
			answer[5] = 0;
			answer[6] = 0;
			answer[7] = TOKEN;
			answer[8] = 0xe0;
			answer[9] = 0x59;
			answer[10] = 0;
			answer[11] = 0;
			answer[12] = 0;
			answer[13] = 0;
			crc16_append(answer,(unsigned long)14);
			CommandAnswer(16);
		}

		*/
}



int main(void) {
  int conf, interf;
	// only for testing
	//asm volatile ("push r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0\npush r0");

#ifdef DEBUG_ON
	UARTInit();
	UARTWrite("Hallo\r\n");
#endif

  USBNInit();
  jtagice.datatogl=1;
  jtagice.emulator_state = NOT_CONNECTED;
  avrContext.PSB0 = 0;
  avrContext.PSB1 = 0;
  avrContext.PDMSB = 0;
  avrContext.PDSB = 0;

  jtag_init();

  DDRA = (1 << DDA4);
  PORTA &= ~(1<<PA4); // this is the LED

  USBNDeviceVendorID(0x03eb);	//atmel ids
  USBNDeviceProductID(0x2103); // atmel ids

  USBNDeviceBCDDevice(0x0200);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  //USBNDeviceManufacture ("EmbeddedProjects");
  //USBNDeviceProduct	("USBprog JTAGICE2");
  USBNDeviceManufacture ("ATMEL");
  USBNDeviceProduct	("JTAGICE mkII");

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

	// ask for new events
	// while send an event block usb receive routine
	uint16_t delay = 0;
	uint8_t ledtimer = 0;

#ifdef DEBUG_VERBOSE
		UARTWrite("Main Loop\r\n");
#endif

	/* This is the program main loop.
	 * It maintains the state changes of the target mcu by cyclic checking
	 * of the debug registers.
	 * Notice that all messages are processed in the ISR of the USBN
	 */
	while (1) {
		if (delay++ != 0)
			continue;

		// when emulator is running cyclicly check whether it has met a break condidtion
		if (jtagice.emulator_state == RUNNING) {
			if (++ledtimer == 0)
				PORTA ^= (1<<PA4); // toggle led while running to signalize working ^^
			// check ocd BSR
			cli(); // does not respond on messages in this time

#ifdef DEBUG_VERBOSE
			UARTWrite("Check BSR:");
#endif

			uint8_t bsr;
			//debug_verbose = 1;
			rd_dbg_ocd(AVR_BSR, &bsr, 0);

#ifdef DEBUG_VERBOSE
			SendHex((char)(bsr>>8));
			SendHex((char)bsr);
			UARTWrite("\r\n");
#endif

			if (bsr != 0) {
				wait_ms(1);
				ocd_save_context();

#ifdef DEBUG_VERBOSE
				UARTWrite("Break!\r\nPC:");
				SendHex((uint8_t)(avrContext.PC>>8));
				SendHex((uint8_t)avrContext.PC);
				UARTWrite("\r\n");

				uint16_t data;
				rd_dbg_ocd(AVR_BCR,&data,0);
				UARTWrite("BCR:");
				SendHex(data>>8);
				SendHex(data);
				rd_dbg_ocd(AVR_BSR,&data,0);
				UARTWrite("\r\nBSR:");
				SendHex(data>>8);
				SendHex(data);
				rd_dbg_ocd(AVR_PSB0,&data,0);
				UARTWrite("\r\nPSB0:");
				SendHex(data>>8);
				SendHex(data);
				rd_dbg_ocd(AVR_PSB1,&data,0);
				UARTWrite("\r\nPSB1:");
				SendHex(data>>8);
				SendHex(data);
				rd_dbg_ocd(AVR_PDSB,&data,0);
				UARTWrite("\r\nPDSB:");
				SendHex(data>>8);
				SendHex(data);
				rd_dbg_ocd(AVR_PDMSB,&data,0);
				UARTWrite("\r\nPDMSB:");
				SendHex(data>>8);
				SendHex(data);
				UARTWrite("\r\n");
#endif

#ifdef DEBUG_ON
				uint8_t data;
				rd_dbg_ocd(AVR_BSR,&data,0);
				UARTWrite("BSR:");
				SendHex(data>>8);
				SendHex(data);
				UARTWrite("\r\n");
#endif

				// the following is the break type line
				uint8_t break_cause = 0;
				if (bsr & 0x10)
					break_cause = 3;
				else if (bsr & 0x00E1) {
					break_cause = 2;
				}

				if (bsr & ~0x3) {
					avrContext.PC--;
				}

				// clear all active breakpoints?!
				/* The AVR067 App Note says that breakpoints
				 * are cleared automaticly after a break.
				 * I don't know what the clear commands should do?
				 */
				PORTA |= (1<<PA4); // LED ON
				//avrContext.break_config &= 0xC000; // rule out all breakpoint configurations
				// wr_dbg_ocd(AVR_BCR,&avrContext.break_config,0); // this is no longer needed because it get's updated on restore context

				(void)evt_break((char *)answer, avrContext.PC, break_cause);
				CommandAnswer(16);
				jtagice.emulator_state = STOPPED;
				PORTA &= ~(1<<PA4); // LED OFF
			}
			sei(); // resume event processing

		}
	}
	// end testing
}


