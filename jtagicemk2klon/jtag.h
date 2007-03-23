/* jtag.h
* Copyright (C) 2007  Benedikt Sauter, sauter@ixbat.de
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
#ifndef _JTAG_H_
#define _JTAG_H_



// setup connection
void jtag_init();

// force jtag tap into reset state
void jtag_reset();

// goto a state
void jtag_goto_state(uint8_t state);

// write to target tdi
uint8_t jtag_read(uint8 numbers, unsigned char * buf);

// read from target tdo
uint8_t jtag_write(uint8_t numbers, unsigned char * buf);



#endif /* _JTAG_H_ */
