/*
 * jtagice - A Downloader/Uploader for AVR device programmers
 * Copyright (C) 2006,2007 Benedikt Sauter
 *		 2007 Robert Schilling robert.schilling@gmx.at
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

#include "jtagice2.h"
#include "jtag_avr_prg.h"
#include "jtag_avr_ocd.h"
#include "jtag_avr.h"
#include "uart.h"
#include "jtag.h"
#include "crc.h"
#include "avr_asm.h"

#include "jtag_avr_defines.h"
#include "../usbn2mc/fifo.h"

// represent acutal state of state machine
//static JTAGICE_STATE jtagicestate;

// actuall message
volatile struct message_t msg;
struct deviceDescriptor_t deviceDescriptor;

uint16_t global_pc;

int cmd_get_sign_on(char *msg, char * answer)
{
	jtag_reset();
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x1c;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]  = RSP_SIGN_ON;		// page 57 datasheet
	answer[9]  = 0x01;	// communication protocoll version
	answer[10] = 0xff;
	// orignal id answer[11] = 0x21;  //07
	// original id answer[12] = 0x04;
	answer[11] = 0x25;  //07
	answer[12] = 0x05;

	answer[13] = 0x00;
	answer[14] = 0xff;
	//original answer[15] = 0x21; //21
	//original answer[16] = 0x04;
	answer[15] = 0x25; //21
	answer[16] = 0x05;

	//original answer[17] = 0x00;
	answer[17] = 0x01;
	answer[18] = 0x00;

	answer[19] = 0xa0;	// serial number
	answer[20] = 0x00;
	answer[21] = 0x00;
	answer[22] = 0x0d;
	answer[23] = 0x3f;	// end of serial number
/* xmega 
	answer[19] = 0xB0;	// serial number
	answer[20] = 0x00;
	answer[21] = 0x00;
	answer[22] = 0x50;
	answer[23] = 0xAC;	// end of serial number
*/
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
	crc16_append(answer,(unsigned long)36);

#ifdef DEBUG_ON
	UARTWrite("DDRB:");
	SendHex(DDRB);
	UARTWrite("\r\n");
#endif


#ifdef DEBUG_ON
	UARTWrite("PSB0:");
	SendHex((char)(avrContext.PSB0>>8));
	SendHex((char)avrContext.PSB0);
	UARTWrite("\r\nPSB1:");
	SendHex((char)(avrContext.PSB1>>8));
	SendHex((avrContext.PSB1));
	UARTWrite("\r\nPDMSB:");
	SendHex((char)(avrContext.PDMSB>>8));
	SendHex((avrContext.PDMSB));
	UARTWrite("\r\nPDSB:");
	SendHex((char)(avrContext.PDSB>>8));
	SendHex((avrContext.PDSB));
	UARTWrite("\r\n");

	uint16_t addr = (uint16_t)&avrContext;
	UARTWrite("avrContextAddr:");
	SendHex((char)(addr>>8));
	SendHex((char)addr);
	UARTWrite("\r\nDDescAddr:");
	addr = (uint16_t)&deviceDescriptor;
	SendHex((char)(addr>>8));
	SendHex((char)addr);
	UARTWrite("\r\n");
#endif

	return 38;
}

int cmd_sign_off(char * msg, char * answer)
{
	// TODO (program answer always with ok!)
	jtagice.emulator_state = NOT_CONNECTED;

	return rsp_ok(answer);
}

int cmd_get_sync(char * msg, char * answer)
{
	jtag_reset();
	avr_jtag_instr(AVR_FORCE_BRK, 0);
	avr_reset(0);
	jtagice.emulator_state = STOPPED;
#ifdef DEBUG_ON
	UARTWrite("Device synced!\r\n");
#endif

	// TODO (program answer always with ok!)
	return rsp_ok(answer);
}



