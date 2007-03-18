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
 * $Id: ParallelPort.h,v 1.2 2005/11/25 22:16:34 uid20104 Exp $
 */
#ifndef __ParallelPort_h
#define __ParallelPort_h

#include "IO.h"


/** \file */


/** This class contains methods and data elements that are common to all IO
 * implementations attached to a parallel port.
 */
class ParallelPort : public IO {
public:
	/** This constructor is called right before a parallel port IO class is
	 * created. It the responsibility of this function to read in the
	 * configuration variables specific to parallel port programmers.
	 * \param port The parallel port number that is being opened.
	 */
	ParallelPort(int port);

	/** Frees all memory and resources associated with this object */
	virtual ~ParallelPort();

	virtual void set_clk(bool state);
	virtual bool get_clk(void);
	virtual void set_data(bool state);
	virtual bool get_data(void);
	virtual bool read_data(void);
	virtual void set_vpp(bool state);
	virtual bool get_vpp(void);
	virtual void set_pwr(bool state);
	virtual bool get_pwr(void);
	virtual void set_lvp(bool state);
	virtual bool get_lvp(void);

protected:
	/** Gets the state of a pin on the parallel port
	 * \param name The name of the pin.
	 * \param reg The parallel port register that controls the pin.
	 * \param bit The bit number within the control register.
	 * \param invert Indicates whether the read value should be inverted.
	 * \return The status of the pin, modified by the invert flag.
	 */
	virtual bool get_pin_state(const char *name, int reg, int bit, bool invert) = 0;

	/** Sets the state of a pin on the parallel port
	 * \param name The name of the pin.
	 * \param reg The parallel port register that controls the pin.
	 * \param bit The bit number within the control register.
	 * \param invert Indicates whether the value to write should be inverted.
	 * \param state The state to set on the pin.
	 */
	virtual void set_pin_state(const char *name, int reg, int bit, bool invert, bool state) = 0;

	/** The parallel port number this object is using */
	int port;

	/** Parallel port pin data for the read data signal */
	short rdatareg, rdatabit, rdatainvert;

	/** Parallel port pin data for the secondary read data signal */
	short rdata2reg, rdata2bit, rdata2invert;

	/** Parallel port pin data for the write data signal */
	short wdatareg, wdatabit, wdatainvert;

	/** Parallel port pin data for the clock signal */
	short clkreg, clkbit, clkinvert;

	/** Parallel port pin data for the Vpp enable signal */
	short vppreg, vppbit, vppinvert;

	/** Parallel port pin data for the Vcc enable signal */
	short pwrreg, pwrbit, pwrinvert;

	/** Parallel port pin data for pin to control the LVP signal */
	short lvpreg, lvpbit, lvpinvert;
};


#endif
