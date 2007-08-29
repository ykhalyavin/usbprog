/*
 * jtagice - A Downloader/Uploader for AVR device programmers
 * Copyright (C) 2006,2007 Benedikt Sauter 
 *		 2007 Robert Schilling robert.schilling@gmx.at
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

#include "jtag_avr_defines.h"
#include "../usbn2mc/fifo.h"

// represent acutal state of state machine
//static JTAGICE_STATE jtagicestate;

// actuall message
volatile struct message_t msg;
struct deviceDescriptor_t deviceDescriptor;

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

	answer[8]	= RSP_SIGN_ON;		// page 57 datasheet
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
	crc16_append(answer,(unsigned long)36);
	return 38;
}

int cmd_sign_off(char * msg, char * answer)
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


int cmd_set_parameter(char *msg, char * answer)
{
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

	switch(msg[9]) {

		case EMULATOR_MODE:
			jtagice.emulatormode = answer[10];
			return 11;
		break;
		
		case DAISY_CHAIN_INFO:
			return 11;
		break;
		case OCD_JTAG_CLOCK:
			return 11;
		break;
		case TIMERS_RUNNING:	
			return 11;
		break;
		default:
			;
	}
	return 11;
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
	
	char signature[3];

	switch(msg[9])
	{
	case OCD_VTARGET:
		answer[3] = 0x03;	//length of Body
		
		answer[8] = RSP_PARAMETER;
		answer[9] = 0xFF;
		answer[10] = 0x20;
		
		crc16_append(answer,(unsigned long)11);
		return 13;
		
	break;
	
	case JTAG_ID_STRING:	
		// get id from target controller over jtag connection
		idcode(jtagbuf);
		
		answer[3] = 0x05;		// length of body
		answer[8] = RSP_PARAMETER;		// (0x80 = ok)
	
		answer[9] = jtagbuf[0];	//JTAG ID
		answer[10] = jtagbuf[1];
		answer[11] = jtagbuf[2];
		answer[12] = jtagbuf[3];
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

	return 0; 
}


int cmd_read_pc(char *msg, char * answer)
{
	unsigned char jtagbuf[4],recvbuf[4];
	jtag_reset();
	avr_jtag_instr(AVR_INSTR,0);

	jtagbuf[0]=0x00;
	jtagbuf[1]=0x00;
	jtagbuf[2]=0xff;
	jtagbuf[3]=0xff;
	jtag_write_and_read(32,jtagbuf,recvbuf);


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
	
	answer[9] = recvbuf[0]/2;
	answer[10] = recvbuf[1]/2;
	answer[11]= 0x00;
	answer[12]= 0x00;

	crc16_append(answer,(unsigned long)13);
	return 15;
}

int cmd_clr_break(char *msg, char * answer)
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

int cmd_set_break(char *msg, char * answer)
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


int cmd_single_step(char *msg, char * answer)
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


int cmd_forced_stop(char * msg, char * answer)
{
 	jtag_reset();
  avr_jtag_instr(AVR_FORCE_BRK, 0);

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

int cmd_go(char * msg, char * answer)
{
  jtag_reset();
  avr_jtag_instr(AVR_RUN, 0);

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

int cmd_restore_target(char * msg, char * answer)
{
	avr_reset(0);	// clear reset mode
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

int cmd_enter_progmode(char * msg, char * answer)
{
	avr_reset(1);
	avr_prog_enable();
	avr_prog_cmd();

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

int cmd_leave_progmode(char * msg, char * answer)
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


int cmd_reset(char * msg, char * answer)
{
	avr_reset(1); // force target controller into reset state

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


int cmd_read_memory(char * msg, char * answer)
{
	int length=8;
	int msglen=0;
	unsigned long len,startaddr;	// length
	
	// byte 2,3,4 of length = default value
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;


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
			answer[9] = rd_lfuse_avr();
			msglen = 2;
			answer[3] = 2;
		
			switch(msg[10])
				{
				case 1:
					if(msg[11] == 0)
					{
						answer[9] = rd_lfuse_avr();
						//msg[11]++;
					}
					else if(msg[11] == 1)
					{
						answer[9] = rd_hfuse_avr();
						//msg[11]++;
					}
					else
					{
						answer[9] = rd_efuse_avr();
						//msg[11] = 0;
					}
					msglen = 2;
					answer[3] = 2;			// length of body with ok
				break; 
				
				case 2:
					answer[9] = rd_lfuse_avr();
					answer[10] = rd_hfuse_avr();
					msglen = 3;
					answer[3] = 3;			// length of body with ok
				break;
				
				case 3:
					answer[9] = rd_lfuse_avr();
					answer[10] = rd_hfuse_avr();
					answer[11] = rd_efuse_avr();
					msglen = 4;
					answer[3] = 4;			// length of body with ok
				break;
			} 
			
			break;


		case SRAM:
			// flash lesen 
			len = msg[16]+(255*msg[17])+(512*msg[18])+(1024*msg[19]);	// length
			//startaddress
			startaddr = msg[20]+(255*msg[21])+(512*msg[22])+(1024*msg[23]);	// length
			answer[3] = (char)msg[16]+1;					// length of body with ok
			msglen=(int)msg[16]+1;
			//SendHex(msg[16]);
		
			answer[9]=0x00;
			answer[10]=0x00;
			answer[11]=0x00;
			answer[12]=0x00;
			//rd_sram_ocd_avr ((short)startaddr, &answer[9] ,(short)len ,0);

		break;

		case SPM:
			// flash lesen 
			len = msg[16]+(255*msg[17])+(512*msg[18])+(1024*msg[19]);	// length
			//startaddress
			startaddr = msg[20]+(255*msg[21])+(512*msg[22])+(1024*msg[23]);	// length
			
			// read data from flash and send back
			//answer[3] = (char)(startaddr+1);					// length of body with ok
			answer[3] = (char)msg[16]+1;					// length of body with ok
			msglen=(int)msg[16]+1;

			//init_avr_jtag (&(reg.avr), 0);
			char buf[20];
			rd_flash_ocd_avr ((short)msg[20], (unsigned char*) buf ,(short)msg[16] ,0);
			//rd_flash_ocd_avr ((short)startaddr, &buf ,(short)len ,0);
			answer[9]=buf[0];
			answer[10]=buf[1];
			//answer[9] - ...

		break;

		default:
			SendHex(0x88);
	}
	// TODO (program answer always with ok!)
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	 
	answer[7] = TOKEN;

	answer[8]	= 0x82;		// (0x80 = ok)

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
	switch(msg[9])
	{
		case LOCK_BITS:
		break;
		case FUSE_BITS:
			//wr_hfuse_avr(msg[18]);
		break;
		case SRAM:
		break;
		case SPM:
		break;
		case FLASH_PAGE:
			wr_flash_page((int) msg[10], (long) msg[14], &msg[18]);
		break;
		default:
		break;
	}
	
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;
	answer[8]	= RSP_OK;		// (0x80 = ok)
	
	crc16_append(answer,(unsigned long)9);
	
	return 11;
}

int cmd_set_device_descriptor(char *msg, char *answer)
{
	unsigned char i, j;
	
	for(i = 0, j = 9; i < 8; i++, j++)
		deviceDescriptor.ucReadIO[i] = msg[j];
		
	for(i = 0, j = 17; i < 8; i++, j++)
		deviceDescriptor.ucReadIOShadow[i] = msg[j];
		
	for(i = 0, j = 25; i < 8; i++, j++)
		deviceDescriptor.ucWriteIO[i] = msg[j];
		
	for(i = 0, j = 33; i < 8; i++, j++)
		deviceDescriptor.ucReadIOShadow[i] = msg[j];
	
	for(i = 0, j = 41; i < 93; i++, j++)
		deviceDescriptor.ucReadExtIO[i] = msg[j];
		
	for(i = 0, j = 93; i < 145; i++, j++)
		deviceDescriptor.ucReadIOExtShadow[i] = msg[j];
		
	for(i = 0, j = 145; i < 197; i++, j++)
		deviceDescriptor.ucWriteExtIO[i] = msg[j];
		
	for(i = 0, j = 197; i < 249; i++, j++)
		deviceDescriptor.ucWriteIOExtShadow[i] = msg[j];
		
	deviceDescriptor.ucIDRAddress = msg[249];
	deviceDescriptor.ucSPMCRAddress = msg[250];
	deviceDescriptor.ulBootAddress = *(unsigned long *)&msg[251];
	deviceDescriptor.ucRAMPZAddress = msg[255];
	deviceDescriptor.uiFlashPageSize = (msg[257] << 8) | msg[256];
	deviceDescriptor.ucEepromPageSize = msg[258];
	deviceDescriptor.uiUpperExtIOLoc = (msg[260] << 8) | msg[259]; 
	deviceDescriptor.ulFlashSize = *(unsigned long *)&msg[261];
	
	for(uint16_t k = 265, i = 0; i < 285; i++, k++)
		deviceDescriptor.ucEepromInst[i] = msg[k];
	
	deviceDescriptor.ucFlashInst[0] = msg[285];
	deviceDescriptor.ucFlashInst[1]	= msg[286];
	deviceDescriptor.ucFlashInst[2] = msg[287];
	deviceDescriptor.ucSPHaddr = msg[288];
	deviceDescriptor.ucSPLaddr = msg[289];
	deviceDescriptor.uiFlashpages = (msg[290] << 8) | msg[291];
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
	deviceDescriptor.EECRAddress = (msg[306] << 8) | msg[307];

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

int cmd_chip_erase(char *msg, char *answer)
{
	answer[0] = MESSAGE_START;
	answer[1] = jtagice.seq1;
	answer[2] = jtagice.seq2;
	answer[3] = 0x01;
	answer[4] = 0;
	answer[5] = 0;
	answer[6] = 0;
	answer[7] = TOKEN;
	answer[8]	= RSP_OK;		// (0x80 = ok)

	chip_erase();
	crc16_append(answer,(unsigned long)9);
	return 11;
}

