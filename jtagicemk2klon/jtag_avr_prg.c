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
#include "jtag_avr_defines.h"
#include "jtag.h"

/*----------------------------------------------------------------------*
 * enable programming of flash of this derivate. We must assure that    *
 * the AVR derivate has been reset properly                             *
 *----------------------------------------------------------------------*/
int enable_prg_avr ()
{
	return 1;
}

/*----------------------------------------------------------------------*
 * disable programming of flash. In all cases it is sufficient to send  *
 * an all "0" to the programming command of all devices                 *
 *----------------------------------------------------------------------*/
int disable_prg_avr (unsigned char delay)
{
	return 1;
}

/*----------------------------------------------------------------------*
 * read single fuse : fuse can be an extended, lock, high or low byte   *
 *----------------------------------------------------------------------*/
int rd_fuse_avr (char *fuse)
{
	return 1;
}
