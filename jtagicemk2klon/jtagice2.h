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

#define CMND_GET_SIGN_OFF		0x00
#define CMND_GET_SIGN_ON		0x01
#define CMND_SET_PARAMETER	0x02
#define CMND_GET_PARAMETER	0x03
#define CMND_READ_MEMORY		0x05
#define CMND_READ_PC				0x07
#define CMND_GO							0x08
#define CMND_SINGLE_STEP		0x09
#define CMND_FORCED_STOP		0x0A
#define CMND_RESET					0x0B
#define CMND_SET_DEVICE_DESCRIPTOR	0x0C
#define CMND_SET_BREAK			0x11
#define CMND_CLR_BREAK			0x1A

#define CMND_ENTER_PROGMODE	0x14
#define CMND_LEAVE_PROGMODE	0x15

#define CMND_RESTORE_TARGET	0x23


//events are from 0xe0 - 0xff
#define EVT_BREAK		0xE0

// memory types
#define SRAM						0x20
#define SPM							0xA0
#define FUSE_BITS				0xb2
#define LOCK_BITS				0xb3


#define RSP_OK					0x80
#define RSP_FAILED			0xA0



#define RSP_SIGN_ON			0x86
#define RSP_SELFTEST		0x86
#define RSP_PARAMETER		0x81

// parameter
#define EMULATOR_MODE					0x03


//jtag cmds
#define AVR_RESET				0x04
#define AVR_PRG_ENABLE	0x04
#define AVR_PRG_CMDS		0x05

int cmd_get_sign_on(char *msg, char * answer);
int cmd_sign_off(char *msg, char * answer);
int cmd_set_parameter(char *msg, char * answer);
int cmd_get_parameter(char *msg, char * answer);
int cmd_read_memory(char *msg, char * answer);
int cmd_read_pc(char *msg, char * answer);
int cmd_single_step(char *msg, char * answer);
int cmd_forced_stop(char *msg, char * answer);
int cmd_set_device_descriptor(char * msg, char * answer);
int cmd_go(char * msg, char * answer);
int cmd_restore_target(char * msg, char * answer);
int cmd_enter_progmode(char * msg, char * answer);
int cmd_leave_progmode(char * msg, char * answer);
int cmd_reset(char * msg, char * answer);
int cmd_set_break(char * msg, char * answer);
int cmd_clr_break(char * msg, char * answer);



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




