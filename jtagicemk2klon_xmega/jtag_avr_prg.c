/*-------------------------------------------------------------------------
* JTAG_AVR_PRG.C
* Copyright (C) 2007 Benedikt Sauter <sauter@ixbat.de>
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
*
*----------------------------------------------------------------------*/

#include "jtag_avr_prg.h"
#include "jtag_avr.h"
#include "jtag_avr_defines.h"
#include "jtag.h"
#include "wait.h"
#include "uart.h"
#include "jtag_avr_ocd.h"
#include "jtagice2.h"

unsigned char rd_lock_avr(void)
{
	unsigned char jtagbuf[2];
	avr_prog_cmd();
	avr_sequence(0x23,0x04,jtagbuf);	//enter fuse lock bits
	avr_sequence(0x36,0x00,jtagbuf);	// select lfuse
	avr_sequence(0x37,0x00,jtagbuf);	// read lock
	return jtagbuf[0];
}

unsigned char rd_efuse_avr(void)
{
	unsigned char jtagbuf[2];
	avr_prog_cmd();
	avr_sequence(0x23,0x04,jtagbuf);	//enter fuse lock bits
	avr_sequence(0x3A,0x00,jtagbuf);	// select lfuse
	avr_sequence(0x3B,0x00,jtagbuf); // read lfuse
	return jtagbuf[0];
}

unsigned char rd_lfuse_avr(void)
{
	unsigned char jtagbuf[2];
	avr_prog_cmd();
	avr_sequence(0x23,0x04,jtagbuf);	//enter fuse lock bits
	avr_sequence(0x32,0x00,jtagbuf);	// select lfuse
	avr_sequence(0x33,0x00,jtagbuf); // read lfuse
	return jtagbuf[0];
}


unsigned char rd_hfuse_avr(void)
{
	unsigned char jtagbuf[2];
	avr_prog_cmd();
	avr_sequence(0x23,0x04,jtagbuf);	//enter fuse lock bits
	avr_sequence(0x3E,0x00,jtagbuf);	// select hfuse
	avr_sequence(0x3F,0x00,jtagbuf); // read hfuse
	return jtagbuf[0];
}

void rd_fuse_avr (unsigned char *buf, int withextend)
{
}


void rd_signature_avr (unsigned char *signature)
{
	unsigned char tmp[2];
	avr_prog_cmd();
	avr_sequence(0x23,0x08,tmp);
	avr_sequence(0x03,0x00,tmp);
	avr_sequence(0x32,0x00,tmp);
	avr_sequence(0x33,0x00,&signature[0]);

	avr_sequence(0x23,0x08,tmp);
	avr_sequence(0x03,0x01,tmp);
	avr_sequence(0x32,0x00,tmp);
	avr_sequence(0x33,0x00,&signature[1]);

	avr_sequence(0x23,0x08,tmp);
	avr_sequence(0x03,0x02,tmp);
	avr_sequence(0x32,0x00,tmp);
	avr_sequence(0x33,0x00,&signature[2]);
}


void wr_hfuse_avr(unsigned char hfuse)
{
	unsigned char tmp[2];
	avr_prog_cmd();
	avr_sequence(0x23,0x40,tmp);
	avr_sequence(0x13,hfuse,tmp);
	avr_sequence(0x37,0x00,tmp);
	avr_sequence(0x35,0x00,tmp);
	avr_sequence(0x37,0x00,tmp);
	avr_sequence(0x37,0x00,tmp);
	avr_sequence(0x37,0x00,tmp);
}

void wr_lfuse_avr(unsigned char lfuse)
{
	unsigned char tmp[2];
	avr_prog_cmd();
	avr_sequence(0x23,0x40,tmp);
	avr_sequence(0x13,lfuse,tmp);
	avr_sequence(0x33,0x00,tmp);
	avr_sequence(0x31,0x00,tmp);
	avr_sequence(0x33,0x00,tmp);
	avr_sequence(0x33,0x00,tmp);
	avr_sequence(0x33,0x00,tmp);
}

