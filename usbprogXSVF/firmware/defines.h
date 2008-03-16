/*
 * Defines for XSVF Player
 * Copyright (C) 2007 Sven Luetkemeier 
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

#ifndef _XSVFPLAYER_DEFINES_H_
#define _XSVFPLAYER_DEFINES_H_

/* USB IDs */
#define VID 0x1781
#define PID 0x0c62

/* in non-XSVF packets the most significant bit of first buffer byte is set */
/* initialize XSVF player and attached JTAG device(s) */
#define XSVF_INIT		0x81
#define XSVF_PRGEND		0x82

/* non-XSVF return codes */
#define SUCCESS			0x80
#define UNKNOWN_COMMAND		0x81

#endif
