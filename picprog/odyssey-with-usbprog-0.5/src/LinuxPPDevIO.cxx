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
 * $Id: LinuxPPDevIO.cxx,v 1.2 2005/11/25 22:16:34 uid20104 Exp $
 */
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

#include <linux/parport.h>
#include <linux/ppdev.h>
#include "LinuxPPDevIO.h"
#include "Util.h"


LinuxPPDevIO::LinuxPPDevIO(int port) : ParallelPort(port)  {
	struct stat fdata;
	char devname[20];
	int arg;

	if(stat("/dev/parports", &fdata) == 0) {
		/* We're using DevFS */
		sprintf(devname, "/dev/parports/%d", port);
	} else {
		sprintf(devname, "/dev/parport%d", port);
	}
	this->fd = open(devname, O_RDWR);
	if(this->fd < 0)
		throw runtime_error(strerror(errno));

	try {
		if(ioctl(this->fd, PPEXCL) < 0)
			throw errno;
		if(ioctl(this->fd, PPCLAIM) < 0)
			throw errno;
		arg = IEEE1284_MODE_BYTE;
		if(ioctl(this->fd, PPSETMODE, &arg) < 0)
			throw errno;
	} catch(int err) {
		close(this->fd);
		throw runtime_error(strerror(err));
	}
    /* TODO generalize this hack */
    set_pin_state("vpp1", 0, 3, true, false);
    set_pin_state("vpp3", 0, 5, true, false);
}


LinuxPPDevIO::~LinuxPPDevIO() {
	/* Turn things off */
	this->off();

	int arg = IEEE1284_MODE_COMPAT;
	ioctl(this->fd, PPSETMODE, &arg);
	ioctl(this->fd, PPRELEASE);		/* Ignore errors */
	close(this->fd);
}


bool LinuxPPDevIO::get_pin_state(const char *name, int reg, int bit,
	bool invert)
{
	int parm1, arg;

	switch(reg) {
	case 0:
		parm1 = PPRDATA;
		break;
	case 1:
		parm1 = PPRSTATUS;
		break;
	case 2:
		parm1 = PPRCONTROL;
		break;
	default:
		THROW_ERROR(runtime_error, "%spin: unknown register %d", name, reg);
	}

	if(ioctl(this->fd, parm1, &arg) < 0) {
		THROW_ERROR(runtime_error, "%spin: read", name);
	}
	arg = (arg >> bit) & 0x01;
	if(invert)
		arg ^= 0x01;
	return (bool)arg;
}


void LinuxPPDevIO::set_pin_state(const char *name, int reg, int bit,
	bool invert, bool state)
{
	int parm1, parm2, arg;

	switch(reg) {
	case 0:
		parm1 = PPRDATA;
		parm2 = PPWDATA;
		break;
	case 2:
		parm1 = PPRCONTROL;
		parm2 = PPWCONTROL;
		break;
	default:
		THROW_ERROR(runtime_error, "%spin: unknown register %d", name, reg);
	}

	if(ioctl(this->fd, parm1, &arg) < 0) {
		THROW_ERROR(runtime_error, "%spin: read", name);
	}
	if(invert)
		state ^= 0x01;
	if(state)
		arg |= (1 << bit);
	else
		arg &= ~(1 << bit);

	if(ioctl(this->fd, parm2, &arg) < 0) {
		THROW_ERROR(runtime_error, "%spin: write", name);
	}
}
