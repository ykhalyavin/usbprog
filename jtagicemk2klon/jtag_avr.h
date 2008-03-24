
/*-------------------------------------------------------------------------
 * JTAG_AVR.H
 * Copyright (C) 2007 Benedikt Sauter  <sauter@ixbat.de>
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

#ifndef JTAG_AVR_H
#define JTAG_AVR_H

#include <avr/pgmspace.h>


void idcode(unsigned char *buf);
void bypass(void);
void avr_reset(uint8_t set);
void avr_prog_enable(void);
void avr_prog_disable(void);
void avr_prog_cmd(void);
int avr_jtag_instr(unsigned char instr, int delay);


void avr_sequence(char tdi2, char tdi1, unsigned char * tdo);

/*
 * The debugger is communicating with the CPU core through the use of
 * the ocdr I/O register when the program is running. When the
 * CPU is halted/reset (i.e. programming flash) then other means
 * is used
 *
 * Flash page size : total * bits per word == total * 16
 * Eeprom page size : total * bits per byte == total * 8
 *
 * The jtag id determines the AVR drivate
 */
struct avr_type {
    unsigned long jtag_id;
    unsigned char ocdr_adr;
    short         eeprom_page_size;
    short         flash_page_size;
    PGM_P         rd_all_fuses_seq;
    PGM_P         *wr_fuse_seq;
    PGM_P         *rd_fuse_seq;
};

struct avr_reg {
    unsigned char r0;
    unsigned char r1;
    unsigned char r2;
    unsigned char r3;
    unsigned char r4;
    unsigned char r5;
    unsigned char r6;
    unsigned char r7;
    unsigned char r8;
    unsigned char r9;
    unsigned char r10;
    unsigned char r11;
    unsigned char r12;
    unsigned char r13;
    unsigned char r14;
    unsigned char r15;
    unsigned char r16;
    unsigned char r17;
    unsigned char r18;
    unsigned char r19;
    unsigned char r20;
    unsigned char r21;
    unsigned char r22;
    unsigned char r23;
    unsigned char r24;
    unsigned char r25;
    unsigned char r26;
    unsigned char r27;
    unsigned char r28;
    unsigned char r29;
    unsigned char r30;
    unsigned char r31;
    unsigned char sreg;
    unsigned short sp;
    unsigned short pc;
};

union tgt_reg {
   struct avr_reg  avr;
};

union tgt_reg reg;

#endif
