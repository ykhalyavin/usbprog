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

#define CMND_GET_SIGN_ON		0x01
#define CMND_SET_PARAMETER	0x02
#define CMND_GET_PARAMETER	0x03
#define CMND_READ_MEMORY		0x05
#define CMND_READ_PC				0x07
#define CMND_SINGLE_STEP		0x09
#define CMND_FORCED_STOP		0x0A
#define CMND_SET_DEVICE_DESCRIPTOR	0x0C


//events are from 0xe0 - 0xff
#define EVT_BREAK		0xE0




#define RSP_OK					0x80
#define RSP_FAILED			0xA0



#define RSP_SIGN_ON			0x86
#define RSP_SELFTEST		0x86
#define RSP_PARAMETER		0x81


// parameter
#define EMULATOR_MODE					0x03


int cmd_get_sign_on(char *msg, char * answer);
int cmd_set_parameter(char * buf);
int cmd_get_parameter(char *msg, char * answer);
int cmd_read_memory(char * buf);
int cmd_read_pc(char * buf);
int cmd_single_step(char * buf);
int cmd_forced_stop(char * buf);
int cmd_set_device_descriptor(char * buf);


volatile struct jtagice_t {
  char lastcmd;
	int longpackage;
	int cmdpackage;
 	unsigned long size;
	int datatogl;
	char seq1;				// sequence number
	char seq2;				// sequence number
	int emulatormode;
} jtagice;


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