void wr_efuse_avr(unsigned char efuse)
{
	unsigned char tmp[2];
	avr_prog_cmd();
	avr_sequence(0x23,0x40,tmp);
	avr_sequence(0x13,efuse,tmp);
	avr_sequence(0x3B,0x00,tmp);
	avr_sequence(0x39,0x00,tmp);
	avr_sequence(0x3B,0x00,tmp);
	avr_sequence(0x3B,0x00,tmp);
	avr_sequence(0x37,0x00,tmp);
}

void wr_lock_avr(unsigned char lock)
{
	unsigned char tmp[2];
	avr_prog_cmd();
	avr_sequence(0x23,0x20,tmp);
	avr_sequence(0x13,lock | 0xC0 ,tmp);
	avr_sequence(0x33,0x00,tmp);
	avr_sequence(0x31,0x00,tmp);
	avr_sequence(0x33,0x00,tmp);
	avr_sequence(0x33,0x00,tmp);

	do
	{
		avr_sequence(0x33,0x00,tmp);
	}
	while(!(tmp[1] & 0x02));
}


unsigned char rd_cal_byte(unsigned char adress)
{
	unsigned char tmp[2];
	avr_prog_cmd();
	avr_sequence(0x23,0x08,tmp);
	avr_sequence(0x03,adress,tmp);
	avr_sequence(0x36,0x00,tmp);
	avr_sequence(0x37,0x00,tmp);
	return tmp[0];
}



void chip_erase(void)
{
	unsigned char tmp[2];
	avr_prog_cmd();
	avr_sequence(0x23,0x80,tmp);
	avr_sequence(0x31,0x80,tmp);
	avr_sequence(0x33,0x80,tmp);
	avr_sequence(0x33,0x80,tmp);

	do
	{
		avr_sequence(0x23,0x08,tmp);
	}
	while(!(tmp[1] & 0x02));
}




void wr_flash_page(unsigned int byteCount, unsigned long adress, unsigned char *data)
{
#if 0
	/*char tmp[2];

	int adr = adress >> 1;


	//UARTWrite("Write Flash Page Address: ");
	SendHex((adr >> 8) & 0xFF);
	SendHex(adr & 0xFF);
	UARTWrite("\n\r");

	avr_prog_cmd();					//Prog Enable
	avr_sequence(0x23, 0x10, tmp);
	avr_sequence(0x07, (adr >> 8) & 0xFF, tmp);
	avr_sequence(0x03, adr & 0xFF , tmp);
	clear_ZPointer();
	avr_jtag_instr(AVR_PRG_PAGE_LOAD, 0);	//inst = PAGE_LOAD 0x06


	for(int i = 0; i < byteCount; i++)
	{
		SendHex(data[i]);
		UARTWrite("\n\r");
		jtag_write(7, &data[i]);
	}

	jtag_goto_state(UPDATE_DR);
	avr_prog_cmd();					//Prog Enable

	avr_sequence(0x37, 0x00, tmp);
	avr_sequence(0x35, 0x00, tmp);
	avr_sequence(0x37, 0x00, tmp);
	avr_sequence(0x37, 0x00, tmp);

	do
	{
		avr_sequence(0x37, 0x00, tmp);
	}
	while(!(tmp[1] & 0x02)); */
#endif


	unsigned char tmp[2];
	adress >>= 1;	//divide by two: convert byte adress to word adress

#ifdef DEBUG_VERBOSE
	UARTWrite("Write Flash page ");
	UARTWrite("Address");
	SendHex((char) ((adress>>8) & 0xFF) );
	SendHex((char) (adress & 0xFF));
	UARTWrite("\n\r");
#endif

	avr_prog_cmd();					//Prog Enable
	avr_sequence(0x23, 0x10, tmp);

	for(int i = 0; i < byteCount; i += 2, adress++)
	{
#ifdef DEBUG_VERBOSE
//		UARTWrite("Adress: ");
		SendHex((adress >> 8) & 0xFF);
		SendHex(adress & 0xFF);
#endif
		avr_sequence(0x07, (adress >> 8) & 0xFF, tmp);
		avr_sequence(0x03, adress & 0xFF, tmp);

		avr_sequence(0x13, data[i], tmp);
		avr_sequence(0x17, data[i + 1], tmp);
#ifdef DEBUG_VERBOSE
		UARTWrite(":");
		SendHex(data[i]);
		SendHex(data[i + 1]);
		UARTWrite("\n\r");
#endif
		avr_sequence(0x37, 0x00, tmp);
		avr_sequence(0x77, 0x00, tmp);
		avr_sequence(0x37, 0x00, tmp);
	}

#ifdef DEBUG_VERBOSE
	UARTWrite("Flash succesfully\n\r");
#endif
	avr_sequence(0x37, 0x00, tmp);
	avr_sequence(0x35, 0x00, tmp);
	avr_sequence(0x37, 0x00, tmp);
	avr_sequence(0x37, 0x00, tmp);

	do
	{
		avr_sequence(0x37, 0x00, tmp);
	}
	while(!(tmp[1] & 0x02));
}