int cmd_set_parameter(char *msg, char * answer)
{

	switch(msg[9]) {

		case EMULATOR_MODE:
			jtagice.emulatormode = answer[10];
			return rsp_ok(answer);
		break;

		case DAISY_CHAIN_INFO:
			return rsp_ok(answer);
		break;
		case OCD_JTAG_CLOCK:
			return rsp_ok(answer);
		break;
		case TIMERS_RUNNING:
			return rsp_ok(answer);
		break;
		default:
			return rsp_illegal_parameter(answer);
	};
	return rsp_failed(answer);
}


int cmd_get_parameter(char *msg, char * answer)
{
	char jtagbuf[4];

	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[7] = TOKEN;

	answer[4] = 0;	//default length
	answer[5] = 0;
	answer[6] = 0;

//	char signature[3];

	switch(msg[9])
	{
	case OCD_VTARGET:
		answer[3] = 0x03;	//length of Body

		answer[8] = RSP_PARAMETER;
		answer[9] = 0x04;
		answer[10] = 0x14;

		crc16_append(answer,(unsigned long)11);
		return 13;

	break;

	case JTAG_ID_STRING:
		// get id from target controller over jtag connection
		idcode((unsigned char *)jtagbuf);

		answer[3] = 0x05;		// length of body
		answer[8] = RSP_PARAMETER;		// (0x80 = ok)

		answer[9] = jtagbuf[0];	//JTAG ID
		answer[10] = jtagbuf[1];
		answer[11] = jtagbuf[2];
		answer[12] = jtagbuf[3];
		
		//answer[9] = 0x3F; xmega
		//answer[10] = 0xC0;
		//answer[11] = 0x74;
		//answer[12] = 0x69;
		crc16_append(answer,(unsigned long)13);
		return 15;
	break;
	case TARGET_SIGNATURE:
		//rd_signature_avr(signature);
		answer[3] = 0x04;
		answer[8] = RSP_PARAMETER;
		answer[9] = 0x11;
		answer[10] = 0x22;
		answer[11] = 0x33;
		crc16_append(answer,(unsigned long)12);
		return 14;
	break;
	default:
	break;
	}

	return rsp_illegal_parameter(answer);
}


int cmd_read_pc(char *msg, char * answer)
{
	if (jtagice.emulator_state != STOPPED) {
		return rsp_illegal_mcu_state(answer);
	}

	uint16_t pc = avrContext.PC;

#ifdef DEBUG_ON
				UARTWrite("GETPC:");
				SendHex((uint8_t)(pc>>8));
				SendHex((uint8_t)pc);
				UARTWrite("\r\n");
#endif


	// TODO (program answer always with ok!)
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x05;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;

	answer[8]	= 0x84;		// (0x80 = ok)

	answer[9] = (char)(pc);
	answer[10] = (char)(pc>>8);
	answer[11]= 0x00;
	answer[12]= 0x00;

	crc16_append(answer,(unsigned long)13);
	return 15;
}

int cmd_write_pc(char *msg, char * answer)
{
	if (jtagice.emulator_state != STOPPED) {
		return rsp_illegal_mcu_state(answer);
	}

	// write the new pc to the restore buffer
	avrContext.PC = msg[9] | (msg[10] << 8);
#ifdef DEBUG_ON
	UARTWrite("SETPC: ");
	SendHex(msg[10]);
	SendHex(msg[9]);
	UARTWrite("\r\n");
#endif

	// TODO (program answer always with ok!)
	return rsp_ok(answer);

}

int cmd_clr_break(char *msg, char * answer)
{
//	int16_t addr = msg[10] | (msg[11] << 8);

	if (msg[9] == 1) {
		ocd_clr_psb0();
	}
	else if (msg[9] == 2) {
		ocd_clr_psb1();
	}
	else if (msg[9] == 3) {
		ocd_clr_pdmsb();
	}
	else if (msg[9] == 4) {
		ocd_clr_pdsb();
	}
	else {
		return rsp_illegal_breakpoint(answer);
	}

	// TODO: ALL Breakpoint types
	return rsp_ok(answer);
}

