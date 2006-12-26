/*
 * avrdude - A Downloader/Uploader for AVR device programmers
 * Copyright (C) 2006/2007 Benedikt Sauter
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

#ifndef usbprog_h
#define usbprog_h

#include "avrpart.h"

#define	USBPROG_VID  	0x1781 
#define	USBPROG_PID 	0x0c62 

#define USBPROG_BULK_EP_WRITE 	0x02
#define USBPROG_BULK_EP_READ  	0x83
#define USBPROG_MAX_XFER 		64

void usbprog_initpgm (PROGRAMMER * pgm);

#include "avr.h"
#include "pgm.h"

extern static int stk500v2_initialize(PROGRAMMER * pgm, AVRPART * p);
extern static void stk500v2_display(PROGRAMMER * pgm, char * p);
extern static void stk500v2_enable(PROGRAMMER * pgm);
extern static void stk500v2_disable(PROGRAMMER * pgm);
extern static int stk500v2_program_enable(PROGRAMMER * pgm, AVRPART * p);
extern static int stk500v2_chip_erase(PROGRAMMER * pgm, AVRPART * p);
extern static int stk500v2_cmd(PROGRAMMER * pgm, unsigned char cmd[4],
                        unsigned char res[4]);
extern static void stk500v2_close(PROGRAMMER * pgm);
extern static int stk500v2_paged_write(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m,
                              int page_size, int n_bytes);
extern static int stk500v2_paged_load(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m,
                             int page_size, int n_bytes);
extern static void stk500v2_print_parms1(PROGRAMMER * pgm, char * p);
extern static int stk500v2_set_sck_period_mk2(PROGRAMMER * pgm, double v);
extern static int stk500v2_perform_osccal(PROGRAMMER * pgm);



#endif


