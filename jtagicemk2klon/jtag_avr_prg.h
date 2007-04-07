/*-------------------------------------------------------------------------
 * JTAG_AVR_PRG.H
 * Copyright (C) 2007 Benedikt sauter <sauter@sistecs.de>
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

char rd_efuse_avr ();
char rd_hfuse_avr ();
char rd_lfuse_avr ();
char rd_lock_avr ();
int rd_fuse_avr (char *buf, int withextend);
int rd_signature_avr (char *signature);

void avr_sequence(char tdi2, char tdi1, char * tdo);


#endif