int cmd_set_break(char *msg, char * answer)
{
	/* Debugging Purposes */
/*	ocd_erase_flash_page(0);

	return rsp_ok(answer); */

	uint16_t addr = msg[11] | (msg[12] << 8);
	// check the type of bp
	if (msg[9] == 1) { // this should be a program memory breakpoint
		// what number?
		if (msg[10] == 1) {
			ocd_set_psb0(addr);
			return rsp_ok(answer);
		}
		else if (msg[10] == 2) {
			ocd_set_psb1(addr);
			return rsp_ok(answer);
		}
		else if (msg[10] == 3) {
			ocd_set_pdmsb(addr,break_program);
			return rsp_ok(answer);
		}
		else if (msg[10] == 4) {
			ocd_set_pdsb(addr,break_program);
			return rsp_ok(answer);
		}
		else
			return rsp_illegal_breakpoint(answer);
	}
	else if (msg[9] == 2) {
		// this is a data breakpoint
		if (msg[10] == 3) {
			ocd_set_pdmsb(addr,msg[15]);
			return rsp_ok(answer);
		}
		else if (msg[10] == 4) {
			ocd_set_pdsb(addr,msg[15]);
			return rsp_ok(answer);
		}
		else
			return rsp_illegal_breakpoint(answer);
	}
	else if (msg[9] == 3) {
		ocd_set_pdmsb(addr,break_mask);
		return rsp_ok(answer);
	}

	return rsp_failed(answer);
}


int cmd_single_step(char *msg, char * answer)
{
	if (jtagice.emulator_state == PROGRAMMING) {
		return rsp_illegal_mcu_state(answer);
	}

#ifdef DEBUG_ON
	UARTWrite("Single Stepping: ");
	SendHex(msg[10]);
	UARTWrite("\r\n");
#endif

	ocd_restore_context();

	uint16_t stepbit = AVR_BRK_STEP;
	wr_dbg_ocd(AVR_BCR, (unsigned char *)&stepbit, 0); // set the stepping bit remove other breakpoints

	avr_jtag_instr(AVR_RUN, 0);
	jtagice.emulator_state = RUNNING;

	return rsp_ok(answer);
}


int cmd_forced_stop(char * msg, char * answer)
{
 	//jtag_reset();
	avr_jtag_instr(AVR_FORCE_BRK, 0);

#ifdef DEBUG_ON
	UARTWrite("STOP!\r\n");
#endif


#ifdef DEBUG_VERBOSE
	uint16_t brk;
	rd_dbg_ocd(AVR_DBG_COMM_CTL,&brk,0);
	UARTWrite("OCDCTL: ");
	SendHex(brk>>8);
	SendHex((char)brk);
	UARTWrite("\r\n");
	rd_dbg_ocd(AVR_BSR,&brk,0);
	UARTWrite("BSR: ");
	SendHex(brk>>8);
	SendHex((char)brk);
	UARTWrite("\r\n");
#endif


	// TODO (program answer always with ok!)
	return rsp_ok(answer);
}

int cmd_go(char * msg, char * answer)
{
	if (jtagice.emulator_state == PROGRAMMING) {
		return rsp_illegal_mcu_state(answer);
	}

//  jtag_reset();
	//avr_reset(0);
	// restore possibly overridden registers
	// this is needed to clear out the 3 instructions that are used to restore the working registers to their normal state
	ocd_restore_context();

	avr_jtag_instr(AVR_RUN, 0);
	jtagice.emulator_state = RUNNING;

#ifdef DEBUG_ON
	uint16_t brk;
	rd_dbg_ocd(AVR_DBG_COMM_CTL, (unsigned char *)&brk, 0);
	UARTWrite("OCDCTL: ");
	SendHex(brk>>8);
	SendHex((char)brk);
	UARTWrite("\r\n");
	rd_dbg_ocd(AVR_BCR, (unsigned char *)&brk, 0);
	UARTWrite("BCR: ");
	SendHex(brk>>8);
	SendHex((char)brk);
	UARTWrite("\r\n");
#endif

	// TODO (program answer always with ok!)
	return rsp_ok(answer);
}

int cmd_restore_target(char * msg, char * answer)
{
	// TODO: Remove software breakpoints in case

	// Reset Target

	avr_reset(1);	// clear reset mode
	jtag_reset(); // reset the complete OCD Interface
	avr_reset(0); // run target

	// TODO (program answer always with ok!)
	return rsp_ok(answer);
}

