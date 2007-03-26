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
	jtagicestate = START;
	fifo_init (inbuf, inbuf_field, 200);
	fifo_init (outbuf, outbuf_field, 200);
}

void JTAGICE_common_state_machine(void)
{
	char sign;
	uint8_t counter=0;
	sign = fifo_get_wait(inbuf);	

	while(1) {
		
		switch(jtagicestate) {
			
			case START:
				if(sign==27) {
					// start timeout timer
					jtagicestate=GET_SEQUENCE_NUMBER;
				} else {
					jtagicestate=START;
				}
			break;

			case GET_SEQUENCE_NUMBER:
				counter=0;
				if(counter<2){
					sign = fifo_get_nowait(inbuf);
					counter++;
				}

				

			break;

			case GET_MESSAGE_SIZE:

			break;

			case GET_TOKEN:

			break;

			case GET_DATA:

			break;

			case GET_CRC:

			break;

			default:
				;
		}	
	}

}