void wr_eeprom_page(unsigned char byteCount, unsigned int adress, unsigned char *data)
{
	unsigned char tmp[2];
	avr_prog_cmd();
	avr_sequence(0x23, 0x11, tmp);

	for(unsigned char i = 0; i < byteCount; i++, adress++)
	{
		avr_sequence(0x07, (adress >> 8) & 0xFF, tmp);
		avr_sequence(0x03, adress & 0xFF, tmp);
		avr_sequence(0x13, data[i], tmp);

		avr_sequence(0x37, 0x00, tmp);
		avr_sequence(0x77, 0x00, tmp);
		avr_sequence(0x37, 0x00, tmp);
	}

	avr_sequence(0x33, 0x00, tmp);
	avr_sequence(0x31, 0x00, tmp);
	avr_sequence(0x33, 0x00, tmp);
	avr_sequence(0x33, 0x00, tmp);

	do
	{
		avr_sequence(0x33, 0x00, tmp);
	}
	while(!(tmp[1] & 0x02));
}

void rd_eeprom_page(uint16_t byteCount, unsigned int adress, unsigned char *data)
{
	unsigned char tmp[2];
	avr_prog_cmd();
	avr_sequence(0x23, 0x03, tmp);
#ifdef DEBUG_VERBOSE
	UARTWrite("EEPROM_Page:");
	SendHex((char)(adress>>8));
	SendHex((char)(adress));
	UARTWrite("\r\n");
#endif

	for(uint16_t i = 0; i < byteCount; i++, adress++)
	{
		avr_sequence(0x07, (adress >> 8) & 0xFF, tmp);
		avr_sequence(0x03, adress & 0xFF, tmp);

		avr_sequence(0x33, adress & 0xFF, tmp);
		avr_sequence(0x32, 0x00, tmp);
		avr_sequence(0x33, 0x00, &data[i]);
#ifdef DEBUG_VERBOSE
		UARTWrite(" ");
		SendHex(data[i]);
#endif
	}
#ifdef DEBUG_VERBOSE
	UARTWrite("\r\n");
#endif
}

void rd_flash_page(unsigned int byteCount, unsigned long adress, unsigned char *data)
{
	unsigned char tmp[2];
	adress >>= 1;

	avr_prog_cmd();
	avr_sequence(0x23, 0x02, tmp);

	for(int i = 0; i < byteCount; i += 2, adress++)
	{
#ifdef DEBUG_VERBOSE
		UARTWrite("FR:");
		SendHex((adress >> 8));
		SendHex((char)adress);
		UARTWrite("\r\n");
#endif
		avr_sequence(0x07, (adress >> 8) & 0xFF, tmp);
		avr_sequence(0x03, adress & 0xFF, tmp);
		avr_sequence(0x32, 0x00, tmp);
		avr_sequence(0x36, 0x00, &data[i]);
		avr_sequence(0x37, 0x00, &data[i + 1]);
	}
}