int cmd_enter_progmode(char * msg, char * answer)
{
	avr_reset(1);
	avr_prog_enable();
	jtagice.emulator_state = PROGRAMMING;

	avr_prog_cmd();

	return rsp_ok(answer);
}

int cmd_leave_progmode(char * msg, char * answer)
{
	// TODO (program answer always with ok!)
	avr_prog_disable();
	jtagice.emulator_state = STOPPED;

	return rsp_ok(answer);
}


int cmd_reset(char * msg, char * answer)
{
	avr_reset(1); // force target controller into reset state
	avr_jtag_instr(AVR_FORCE_BRK,0);
	avr_reset(0);
	ocd_save_context();

	jtagice.emulator_state = STOPPED;

	// TODO (program answer always with ok!)
	return rsp_ok(answer);
}


int cmd_read_memory(char * msg, char * answer)
{
	int length=8;
	int msglen=0;
	unsigned long len,startaddr;	// length

	// byte 2,3,4 of length = default value
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;

#ifdef DEBUG_ON
	UARTWrite("R:"); // Display R:addr:len
	SendHex(msg[9]);
	UARTWrite(":");
	SendHex(msg[16]);
	SendHex(msg[15]);
	SendHex(msg[14]);
	UARTWrite(":");
	SendHex(msg[12]);
	SendHex(msg[11]);
	SendHex(msg[10]);
	UARTWrite("\r\n");
#endif

	// decode generic parameters
	len = ((unsigned long)msg[13] << 24) |
		((unsigned long)msg[12] << 16) | (msg[11] << 8) | msg[10];
	startaddr = ((unsigned long)msg[17] << 24) |
		((unsigned long)msg[16] << 16) | (msg[15] << 8) | msg[14];


	//// !!!!!!!!!!!!!!!!!!!!!! TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// Length Check for the len Parameters to prohibit Buffer overflow in answer!!!

//	char jtagbuf[6];
	//SendHex(msg[15]);
	switch(msg[9]) {
		case LOCK_BITS:
			//SendHex(0xff);
			msglen=2;
			answer[3] = 2;					// length of body with ok
			answer[9]	= rd_lock_avr();		// (lock bits)
		break;

		case FUSE_BITS:
			switch(msg[10])
				{
				case 1:
					if(msg[14] == 0) // read low fuse?
					{
						answer[9] = rd_lfuse_avr();
					}
					else if(msg[14] == 1) // read high fuse?
					{
						answer[9] = rd_hfuse_avr();
					}
					else // otherwise return efuse
					{
						answer[9] = rd_efuse_avr();
					}
					msglen = 2;
				break;

				case 2:
					if (msg[14] == 0) {
						answer[9] = rd_lfuse_avr();
						answer[10] = rd_hfuse_avr();
					}
					else if (msg[14] == 1) {
						answer[9] = rd_hfuse_avr();
						answer[10] = rd_efuse_avr();
					}
					msglen = 3;
				break;

				case 3:
					answer[9] = rd_lfuse_avr();
					answer[10] = rd_hfuse_avr();
					answer[11] = rd_efuse_avr();
					msglen = 4;
				break;
			}

			break;

		case EEPROM:
			// read from registerspace, io/memory or sram
			msglen = len+1;
			// todo: range checks against the device parameters

			ocd_rd_eeprom((uint16_t)startaddr,(uint16_t)len, (uint8_t *)&answer[9]);
			break;
		case SRAM:
			// read from registerspace, io/memory or sram
			msglen = len+1;
			// todo: range checks against the device parameters

			ocd_rd_sram((uint16_t)startaddr,(uint16_t)len,(uint8_t *)&answer[9]);

		break;

		case SPM:
			// read from registerspace, io/memory or sram
			msglen = len+1;
			// todo: range checks against the device parameters

			ocd_rd_flash((uint16_t)startaddr,(uint16_t)len,(uint8_t *)&answer[9]);
		break;

		case OSCCAL_BYTE:
			answer[9] = rd_cal_byte(msg[14]);
			msglen=2;
			answer[3] = 2;					// length of body with ok
		break;

		case SIGN_JTAG:
			rd_signature_avr((unsigned char *)&answer[9]);
			//msglen = 4;
			//answer[3] = 4;					// length of body with ok

			switch(msg[14]){
			  case 0:
			    msglen = 2;
			    answer[3] = 2;
			  break;
			  case 1:
			    msglen = 2;
			    answer[3] = 2;
			    answer[9] = answer[10];
			  break;
			  case 2:
			    msglen = 2;
			    answer[3] = 2;
			    answer[9] = answer[11];
			  break;
			  default:
			    ;
			}
/*
			// xmega test
			msglen = 4;
			answer[3] = 4;
			answer[8] = 0x82;
			answer[9] = 0x1E;
			answer[10] = 0x97;
			answer[11] = 0x4C;
*/
		break;

		case FLASH_PAGE:
		{
#ifdef DEBUG_ON
			UARTWrite("flash");
#endif
			rd_flash_page(len, startaddr, (uint8_t *)&answer[9]);
			msglen = len + 1;
		}
		break;

		case EEPROM_PAGE:
			rd_eeprom_page((msg[11] << 8) | msg[10], (msg[15] << 8) | msg[14], (uint8_t *)&answer[9]);
			answer[3] = msg[10] + 1;
			msglen = answer[3];
		break;


		default:
#ifdef DEBUG_ON
			SendHex(0x88);
#endif
			break;
	}
	// TODO (program answer always with ok!)
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	// write message len to answer packet
	answer[3] = msglen & 0xFF;			// length of body with ok
	answer[4] = (msglen >> 8) & 0xFF;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;
	answer[8]	= 0x82;		// (0x80 = ok)

	// afterwards the handler above places its data

	length = length+msglen;
	crc16_append(answer,(unsigned long)length);
	return length+2;
}

