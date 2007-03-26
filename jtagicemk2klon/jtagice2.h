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

struct message_t {
	unsigned char start;
	unsigned short sequence_number;
 	unsigned long size;
	unsigned char token;
	unsigned char * body;
	unsigned char crc;
};

fifo_t * inbuf;
fifo_t * outbuf;

char inbuf_field[200];
char outbuf_field[200];

typedef enum {
	START,
	GET_SEQUENCE_NUMBER,
	GET_MESSAGE_SIZE,
	GET_TOKEN,
	GET_DATA,
	GET_CRC
} JTAGICE_STATE;

void JTAGICE_init(void);
void JTAGICE_common_state_machine(void);




