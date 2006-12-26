/*
 * avrdude - A Downloader/Uploader for AVR device programmers
 * Copyright (C) 2005 Erik Walthinsen
 * Copyright (C) 2002-2004 Brian S. Dean <bsd@bsdhome.com>
 * Copyright (C) 2006 David Moore
 * Copyright (C) 2006 Joerg Wunsch <j@uriah.heep.sax.de>
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

/* $Id: stk500v2.c,v 1.34 2006/12/20 23:43:34 joerg_wunsch Exp $ */
/* Based on Id: stk500.c,v 1.46 2004/12/22 01:52:45 bdean Exp */

/*
 * avrdude interface for Atmel STK500V2 programmer
 *
 * As the AVRISP mkII device is basically an STK500v2 one that can
 * only talk across USB, and that misses any kind of framing protocol,
 * this is handled here as well.
 *
 * Note: most commands use the "universal command" feature of the
 * programmer in a "pass through" mode, exceptions are "program
 * enable", "paged read", and "paged write".
 *
 */

#include "avr.h"
#include "pgm.h"

enum hvmode
{
  PPMODE, HVSPMODE
};

 int stk500v2_getparm(PROGRAMMER * pgm, unsigned char parm, unsigned char * value);
 int stk500v2_setparm(PROGRAMMER * pgm, unsigned char parm, unsigned char value);
 void stk500v2_print_parms1(PROGRAMMER * pgm, char * p);
 int stk500v2_is_page_empty(unsigned int address, int page_size,
                                  const unsigned char *buf);

 unsigned int stk500v2_mode_for_pagesize(unsigned int pagesize);

 int stk500v2_set_sck_period_mk2(PROGRAMMER * pgm, double v);

 unsigned short
b2_to_u16(unsigned char *b);

 int stk500v2_send_mk2(PROGRAMMER * pgm, unsigned char * data, size_t len);
 unsigned short get_jtagisp_return_size(unsigned char cmd);
 int stk500v2_jtagmkII_send(PROGRAMMER * pgm, unsigned char * data, size_t len);

 int stk500v2_send(PROGRAMMER * pgm, unsigned char * data, size_t len);
 int stk500v2_drain(PROGRAMMER * pgm, int display);
 int stk500v2_recv_mk2(PROGRAMMER * pgm, unsigned char msg[],
			     size_t maxsize);
 int stk500v2_jtagmkII_recv(PROGRAMMER * pgm, unsigned char msg[],
                                  size_t maxsize);
 int stk500v2_recv(PROGRAMMER * pgm, unsigned char msg[], size_t maxsize);
 int stk500v2_getsync(PROGRAMMER * pgm);
 int stk500v2_command(PROGRAMMER * pgm, unsigned char * buf,
                            size_t len, size_t maxlen);
 int stk500v2_cmd(PROGRAMMER * pgm, unsigned char cmd[4],
                        unsigned char res[4]);
 int stk500v2_chip_erase(PROGRAMMER * pgm, AVRPART * p);
 int stk500hv_chip_erase(PROGRAMMER * pgm, AVRPART * p, enum hvmode mode);
 int stk500pp_chip_erase(PROGRAMMER * pgm, AVRPART * p);
 int stk500hvsp_chip_erase(PROGRAMMER * pgm, AVRPART * p);
 int stk500v2_program_enable(PROGRAMMER * pgm, AVRPART * p);
 int stk500pp_program_enable(PROGRAMMER * pgm, AVRPART * p);
 int stk500hvsp_program_enable(PROGRAMMER * pgm, AVRPART * p);
 int stk500v2_initialize(PROGRAMMER * pgm, AVRPART * p);

 int stk500hv_initialize(PROGRAMMER * pgm, AVRPART * p, enum hvmode mode);
 int stk500pp_initialize(PROGRAMMER * pgm, AVRPART * p);
 int stk500hvsp_initialize(PROGRAMMER * pgm, AVRPART * p);
 void stk500v2_disable(PROGRAMMER * pgm);
 void stk500hv_disable(PROGRAMMER * pgm, enum hvmode mode);
 void stk500pp_disable(PROGRAMMER * pgm);
 void stk500hvsp_disable(PROGRAMMER * pgm);
 void stk500v2_enable(PROGRAMMER * pgm);
 int stk500v2_open(PROGRAMMER * pgm, char * port);
 void stk500v2_close(PROGRAMMER * pgm);
 int stk500v2_loadaddr(PROGRAMMER * pgm, unsigned int addr);
 int stk500hv_read_byte(PROGRAMMER * pgm, AVRPART * p, AVRMEM * mem,
			      unsigned long addr, unsigned char * value,
			      enum hvmode mode);
 int stk500pp_read_byte(PROGRAMMER * pgm, AVRPART * p, AVRMEM * mem,
			      unsigned long addr, unsigned char * value);
 int stk500hvsp_read_byte(PROGRAMMER * pgm, AVRPART * p, AVRMEM * mem,
				unsigned long addr, unsigned char * value);
 int stk500hv_write_byte(PROGRAMMER * pgm, AVRPART * p, AVRMEM * mem,
			       unsigned long addr, unsigned char data,
			       enum hvmode mode);
 int stk500pp_write_byte(PROGRAMMER * pgm, AVRPART * p, AVRMEM * mem,
			       unsigned long addr, unsigned char data);
 int stk500hvsp_write_byte(PROGRAMMER * pgm, AVRPART * p, AVRMEM * mem,
			       unsigned long addr, unsigned char data);
 int stk500v2_paged_write(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m, 
                              int page_size, int n_bytes);
 int stk500hv_paged_write(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m,
				  int page_size, int n_bytes,
				  enum hvmode mode);
 int stk500pp_paged_write(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m,
				int page_size, int n_bytes);
 int stk500hvsp_paged_write(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m,
				  int page_size, int n_bytes);
 int stk500v2_is_page_empty(unsigned int address, int page_size,
                                const unsigned char *buf);
 int stk500v2_paged_load(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m,
                             int page_size, int n_bytes);
 int stk500hv_paged_load(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m,
			       int page_size, int n_bytes,
			       enum hvmode mode);
 int stk500pp_paged_load(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m,
			       int page_size, int n_bytes);
 int stk500hvsp_paged_load(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m,
				 int page_size, int n_bytes);


 int stk500v2_set_vtarget(PROGRAMMER * pgm, double v);
 int stk500v2_set_varef(PROGRAMMER * pgm, double v);
 int stk500v2_set_fosc(PROGRAMMER * pgm, double v);

 int stk500v2_set_sck_period_mk2(PROGRAMMER * pgm, double v);
 unsigned int stk500v2_mode_for_pagesize(unsigned int pagesize);
 int stk500v2_set_sck_period(PROGRAMMER * pgm, double v);


 int stk500v2_getparm(PROGRAMMER * pgm, unsigned char parm, unsigned char * value);

 int stk500v2_setparm_real(PROGRAMMER * pgm, unsigned char parm, unsigned char value);
 int stk500v2_setparm(PROGRAMMER * pgm, unsigned char parm, unsigned char value);
 void stk500v2_display(PROGRAMMER * pgm, char * p);
 void stk500v2_print_parms1(PROGRAMMER * pgm, char * p);
 void stk500v2_print_parms(PROGRAMMER * pgm);

 int stk500v2_perform_osccal(PROGRAMMER * pgm);
 int stk500v2_jtagmkII_open(PROGRAMMER * pgm, char * port);
 int stk500v2_dragon_isp_open(PROGRAMMER * pgm, char * port);
 int stk500v2_dragon_hv_open(PROGRAMMER * pgm, char * port);