int cmd_selftest(char *msg, char *answer)
{
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;
	answer[8] = RSP_SELFTEST;
	crc16_append(answer,(unsigned long)9);
	return 11;
}

int cmd_write_memory(char *msg, char *answer)
{
#ifdef DEBUG_ON
	UARTWrite("W:"); // Display W:addr:len
	SendHex(msg[9]);
	UARTWrite(":");
	SendHex(msg[16]);
	SendHex(msg[15]);
	SendHex(msg[14]);
	UARTWrite(":");
	SendHex(msg[12]);
	SendHex(msg[11]);
	SendHex(msg[10]);;
	UARTWrite("\r\n");
#endif
	switch(msg[9])
	{
		case LOCK_BITS:
			wr_lock_avr(msg[18]);
		break;
		case FUSE_BITS:
			if (jtagice.emulator_state != PROGRAMMING) {
				return rsp_illegal_mcu_state(answer);
			}
			switch(msg[10]) {
				case 1:
					if(msg[14] == 0) // read low fuse?
					{
						wr_lfuse_avr(msg[18]);
					}
					else if(msg[14] == 1) // read high fuse?
					{
						wr_hfuse_avr(msg[18]);
					}
					else if (msg[14] == 2)// otherwise return efuse
					{
						wr_efuse_avr(msg[18]);
					}
					else {
						return rsp_illegal_memory_range(answer);
					}
					return rsp_ok(answer);
				break;

				case 2:
					if (msg[14] == 0) {
						wr_lfuse_avr(msg[18]);
						wr_hfuse_avr(msg[19]);
					}
					else if (msg[14] == 1) {
						wr_hfuse_avr(msg[18]);
						wr_efuse_avr(msg[19]);
					}
					else {
						return rsp_illegal_memory_range(answer);
					}
					return rsp_ok(answer);
				break;

				case 3:
					if (msg[14] == 0) {
						wr_lfuse_avr(msg[18]);
						wr_hfuse_avr(msg[19]);
						wr_efuse_avr(msg[20]);
						return rsp_ok(answer);
					}
					else {
						return rsp_illegal_memory_range(answer);
					}
				break;
				};

		break;
		case EEPROM:
			ocd_wr_eeprom((msg[15] << 8) | msg[14], (msg[11] << 8) | msg[10], (uint8_t *)&msg[18]);
			return rsp_ok(answer);
		break;

		case SRAM:
			ocd_wr_sram((msg[15] << 8) | msg[14], (msg[11] << 8) | msg[10], (uint8_t *)&msg[18]);
			return rsp_ok(answer);
		break;

		case SPM:
			wr_flash_page((msg[11] << 8) | msg[10], ((unsigned long) msg[16] << 16) | ((unsigned long) msg[15] << 8) | msg[14], (uint8_t *)&msg[18]);
			return rsp_ok(answer);
		break;

		case FLASH_PAGE:
			wr_flash_page((msg[11] << 8) | msg[10], ((unsigned long) msg[16] << 16) | ((unsigned long) msg[15] << 8) | msg[14], (uint8_t *)&msg[18]);
			return rsp_ok(answer);
		break;

		case EEPROM_PAGE:
			wr_eeprom_page(msg[10], (msg[15] << 8) | msg[14], (unsigned char *) &msg[18]);
			return rsp_ok(answer);
		break;

		default:
			return rsp_illegal_memory_type(answer);
		break;
	}

	return rsp_failed(answer);
}

