/*
 * usbprog - A Downloader/Uploader for AVR device programmers
 * Copyright (C) 	2006,2007 Benedikt Sauter
 *		 						2007 Robert Schilling robert.schilling@gmx.at
									2008 Martin Lang <Martin.Lang@rwth-aachen.de>
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
#define TOKEN				0x0e

#define CMND_GET_SIGN_OFF		0x00
#define CMND_GET_SIGN_ON		0x01
#define CMND_SET_PARAMETER		0x02
#define CMND_GET_PARAMETER		0x03
#define CMND_WRITE_MEMORY		0x04
#define CMND_READ_MEMORY		0x05
#define CMND_WRITE_PC			0x06
#define CMND_READ_PC			0x07
#define CMND_GO				0x08
#define CMND_SINGLE_STEP		0x09
#define CMND_FORCED_STOP		0x0A
#define CMND_RESET			0x0B
#define CMND_SET_DEVICE_DESCRIPTOR	0x0C
#define CMND_GET_SYNC			0x0F
#define CMND_SET_BREAK			0x11
#define CMND_CLR_BREAK			0x1A
#define CMND_SELFTEST			0x10
#define CMND_CHIP_ERASE			0x13
#define CMND_ENTER_PROGMODE		0x14
#define CMND_LEAVE_PROGMODE		0x15

#define CMND_RESTORE_TARGET		0x23


//events are from 0xe0 - 0xff
#define EVT_BREAK			0xE0

// memory types
#define IO_SHADOW			0x30
#define SRAM				0x20
#define EEPROM				0x22
#define EVENT				0x60
#define SPM					0xA0
#define FLASH_PAGE			0xB0
#define EEPROM_PAGE			0xB1
#define FUSE_BITS			0xB2
#define LOCK_BITS			0xB3
#define SIGN_JTAG			0xB4
#define OSCCAL_BYTE			0xB5
#define CAN					0xB6


#define RSP_OK					0x80
#define RSP_FAILED				0xA0
#define RSP_SIGN_ON				0x86
#define RSP_SELFTEST			0x85
#define RSP_PARAMETER			0x81
#define RSP_ILLEGAL_BREAKPOINT 0xA8
#define RSP_ILLEGAL_MCU_STATE 0xA5
#define RSP_ILLEGAL_MEMORY_TYPE 0xA2
#define RSP_ILLEGAL_MEMORY_RANGE 0xA3
#define RSP_ILLEGAL_COMMAND 0xAA
#define RSP_ILLEGAL_PARAMETER 0xA1

// parameter
#define HARDWARE_VERSION		0x01
#define SOFTWARE_VERSION		0x02
#define EMULATOR_MODE			0x03
#define IREG					0x04
#define BAUD_RATE				0x05
#define OCD_VTARGET				0x06
#define OCD_JTAG_CLOCK			0x07
#define OCD_BREAK_CAUE			0x08
#define TIMERS_RUNNING			0x09
#define BREAK_ON_CHANGE_FLOW	0x0A
#define BREAK_ADDR1				0x0B
#define BREAK_ADDR2				0x0C
#define COMB_BREAK_CTRL			0x0D
#define JTAG_ID_STRING			0x0E
#define UNITS_BEFORE			0x0F
#define UNITS_AFTER				0x10
#define BIT_BEFORE				0x11
#define BIT_AFTER				0x12
#define EXTERNAL_RESET			0x13
#define FLASH_PAGE_SIZE			0x14
#define EEPROM_PAGE_SIZE		0x15
//#define PSB0					0x17
//#define PSB1					0x18
#define PROTOCOL_DEBUG_EVENT	0x19
#define TARGET_MCU_STATE		0x1A
#define DAISY_CHAIN_INFO		0x1B
#define BOOT_ADRESS				0x1C
#define TARGET_SIGNATURE		0x1D
#define DEBUG_WIRE_BAUDRATE		0x1E
#define PROGRAM_ENTRY_POINT		0x1F
#define PACKET_PARSING_ERRORS	0x40
#define VALID_PACKETS_RECIEVED	0x41
#define INTERCOM_TX_FAILURES	0x42
#define INTERCOM_RX_FALURES		0x43
#define CRC_ERROR				0x44
#define POWER_SOURCE			0x45
#define CAN_FLAG				0x22
#define PAR_ENABLE_IDR_IN_RUN_MODE				0x23
#define PAR_ALLOW_PAGEPROGRAMMING_IN_SCANCHAIN	0x24

#define MESSAGE_BUFFER_SIZE 320


//jtag cmds
//#define AVR_RESET			0x04
//#define AVR_PRG_ENABLE		0x04
//#define AVR_PRG_CMDS			0x05

int cmd_get_sign_on(char *msg, char * answer);
int cmd_sign_off(char *msg, char * answer);
int cmd_set_parameter(char *msg, char * answer);
int cmd_get_parameter(char *msg, char * answer);
int cmd_get_sync(char *msg, char *answer);
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
int cmd_selftest(char *msg, char *buf);
int cmd_write_memory(char *msg, char *answer);
int cmd_chip_erase(char *msg, char *answer);

int cmd_write_pc(char *msg, char *answer);

int rsp_ok(char *answer);
int rsp_failed(char *answer);
int rsp_illegal_breakpoint(char *answer);
int rsp_illegal_mcu_state(char *answer);
int rsp_illegal_memory_type(char *answer);
int rsp_illegal_memory_range(char *answer);
int rsp_illegal_command(char *answer);
int rsp_illegal_parameter(char *answer);

int evt_break(char *answer, uint16_t pc, uint8_t break_cause);


extern struct deviceDescriptor_t deviceDescriptor;


volatile struct jtagice_t {
  char lastcmd;
	int cmdpackage;
 	unsigned long size;
	volatile int datatogl;
	char seq1;				// sequence number
	char seq2;				// sequence number
	int emulatormode;
	char emulator_state;
	uint16_t pcmask;
} jtagice;

enum emulatormode_t {
	NOT_CONNECTED = 4,
	STOPPED = 0,
	RUNNING = 1,
	PROGRAMMING = 2
};

enum breakpoint_mode {
	break_memory_read = 0,
	break_memory_write = 1,
	break_memory_rw = 2,
	break_program = 3,
	break_mask = 4
};

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

struct deviceDescriptor_t
{
	unsigned char ucReadIO[8];		//LSB = IOloc 0, MSB = IOloc63
	unsigned char ucReadIOShadow[8];	//LSB = IOloc 0, MSB = IOloc63
	unsigned char ucWriteIO[8]; 		//LSB = IOloc 0, MSB = IOloc63
	unsigned char ucWriteIOShadow[8];	//LSB = IOloc 0, MSB = IOloc63
	unsigned char ucReadExtIO[52]; 		//LSB = IOloc 96, MSB = IOloc511
	unsigned char ucReadIOExtShadow[52];	//LSB = IOloc 96, MSB = IOloc511
	unsigned char ucWriteExtIO[52]; 	//LSB = IOloc 96, MSB = IOloc511
	unsigned char ucWriteIOExtShadow[52];	//LSB = IOloc 96, MSB = IOloc511
	unsigned char ucIDRAddress; 		//IDR address
	unsigned char ucSPMCRAddress; 		//SPMCR Register address and dW BasePC
	unsigned long ulBootAddress; 		//Device Boot Loader Start Address
	unsigned char ucRAMPZAddress; 		//RAMPZ Register address in SRAM I/O
	//space
	unsigned int uiFlashPageSize; 		//Device Flash Page Size, Size =
	//2 exp ucFlashPageSize
	unsigned char ucEepromPageSize; 	//Device Eeprom Page Size in bytes
	unsigned int uiUpperExtIOLoc; 		//Topmost (last) extended I/O
	//location, 0 if no external I/O
	unsigned long ulFlashSize; 		//Device Flash Size
	unsigned char ucEepromInst[20]; 	//Instructions for W/R EEPROM
	unsigned char ucFlashInst[3]; 		//Instructions for W/R FLASH
	unsigned char ucSPHaddr; 		// Stack pointer high
	unsigned char ucSPLaddr; 		// Stack pointer low
	unsigned int uiFlashpages; 		// number of pages in flash
	unsigned char ucDWDRAddress; 		// DWDR register address
	unsigned char ucDWBasePC; 		// Base/mask value of the PC
	unsigned char ucAllowFullPageBitstream; // FALSE on ALL new
	//parts
	unsigned int uiStartSmallestBootLoaderSection; //
	unsigned char EnablePageProgramming; 	// For JTAG parts only,
	// default TRUE
	unsigned char ucCacheType; 		// CacheType_Normal 0x00,
						// CacheType_CAN 0x01,
						// CacheType_HEIMDALL 0x02
	unsigned int uiSramStartAddr; 		// Start of SRAM
	unsigned char ucResetType; 		// Selects reset type. ResetNormal = 0x00
						// ResetAT76CXXX = 0x01
	unsigned char ucPCMaskExtended; 	// For parts with extended PC
	unsigned char ucPCMaskHigh; 		// PC high mask
	unsigned char ucEindAddress; 		// Selects reset type.
	unsigned int EECRAddress; 		// EECR IO address
};






