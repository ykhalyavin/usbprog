/*-------------------------------------------------------------------------
 * JTAG_AVR_PRG.H
 * Copyright (C) 2007 Benedikt sauter <sauter@sistecs.de>
 *				 2007 Robert Schilling robert.schilling@gmx.at
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

#ifndef JTAG_AVR_PRG_H
#define JTAG_AVR_PRG_H

unsigned char rd_efuse_avr (void);
unsigned char rd_hfuse_avr (void);
unsigned char rd_lfuse_avr (void);
unsigned char rd_lock_avr (void);
void rd_fuse_avr (unsigned char *buf, int withextend);
void rd_signature_avr (unsigned char *signature);
void wr_lfuse_avr(unsigned char lfuse);
void wr_hfuse_avr(unsigned char hfuse);
void wr_efuse_avr(unsigned char efuse);
void wr_lock_avr(unsigned char lock);
void chip_erase(void);
void wr_flash_page(unsigned int byteCount, unsigned long adress, unsigned char *data);
void rd_flash_page(unsigned int byteCount, unsigned long adress, unsigned char *data);
unsigned char rd_cal_byte(unsigned char adress);
void rd_eeprom_page(unsigned int byteCount, unsigned int adress, unsigned char *data);
void wr_eeprom_page(unsigned char byteCount, unsigned int adress, unsigned char *data);
void avr_sequence(char tdi2, char tdi1, unsigned char * tdo);


#endif