int cmd_set_device_descriptor(char *msg, char *answer)
{
	unsigned char i, j;

	for(i = 9, j = 7; i < 17; i++, j--)
		deviceDescriptor.ucReadIO[j] = msg[i];

	for(i = 17, j = 7; i < 25; i++, j--)
		deviceDescriptor.ucReadIOShadow[j] = msg[i];

	for(i = 25, j = 7; i < 33; i++, j--)
		deviceDescriptor.ucWriteIO[j] = msg[i];

	for(i = 33, j = 7; i < 41; i++, j--)
		deviceDescriptor.ucReadIOShadow[j] = msg[i];

	for(i = 41, j = 51; i < 93; i++, j--)
		deviceDescriptor.ucReadExtIO[j] = msg[i];

	for(i = 93, j = 51; i < 145; i++, j--)
		deviceDescriptor.ucReadIOExtShadow[j] = msg[i];

	for(i = 145, j = 51; i < 197; i++, j--)
		deviceDescriptor.ucWriteExtIO[j] = msg[i];

	for(i = 197, j = 51; i < 249; i++, j--)
		deviceDescriptor.ucWriteIOExtShadow[j] = msg[i];

	deviceDescriptor.ucIDRAddress = msg[249];
	deviceDescriptor.ucSPMCRAddress = msg[250];
	deviceDescriptor.ulBootAddress = ((long) msg[254] << 24) | ((long) msg[253] << 16) | (msg[252] << 8) | msg[25];
	deviceDescriptor.ucRAMPZAddress = msg[255];
	deviceDescriptor.uiFlashPageSize = (msg[256] << 8) | msg[257];
	deviceDescriptor.ucEepromPageSize = msg[258];
	deviceDescriptor.uiUpperExtIOLoc = (msg[259] << 8) | msg[260];
	deviceDescriptor.ulFlashSize = ((long) msg[264] << 24) | ((long) msg[263] << 16) | (msg[262] << 8) | msg[261];

	// for debugging purposes - although this is a dirty hack
	jtagice.pcmask = (uint16_t)((deviceDescriptor.ulFlashSize>>1)-1);
#ifdef DEBUG_ON
	UARTWrite("Set PCMASK to:");
	SendHex((char)(jtagice.pcmask>>8));
	SendHex((char)jtagice.pcmask);
	UARTWrite("\r\n");
#endif

#ifdef DEBUG_VERBOSE
	UARTWrite("IDR Addr:");
	SendHex(deviceDescriptor.ucIDRAddress);
	UARTWrite("\r\nFlashPZ:");
	SendHex((char)(deviceDescriptor.uiFlashPageSize >> 8));
	SendHex((char)deviceDescriptor.uiFlashPageSize);
	UARTWrite("\r\nFlashS:");
	SendHex((char)(deviceDescriptor.ulFlashSize >> 16));
	SendHex((char)(deviceDescriptor.ulFlashSize >> 8));
	SendHex((char)deviceDescriptor.ulFlashSize);
	UARTWrite("\r\n");
#endif

	for(uint16_t k = 265, i = 0; k < 285; i++, k++)		//da passt ev. noch was nicht !!
		deviceDescriptor.ucEepromInst[i] = msg[k];

	deviceDescriptor.ucFlashInst[0] = msg[285];
	deviceDescriptor.ucFlashInst[1]	= msg[286];
	deviceDescriptor.ucFlashInst[2] = msg[287];

	deviceDescriptor.ucSPHaddr = msg[288];
	deviceDescriptor.ucSPLaddr = msg[289];
	deviceDescriptor.uiFlashpages = (msg[291] << 8) | msg[290];
	deviceDescriptor.ucDWDRAddress = msg[292];
	deviceDescriptor.ucDWBasePC = msg[293];
	deviceDescriptor.ucAllowFullPageBitstream = msg[294];
	deviceDescriptor.uiStartSmallestBootLoaderSection = (msg[296] << 8) | msg[295];
	deviceDescriptor.EnablePageProgramming = msg[297];
	deviceDescriptor.ucCacheType = msg[298];
	deviceDescriptor.uiSramStartAddr = (msg[300] << 8) | msg[299];
	deviceDescriptor.ucResetType = msg[301];
	deviceDescriptor.ucPCMaskExtended = msg[302];
	deviceDescriptor.ucPCMaskHigh = msg[303];
	deviceDescriptor.ucEindAddress = msg[304];
	deviceDescriptor.EECRAddress = (msg[306] << 8) | msg[305];

	return rsp_ok(answer);
}

