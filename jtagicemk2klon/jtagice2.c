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
	//fifo_init (inanswer, inbuf_field, 200);
	//fifo_init (outanswer, outbuf_field, 200);
}

void JTAGICE_common_state_machine(void)
{
#if 0
	char sign[1];
	uint8_t counter=0;
	sign[0] = fifo_get_wait(inanswer);	

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
					if(fifo_get_nowait(inanswer,sign))
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
					if(fifo_get_nowait(inanswer,sign))
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
				if(fifo_get_nowait(inanswer,sign))
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



int cmd_get_sign_on(char *msg, char * answer)
{
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x1c;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]	= RSP_SELFTEST;		// page 57 datasheet
	answer[9]	= 0x01;	// communication protocoll version
	answer[10] = 0xff;	
	answer[11] = 0x07;
	answer[12] = 0x04;
	answer[13] = 0x00;
	answer[14] = 0xff;
	answer[15] = 0x14;
	answer[16] = 0x04;
	answer[17] = 0x00;
	answer[18] = 0x00;

	answer[19] = 0xa0;	// serial number
	answer[20] = 0x00;
	answer[21] = 0x00;
	answer[22] = 0x0d;
	answer[23] = 0x3f;	// end of serial number

	answer[24] = 'J';
	answer[25] = 'T';
	answer[26] = 'A';
	answer[27] = 'G';
	answer[28] = 'I';
	answer[29] = 'C';
	answer[30] = 'E';
	answer[31] = 'm';
	answer[32] = 'k';
	answer[33] = 'I';
	answer[34] = 'I';
	answer[35] = 0x00;
	answer[36] = 0x00;
	answer[37] = 0x00;
	crc16_append(answer, (unsigned long)36);
	return 38;
}


int cmd_set_parameter(char *msg, char * answer)
{
	switch(msg[9]) {

		case EMULATOR_MODE:
			jtagice.emulatormode = answer[10];
			answer[0] = MESSAGE_START;
			answer[1] = jtagice.seq1;
			answer[2] = jtagice.seq2;
			answer[3] = 0x01;					// length of body
			answer[4] = 0;
			answer[5] = 0;
			answer[6] = 0;
			answer[7] = TOKEN;

			answer[8]	= 0x80;		// page 57 datasheet 0xab = no target power (0x80 = ok)
			crc16_append(answer,(unsigned long)9);
			return 11;

		break;
		default:
			;
	}

}


int cmd_get_parameter(char *msg, char * answer)
{
  jtag_goto_state(SHIFT_DR);

	char jtagbuf[4];
	jtag_read(32,jtagbuf);
	
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x05;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]	= 0x81;		// (0x80 = ok)

	answer[9] = jtagbuf[0];
	answer[10] = jtagbuf[1];
	answer[11] = jtagbuf[2];
	answer[12] = jtagbuf[3];
	crc16_append(answer,(unsigned long)13);
	return 15;
}

int cmd_read_memory(char * answer)
{
}

int cmd_read_pc(char * answer)
{
}

int cmd_single_step(char * answer)
{
}

int cmd_forced_stop(char * msg, char * answer)
{
	// TODO (program answer always with ok!)
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]	= 0x80;		// (0x80 = ok)
	crc16_append(answer,(unsigned long)9);
	return 11;
}

int cmd_set_device_descriptor(char * answer)
{
}

