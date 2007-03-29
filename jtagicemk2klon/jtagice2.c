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

#include "jtagice2.h" 
#include "uart.h"
#include "jtag.h"

#include "../usbn2mc/fifo.h"


// represent acutal state of state machine
static JTAGICE_STATE jtagicestate;

// actuall message
volatile struct message_t msg;

void JTAGICE_init()
{
	//jtagicestate = START;
	//fifo_init (inbuf, inbuf_field, 200);
	//fifo_init (outbuf, outbuf_field, 200);
}

void JTAGICE_common_state_machine(void)
{
#if 0
	char sign[1];
	uint8_t counter=0;
	sign[0] = fifo_get_wait(inbuf);	

	while(1) {
		
		switch(jtagicestate) {
			
			case START:
				if(sign[0]==27) {
					// start timeout timer
					msg.start = sign[0];
					jtagicestate=GET_SEQUENCE_NUMBER;
				} else {
					jtagicestate=START;
				}
			break;

			case GET_SEQUENCE_NUMBER:
				counter=0;
				if(counter<2){
					if(fifo_get_nowait(inbuf,sign))
					{
						msg.sequence_number[counter];	
					}else {
						// error fifo epmty
						jtagicestate=START;
					}
					counter++;
				} else {
					if(counter==2) {
						jtagicestate=GET_MESSAGE_SIZE;
					}
					else {
						// error timeout
						jtagicestate=START;
					}
				}
			break;

			case GET_MESSAGE_SIZE:
				counter=0;
				if(counter<4){
					if(fifo_get_nowait(inbuf,sign))
					{
						msg.size[counter]=sign;	
					}else {
						// error fifo epmty
						jtagicestate=START;
					}
					counter++;
				} else {
					if(counter==4) {
						jtagicestate=GET_TOKEN;
					}
					else {
						// error timeout
						jtagicestate=START;
					}
				}

			break;

			case GET_TOKEN:
				if(fifo_get_nowait(inbuf,sign))
			break;

			case GET_DATA:

			break;

			case GET_CRC:

			break;

			default:
				// error unknown state
				jtagicestate=START;
		}	
	}
#endif
}



int cmd_get_sign_on(char * buf)
{
	buf[0] = MESSAGE_START;
	buf[1] = jtagice.seq1;
	buf[2] = jtagice.seq2;
	buf[3] = 0x1c;					// length of body
	buf[4] = 0;
	buf[5] = 0;
	buf[6] = 0;
	buf[7] = TOKEN;

	buf[8]	= RSP_SELFTEST;		// page 57 datasheet
	buf[9]	= 0x01;	// communication protocoll version
	buf[10] = 0xff;	
	buf[11] = 0x07;
	buf[12] = 0x04;
	buf[13] = 0x00;
	buf[14] = 0xff;
	buf[15] = 0x14;
	buf[16] = 0x04;
	buf[17] = 0x00;
	buf[18] = 0x00;

	buf[19] = 0xa0;	// serial number
	buf[20] = 0x00;
	buf[21] = 0x00;
	buf[22] = 0x0d;
	buf[23] = 0x3f;	// end of serial number

	buf[24] = 'J';
	buf[25] = 'T';
	buf[26] = 'A';
	buf[27] = 'G';
	buf[28] = 'I';
	buf[29] = 'C';
	buf[30] = 'E';
	buf[31] = 'm';
	buf[32] = 'k';
	buf[33] = 'I';
	buf[34] = 'I';
	buf[35] = 0x00;
	buf[36] = 0x00;
	buf[37] = 0x00;
	crc16_append(buf,36);
	return 38;
}



int cmd_set_parameter(char * buf)
{
	switch(buf[9]) {

		case EMULATOR_MODE:
			jtagice.emulatormode = buf[10];
			buf[0] = MESSAGE_START;
			buf[1] = jtagice.seq1;
			buf[2] = jtagice.seq2;
			buf[3] = 0x01;					// length of body
			buf[4] = 0;
			buf[5] = 0;
			buf[6] = 0;
			buf[7] = TOKEN;

			buf[8]	= 0xAB;		// page 57 datasheet no target power
			crc16_append(buf,9);
			return 11;

		break;
		default:
			;
	}

}