int cmd_chip_erase(char *msg, char *answer)
{
	if (jtagice.emulator_state != PROGRAMMING) {
		return rsp_illegal_mcu_state(answer);
	}

	chip_erase();

	return rsp_ok(answer);
}


int rsp_ok(char *answer) {
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;
	answer[8]	= RSP_OK;		// (0x80 = ok)
	crc16_append(answer,(unsigned long)9);
	return 11;
}

int rsp_failed(char *answer) {
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;
	answer[8]	= RSP_FAILED;
	crc16_append(answer,(unsigned long)9);
	return 11;
}

int rsp_illegal_breakpoint(char *answer) {
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;
	answer[8]	= RSP_ILLEGAL_BREAKPOINT;
	crc16_append(answer,(unsigned long)9);
	return 11;
}

int rsp_illegal_mcu_state(char *answer) {
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x02;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;
	answer[8]	= RSP_ILLEGAL_MCU_STATE;
	answer[9] = jtagice.emulator_state;
	crc16_append(answer,(unsigned long)8);
	return 12;
}

int rsp_illegal_memory_type(char *answer) {
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;
	answer[8]	= RSP_ILLEGAL_MEMORY_TYPE;
	crc16_append(answer,(unsigned long)9);
	return 11;
}

int rsp_illegal_memory_range(char *answer) {
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;
	answer[8]	= RSP_ILLEGAL_MEMORY_RANGE;
	crc16_append(answer,(unsigned long)9);
	return 11;
}

int rsp_illegal_command(char *answer) {
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;
	answer[8]	= RSP_ILLEGAL_COMMAND;
	crc16_append(answer,(unsigned long)9);
	return 11;
}

int rsp_illegal_parameter(char *answer) {
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;					// length of body
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;
	answer[8]	= RSP_ILLEGAL_PARAMETER;
	crc16_append(answer,(unsigned long)9);
	return 11;
}


int evt_break(char *answer, uint16_t pc, uint8_t break_cause) {
	answer[0] = MESSAGE_START;
	answer[1] = 0xff;
	answer[2] = 0xff;
	answer[3] = 0x06;
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;
	answer[8] = 0xe0;
	answer[9] = (pc & 0xFF);
	answer[10] = (pc >> 8);
	answer[11] = 0;
	answer[12] = 0;
	answer[13] = break_cause;

	crc16_append(answer,(unsigned long)14);
	return 16;
}
