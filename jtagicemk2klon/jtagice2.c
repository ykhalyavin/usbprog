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

#include "../usbprog_base/firmwarelib/avrupdate.h"
#include "uart.h"
#include "jtag.h"

static JTAGICE_STATE jtagicestate;

void JTAGICE_init()
{
	jtagicestate = START;
}

void JTAGICE_common_state_machine(void)
{
	char sign;
	sign = fifo_get_nowait(recvfifo);	

	while(1) {
	
		switch(state) {
			case START:

			break;
			case GET_SEQUENCE_NUMBER:

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
		}	
	}

}



