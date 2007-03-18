/* Copyright (C) 2002-2003  Mark Andrew Aikens <marka@desert.cx>
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
 * $Id: Device.h,v 1.3 2006/01/02 17:43:24 gaufille Exp $
 */
#ifndef __Device_h
#define __Device_h

#include <vector>
#include <string>
#include "DataBuffer.h"
#include "IO.h"

/** \file */


/** Shortcut for a vector of pairs of integers. This is used mainly for the
 * device memory map. */
typedef vector<pair <int, int> > IntPairVector;


/** A base class representing a memory device which can be manipulated. This
 * class contains the basic high-level operators erase, program, and read.
 */
class Device {
public:
	/** Gets a list of the available device types.
	 * \returns A string vector containing the name of all the device types.
	 *     If a device is untested, it will be prefixed with an asterisk.
	 */
	static vector<string> *list(void);

	/** Creates an instance of a device given only its name. This function is
	 * basically a switch which parses the name and returns an object of
	 * the subclass which describes the device.
	 *
	 * \param name The name of the device (case sensitive).
	 * \retval NULL if the device is unknown.
	 * \retval Device An instance of a subclass of Device representing the
	 *         device given by the name parameter.
	 */
	static Device *load(char *name);

	/** Frees all the memory and resources associated with this object */
	virtual ~Device();

	/** Quickly check that the device is present and correct.
	 * \pre set_iodevice() must have been called with a valid IO object.
	 * \returns A boolean value indicating if the check was successful.
	 * \throws runtime_error Contains a textual description of the error.
	 */
	virtual bool check(void) = 0;

	/** Erase the memory contents of the device. This is only applicable
	 * to EEPROM or flash devices that can be electronically erased.
	 * \pre set_iodevice() must have been called with a valid IO object.
	 * \throws runtime_error Contains a textual description of the error.
	 */
	virtual void erase(void) = 0;

	/** Program the contents of the DataBuffer to the device. This performs
	 * no erasure or blank check.
	 * \param buf The DataBuffer containing the data to program.
	 * \pre set_iodevice() must have been called with a valid IO object.
	 * \throws runtime_error Contains a textual description of the error.
	 */
	virtual void program(DataBuffer& buf) = 0;

	/** Read the contents of a device into the DataBuffer.
	 * \param buf The DataBuffer to store the read data.
	 * \param verify If this flag is true, don't store the data in the
	 *        DataBuffer but verify the contents of it.
	 * \pre set_iodevice() must have been called with a valid IO object.
	 * \throws runtime_error Contains a textual description of the error.
	 */
	virtual void read(DataBuffer& buf, bool verify=false) = 0;

	/** Prepare the default value of the configuration registers.
	 */
	virtual void set_config_default(DataBuffer& buf) = 0;

	/** Retrieves the name of the device.
	 * \returns A string containing the name of the device.
	 */
	string get_name(void);

	/** Get a vector of the valid memory locations of a device. A vector is
	 * returned which contains pairs of integers. The first of the pair is the
	 * memory segment base and the second is the length. This is used to
	 * find out what portions of a read device are valid.
	 *
	 * Example. After reading a device into \c buf, write it to HexFile \c hf.
	 * \code
	 * IntPairVector mmap = device->get_mmap();
	 * IntPairVector::iterator n = mmap.begin();
	 * for(; n != mmap.end(); n++)
	 *     hf->write(buf, n->first, n->second);
	 * \endcode
	 */
	IntPairVector& get_mmap(void);

	/** Gets the native word size of the device (in bits).
	 * \returns The number of bits in a data word.
	 */
	int get_wordsize(void);

	/** Sets the instance of the IO class that should be used to communicate
	 * with the hardware. */
	void set_iodevice(IO *iodev);

	/** Set the current progress callback. The callback will be called
	 * every time a word is read/written to the device. The callback also
	 * returns a boolean value indicating if the operation should continue.
	 * This allows a user interface to gracefully stop an operation.
	 * \param cb The callback function to set. Set this to NULL to disable
	 *        the callback.
	 * \param data An arbitrary pointer that is passed directly to the
	 *        callback function.
	 */
	void set_progress_cb(bool (*cb)(void *data, long addr, int percent),
	  void *data=NULL);

protected:
	/** The constructor just initializes the Device class variables to
	 * default values. */
	Device(char *name);

	/** Calls the progress callback, if it has been defined. The percent
	 * completed will be calculated from the progress_counter and
	 * progress_total members.
	 * \param addr The address currently being accessed. Passed verbatim
	 *        to the progress callback.
	 * \returns A boolean value indicating if the current operation should
	 *        continue.
	 */
	bool progress(unsigned long addr);

	/** The size of a data word in bits. The default value for this is 8. */
	int wordsize;

	/** A vector containing pairs of integers that describe the internal
	 * layout of memory in the device. This allows a device with a fragmented
	 * memory space to be described. */
	IntPairVector memmap;

	/** A pointer to an instance of the IO class. This is used internally
	 * by the class to communicate with the actual hardware. */
	IO *io;

	/** Pointer to a function that will be called after a word is
	 * read/written to the device.
	 * \param data The data pointer that was passed to set_progress_cb().
	 * \param addr The current address that is being read/written.
	 * \param percent An integer from 0-100 indicating the percentage done.
	 * \returns A boolean value indicating if the operation should continue.
	 */
	bool (*progress_cb)(void *data, long addr, int percent);

	/** An arbitrary pointer which is passed verbatim to the progress
	 * callback. */
	void *progress_cb_data;

	/** A counter to keep track of the progress of an operation. This must
	 * be initialized before calling the progress() method. */
	unsigned long progress_count;

	/** The total 'count' for a full operation. This is used by the
	 * progress() method to compute the percent complete. */
	unsigned long progress_total;

	/** The name of the device that was given to the constructor. */
	string name;
};


#endif
