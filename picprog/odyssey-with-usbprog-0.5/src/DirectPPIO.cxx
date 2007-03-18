/* Copyright (C) 2002-2004  Mark Andrew Aikens <marka@desert.cx>
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
 * $Id: DirectPPIO.cxx,v 1.2 2005/11/25 22:16:34 uid20104 Exp $
 */
using namespace std;

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/io.h>
#include <stdexcept>
#include "DirectPPIO.h"
#include "Util.h"


#define SET_BIT_COMMON(prefix) \
	if(this->prefix##invert) state ^= 0x01; \
	data = inb(this->ioport + this->prefix##reg); \
	if(state) \
		data |= (1 << this->prefix##bit); \
	else \
		data &= ~(1 << this->prefix##bit); \
	outb(data, this->ioport + this->prefix##reg);


static int ioports[3] = { 0x378, 0x278, 0x3bc };


DirectPPIO::DirectPPIO(int port) : ParallelPort(port) {
	if((port > 2) || (port < 0))
		throw runtime_error("Invalid DirectPP port number");

	/* Turn port access on */
	this->ioport = ioports[port];
	if(ioperm(this->ioport, 3, 1) < 0)
		THROW_ERROR(runtime_error, "ioperm: %s", strerror(errno));

	/* Set UID back to user if running setuid */
	if(setuid(getuid()) < 0)
		fprintf(stderr, "Warning: couldn't drop root privledges.\n");
}


DirectPPIO::~DirectPPIO() {
	/* Turn everything off */
	this->off();

	/* Turn port access off */
	ioperm(this->ioport, 3, 0);
}


bool DirectPPIO::get_pin_state(const char *name, int reg, int bit,
	bool invert)
{
	unsigned int data;

	data = inb(this->ioport + reg);
	data = (data >> bit) & 0x01;
	if(invert)
		data ^= 0x01;
	return (bool)data;
}


void DirectPPIO::set_pin_state(const char *name, int reg, int bit, bool invert,
	bool state)
{
	unsigned int data;

	if(invert)
		state ^= 0x01;
	data = inb(this->ioport + reg);
	if(state)
		data |= (1 << bit);
	else
		data &= ~(1 << bit);
	outb(data, this->ioport + reg);
}
