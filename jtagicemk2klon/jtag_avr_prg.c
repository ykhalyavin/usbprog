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


char rd_lock_avr ()
{
	char jtagbuf[2];
	
	avr_sequence(0x23,0x04,jtagbuf);	//enter fuse lock bits
	avr_sequence(0x36,0x00,jtagbuf);	// select lfuse
	avr_sequence(0x37,0x00,jtagbuf);	// read lock

	return jtagbuf[0];
}

char rd_efuse_avr ()
{
	char jtagbuf[2];
	
	avr_sequence(0x23,0x04,jtagbuf);	//enter fuse lock bits
	avr_sequence(0x32,0x00,jtagbuf);	// select lfuse
	avr_sequence(0x33,0x00,jtagbuf); // read lfuse

	return jtagbuf[0];
}

char rd_lfuse_avr ()
{
	char jtagbuf[2];
	
	avr_sequence(0x23,0x04,jtagbuf);	//enter fuse lock bits
	avr_sequence(0x32,0x00,jtagbuf);	// select lfuse
	avr_sequence(0x33,0x00,jtagbuf); // read lfuse

	return jtagbuf[0];
}


char rd_hfuse_avr ()
{
	char jtagbuf[2];
	
	avr_sequence(0x23,0x04,jtagbuf);	//enter fuse lock bits
	avr_sequence(0x3E,0x00,jtagbuf);	// select hfuse
	avr_sequence(0x3F,0x00,jtagbuf); // read hfuse

	return jtagbuf[0];
}

int rd_fuse_avr (char *buf, int withextend)
{
	return 1;
}


int rd_signature_avr (char *signature)
{
	char tmp[2];
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

	return 1;
}

void wr_hfuse_avr(char hfuse)
{
	char tmp[2];
	avr_sequence(0x23,0x40,tmp);
	avr_sequence(0x13,hfuse,tmp);
	avr_sequence(0x37,0x00,tmp);
	avr_sequence(0x35,0x00,tmp);
	avr_sequence(0x37,0x00,tmp);
	avr_sequence(0x37,0x00,tmp);
	avr_sequence(0x37,0x00,tmp);
}

void wr_lfuse_avr(char lfuse)
{
	char tmp[2];
	avr_sequence(0x23,0x40,tmp);
	avr_sequence(0x13,lfuse,tmp);
	avr_sequence(0x33,0x00,tmp);
	avr_sequence(0x31,0x00,tmp);
	avr_sequence(0x33,0x00,tmp);
	avr_sequence(0x33,0x00,tmp);
	avr_sequence(0x33,0x00,tmp);
}

void wr_efuse_avr(char efuse)
{
	char tmp[2];
	avr_sequence(0x23,0x40,tmp);
	avr_sequence(0x13,efuse,tmp);
	avr_sequence(0x3B,0x00,tmp);
	avr_sequence(0x39,0x00,tmp);
	avr_sequence(0x3B,0x00,tmp);
	avr_sequence(0x3B,0x00,tmp);
	avr_sequence(0x37,0x00,tmp);
}

void wr_lock_avr(char lock)
{
	char tmp[2];
	avr_sequence(0x20,0x40,tmp);
	avr_sequence(0x13,lock | 0xC0 ,tmp);
	avr_sequence(0x33,0x00,tmp);
	avr_sequence(0x31,0x00,tmp);
	avr_sequence(0x33,0x00,tmp);
	avr_sequence(0x33,0x00,tmp);
	avr_sequence(0x33,0x00,tmp);
}

// (2) data[tdi] = [0x80, 0x23], [0x80, 0x31], [0x80, 0x33], [0x80, 0x33], [0x80, 0x33] (chip erase sequence)

void chip_erase(void)
{
	char tmp[2];
	
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



void wr_flash_page(unsigned int byteCount, unsigned long adress, char *data)
{
	char tmp[2];
	
	int adr = adress >> 1;
	

	UARTWrite("Write Flash Page\n");
	UARTWrite("Adress: ");
	SendHex((adr >> 8) & 0xFF);
	SendHex(adr & 0xFF);
	UARTWrite("\n");

	avr_prog_cmd();					//Prog Enable
	avr_sequence(0x23, 0x10, tmp);
	avr_sequence(0x07, (adr >> 8) & 0xFF, tmp);
	avr_sequence(0x03, adr & 0xFF , tmp);
	avr_jtag_instr(AVR_PRG_PAGE_LOAD, 0);	//inst = PAGE_LOAD 0x06
	
		
	for(int i = 0; i < byteCount; i++)
	{
		SendHex(data[i]);
		UARTWrite("\n");
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
	while(!(tmp[1] & 0x02)); 
	
	/*char tmp[2];
	static unsigned int addr = 0;
	
	UARTWrite("Write Flash page\n");
	
	avr_prog_cmd();					//Prog Enable
	avr_sequence(0x23, 0x10, tmp);
	
	for(int i = 0; i < byteCount; i += 2, addr++)
	{
		UARTWrite("Adress: ");
		SendHex((addr >> 8) & 0xFF);
		SendHex(addr & 0xFF);
		
		avr_sequence(0x07, (addr >> 8) & 0xFF, tmp);
		avr_sequence(0x03, addr & 0xFF, tmp);
		
		avr_sequence(0x13, data[i], tmp);
		avr_sequence(0x17, data[i + 1], tmp);
		
		UARTWrite(" Data: ");
		SendHex(data[i]);
		SendHex(data[i + 1]);	
		UARTWrite("\n");

		avr_sequence(0x37, 0x00, tmp);
		avr_sequence(0x77, 0x00, tmp);
		avr_sequence(0x37, 0x00, tmp);
	}
		
	avr_sequence(0x37, 0x00, tmp);
	avr_sequence(0x35, 0x00, tmp);
	avr_sequence(0x37, 0x00, tmp);
	avr_sequence(0x37, 0x00, tmp);
	
	do
	{
		avr_sequence(0x37, 0x00, tmp);
	}
	while(!(tmp[1] & 0x02)); */	
}
	
	


