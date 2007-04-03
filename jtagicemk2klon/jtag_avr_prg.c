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


char rd_lock_avr ()
{
	char jtagbuf[2];
	
	avr_sequence(0x23,0x04,&jtagbuf);	//enter fuse lock bits
	avr_sequence(0x36,0x00,&jtagbuf);	// select lfuse
	avr_sequence(0x3,0x00,&jtagbuf); // read lfuse

	return jtagbuf[0];
}

char rd_efuse_avr ()
{
	char jtagbuf[2];
	
	//avr_sequence(0x23,0x04,&jtagbuf);	//enter fuse lock bits
	//avr_sequence(0x32,0x00,&jtagbuf);	// select lfuse
	//avr_sequence(0x33,0x00,&jtagbuf); // read lfuse
	jtagbuf[0]=0x00;

	return jtagbuf[0];
}

char rd_lfuse_avr ()
{
	char jtagbuf[2];
	
	avr_sequence(0x23,0x04,&jtagbuf);	//enter fuse lock bits
	avr_sequence(0x32,0x00,&jtagbuf);	// select lfuse
	avr_sequence(0x33,0x00,&jtagbuf); // read lfuse

	return jtagbuf[0];
}


char rd_hfuse_avr ()
{
	char jtagbuf[2];
	
	avr_sequence(0x23,0x04,&jtagbuf);	//enter fuse lock bits
	avr_sequence(0x3E,0x00,&jtagbuf);	// select hfuse
	avr_sequence(0x3F,0x00,&jtagbuf); // read hfuse

	return jtagbuf[0];
}

int rd_fuse_avr (char *buf, int withextend)
{
	char jtagbuf[2];

	avr_sequence(0x04,0x23,&jtagbuf);
	avr_sequence(0x00,0x32,&jtagbuf);
	avr_sequence(0x00,0x33,&jtagbuf);

	return 1;
}


int rd_signature_avr (char *signature)
{

/*
	avr_sequence(0x23,0x08,&jtagbuf);	//enter
	avr_sequence(0x03,0x01,&jtagbuf);	//load address
	avr_sequence(0x32,0x00,&jtagbuf);	//read
	avr_sequence(0x33,0x00,&jtagbuf);	//restult
*/
	return 1;
}


