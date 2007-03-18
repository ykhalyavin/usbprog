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
 * $Id: LinuxPPDevIO.h,v 1.2 2005/11/25 22:16:34 uid20104 Exp $
 */
#ifndef __LinuxPPDevIO_h
#define __LinuxPPDevIO_h

#include "ParallelPort.h"

/** \file */


/**
 * An implementation of the IO interface which uses the Linux 2.4 ppdev
 * driver.
 */
class LinuxPPDevIO : public ParallelPort {
public:
	/**
	 * This constructor will open and initialize the parallel port ppdev
	 * device. Both DevFS and pre-DevFS setups are supported.
	 *
	 * \param port The number of the parallel port to use. 
	 * \throws runtime_error Contains a textual description of the error.
	 */
	LinuxPPDevIO(int port);

	/** Destructor */
	~LinuxPPDevIO();

	bool get_pin_state(const char *name, int reg, int bit, bool invert);
	void set_pin_state(const char *name, int reg, int bit, bool invert, bool state);

private:
	int fd;
};


#endif
