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
#include <stdint.h>
#include "../usbn2mc/fifo.h"


#define MESSAGE_START		0x1b
#define TOKEN						0x0e

#define CMND_GET_SIGN_ON	0x01



#define RSP_OK					0x80
#define RSP_FAILED			0xA0



#define RSP_SIGN_ON			0x86
#define RSP_SELFTEST		0x86



struct message_t {
	unsigned char start;
	unsigned short sequence_number;
 	unsigned long size;
	unsigned char token;
	unsigned char * body;
	unsigned char crc;
};

typedef enum {
	START,
	GET_SEQUENCE_NUMBER,
	GET_MESSAGE_SIZE,
	GET_TOKEN,
	GET_DATA,
	GET_CRC
} JTAGICE_STATE;

void JTAGICE_init(void);
//void JTAGICE_common_state_machine(void);




