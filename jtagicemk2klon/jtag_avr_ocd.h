/*-------------------------------------------------------------------------
 * JTAG_AVR_OCD.H
 * Copyright (C) 2003 Armand ten Doesschate <a.doesschate@hccnet.nl>
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

#ifndef JTAG_AVR_OCD_H
#define JTAG_AVR_OCD_H

#include "tgt_info.h"

extern unsigned char rd_flash_ocd_avr (unsigned long addr, unsigned char *buf,
                                       short size, unsigned char delay);
extern unsigned char rd_sram_ocd_avr (unsigned short addr, unsigned char *buf,
                                      short size, unsigned char delay);
extern unsigned char rd_io_ocd_avr (unsigned char addr, unsigned char *buf,
                                    short size, unsigned char delay);
extern unsigned char rd_e2_ocd_avr (unsigned short addr, unsigned char *buf,
                                    short size, unsigned char delay);
extern unsigned char wr_avr_flash_ocd (unsigned long addr, unsigned char *buf,
                                       short size, unsigned char delay);
extern unsigned char wr_sram_ocd_avr (unsigned short addr, unsigned char *buf,
                                      short size, unsigned char delay);
extern unsigned char wr_io_ocd_avr (unsigned char addr, unsigned char *buf,
                                    short size, unsigned char delay);
extern void step_avr (unsigned char delay);
extern unsigned char force_avr_stop (unsigned char delay);
extern unsigned char run_avr (unsigned char mode, unsigned char go_flg,
                              unsigned long addr, unsigned char delay);
extern unsigned char init_avr_jtag (struct avr_reg *reg, unsigned char delay);

extern void init_all_regs_avr (void);
extern void get_all_regs_avr (unsigned char delay);
extern void set_all_regs_avr (unsigned char *buf, unsigned char delay);


#endif
