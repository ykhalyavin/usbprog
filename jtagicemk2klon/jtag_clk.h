/*-------------------------------------------------------------------------
 * JTAG_CLK.H
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

#ifndef JTAG_CLK_H
#define JTAG_CLK_H

extern unsigned char feed_jtag_data (unsigned char *tdo, unsigned char *tdi,
                                     short num_bits, unsigned char state,
                                     unsigned char delay);
extern unsigned char reset_jtag (char delay);
extern unsigned char feed_jtag_instr (unsigned char jtag_instr,
                                      unsigned char delay);
extern unsigned char get_jtag_idcode (unsigned char instr_id,
                                      unsigned long *jtag_id,
                                      unsigned char delay);

#endif
