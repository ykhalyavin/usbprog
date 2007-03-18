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
 * $Id: PicDevice.h,v 1.5 2006/12/02 22:28:11 gaufille Exp $
 */
#ifndef __PicDevice_h
#define __PicDevice_h

#include <stdint.h>
#include "ConfigFile.h"
#include "DataBuffer.h"
#include "Device.h"
#include "IO.h"

/** \file */


/** Global handle to the pic.conf configuration file. */
extern ConfigFile *pic_config;

typedef pair<int, int> IntPair;			/**< A pair of integers */


#define PIC_FEATURE_EEPROM 0x00000001	/**< PIC has a data EEPROM */
#define PIC_REQUIRE_EPROG  0x00000002	/**< PIC requires END_PROG command */
#define PIC_FEATURE_BKBUG  0x00000004	/**< PIC has on-chip debugger */
#define PIC_HAS_OSCAL      0x00000008	/**< PIC has an OSCAL value that must be saved and restored on a chip erase */
#define PIC_HAS_DEVICEID   0x00000010	/**< PIC has a device ID that can be read by the programming software */


/** Macro to assign a default value if the configuration variable isn't set */
#define READ_PIC_CONFIG_ENTRY_DEFAULT(cfgent, varname, defval) \
	do { \
		long __tmpval; \
		if(pic_config->get_integer(name, cfgent, &__tmpval)) { \
			this->varname = __tmpval; \
		} else { \
			this->varname = (defval); \
		} \
	} while(0)


/** A Device implementation which implements a base class for Microchip's PIC
 * microcontrollers. These microcontrollers are programmed serially and have
 * a word size of 12, 14, or 16 bits. They come in memory configurations of
 * flash or EPROM with some devices having an optional EEPROM dedicated to
 * data storage. */
class PicDevice : public Device {
public:
	/* PIC commands */
	const static int
		COMMAND_LOAD_CONFIG=0x00,	/**< Load Configuration */
		COMMAND_LOAD_PROG_DATA=0x02,/**< Load Data for Program Memory */
		COMMAND_READ_PROG_DATA=0x04,/**< Read Data from Program Memory */
		COMMAND_INC_ADDRESS=0x06,	/**< Increment Address */
		COMMAND_BEGIN_PROG=0x08,	/**< Begin Erase/Programming */
		COMMAND_BEGIN_PROG_ONLY=0x18,	/**< Begin Programming only (for 16F81x) */
		COMMAND_END_PROG=0x0e,		/**< End Programming */
		COMMAND_END_PROG_16F81X=0x17,	/**< End Programming for 16F81x */
		COMMAND_LOAD_DATA_DATA=0x03,/**< Load Data for Data Memory */
		COMMAND_READ_DATA_DATA=0x05,/**< Read Data from Data Memory */
		COMMAND_ERASE_PROG_MEM=0x09,/**< Bulk Erase Program Memory */
		COMMAND_ERASE_DATA_MEM=0x0b,/**< Bulk Erase Data Memory */
		COMMAND_CHIP_ERASE=0x1F;    /**< Chip Erase (for 16F81x) */

	/** Values for the different types of PIC memory and their algorithms */
	typedef enum {
		MEMTYPE_EPROM=0,
		MEMTYPE_FLASH,
	} pic_memtype_t;

	/** Gets a list of the available PIC devices.
	 * \param v A vector of strings to which the device names are added.
	 * \post The PIC device types are added to the vector \c v. Any devices
	 *     that have not been tested or are experimental are prefixed with an
	 *     asterisk.
	 */
	static void list(vector<string> *v);

	/** Creates an instance of a PIC device from its name. This function is
	 * called from Device::load and selects the appropriate Pic16Device subclass
     * to instantiate. This allows for efficient implementation of different
	 * PIC programming algorithms.
	 *
	 * \param name The name of the device (case sensitive).
	 * \retval NULL if the device is unknown.
	 * \retval Device An instance of a subclass of Device representing the
	 *         device given by the name parameter.
	 */
	static Device *load(char *name);

	/** Constructor */
	PicDevice(char *name);

	/** Destructor */
	~PicDevice();

	virtual bool check(void);

protected:
	/** Perform a single program cycle for program memory. The following steps
	 * are performed:
	 *   - The data is written to the PIC with write_prog_data().
	 *   - The BEGIN_PROG command is sent.
	 *   - A delay of program_time is initiated.
	 *   - If the PIC requires it, the END_PROG command is sent.
	 *   - A readback is performed and is compared with the original data.
	 * \param data The data word to program into program memory at the PIC's
	 *        program counter.
	 * \param mask A mask which is used when verifying the data. The read back
	 *             data and the data parameter are both masked with this mask
	 *             before being compared. This is required for properly
	 *             verifying the configuration word.
	 * \returns A boolean value indicating if the data read back matches the
	 *          data parameter.
	 */
	virtual bool program_cycle(uint32_t data, uint32_t mask=0xffffffff);

	/** Put the PIC device in program/verify mode. On entry to
	 * program/verify mode, the program counter is set to 0 or -1 depending
	 * on the device.
	 */
	virtual void set_program_mode(void);

	/** Turn off the PIC device. This will set the clock and data lines to
	 * low and shut off both Vpp and Vcc. */
	virtual void pic_off(void);

	/** Write a 6 bit command to the PIC. After the write, a 1us delay is
	 * performed as required by the device.
	 * \param command The 6 bit command to write.
	 */
	virtual void write_command(uint32_t command);

	/** Send the LOAD_PROG_DATA command and write the data to the PIC at
	 * the address specified by the program counter. This method doesn't
	 * actually program the data permanently into the PIC. It's part of the
	 * procedure though.
	 * \param data The program data to send to the PIC.
	 */
	virtual void write_prog_data(uint32_t data);

	/** Send the READ_PROG_DATA command and read in the program data at the
	 * address specified by program counter.
	 * \returns The program data at the current PIC address.
	 */
	virtual uint32_t read_prog_data(void);

	/** Read the device ID from the device. 
	 * \pre The device has just been put into program mode.
	 * \post The device is in an undetermined state. Programming may not
	 *       be possible unless program mode is exited and re-entered.
	 * \returns The value of the device ID word.
	 */
	virtual uint32_t read_deviceid(void);
	
	/** Prepare the default value of the configuration registers.
	 */
	virtual void set_config_default(DataBuffer& buf);
	
/* Protected data: */
	/** The calculated bitmask for clearing upper bits of a word. */
	uint32_t wordmask;

	/** Flags for this PIC device. */
	uint32_t flags;

	/** The memory type of this PIC. */
	pic_memtype_t memtype;

	/** Number of words of code this PIC has. */
	unsigned int codesize;

	/** Number of bytes of data EEPROM this PIC has. Only valid if
	 * \c PIC_FEATURE_EEPROM is set. */
	unsigned int eesize;

	/** Maximum number of times to attempt to program a memory location before
	 * reporting an error. */
	unsigned int program_count;

	/** Overprogramming multiplier. After a memory location has been programmed
	 * and successfully verified, program it (count * program_multiplier) more
	 * times. */
	unsigned int program_multiplier;

	/** The number of microseconds it takes to program one memory location.
	 * This value is defined in the PIC datasheet. */
	unsigned int program_time;

	/** The amount of time (in microseconds) it takes to erase an electrically
	 * erasable device. */
	unsigned int erase_time;

	/** Expected device ID value and mask for which bits to care about.
	 * These are only valid if the PIC_HAS_DEVICEID flag is set. */
	uint32_t deviceid, deviceidmask;
};


/** An implementation of the Device class which supports most 14-bit devices
 * with the part number prefix 16. */
class Pic16Device : public PicDevice {
public:
	/** Create a new instance and read in in the configuration for the PIC
	 * device. This constructor is called from Device::load() when the device
	 * name begins with the string "PIC". This function will open the PIC
	 * device configuration file "pic.conf" and read the configuration for
	 * the device specified.
	 * \param name The name of the PIC device.
	 * \throws runtime_error Contains a description of the error.
	 */
	Pic16Device(char *name);

	/** Destructor */
	~Pic16Device();

	virtual void erase(void);
	virtual void program(DataBuffer& buf);
	virtual void read(DataBuffer& buf, bool verify=false);

protected:
	/** Bulk erase the program and data memory of a flash device.
	 * \pre The device has flash memory.
	 * \pre Code protection on the device is verified to be completely off.
	 * \pre The device is turned completely off.
	 * \post The device is turned completely off.
	 * \throws runtime_error Contains a description of the error.
	 */
	virtual void bulk_erase(void);

	/** Disables code protect on a flash PIC and clears the program and data
	 * memory.
	 * \pre The device has flash memory.
	 * \pre The device is turned completely off.
	 * \post The device is turned completely off.
	 * \throws runtime_error Contains a description of the error.
	 */
	virtual void disable_codeprotect(void);

	/** Program the entire contents of program memory to the PIC device.
	 * \param buf A DataBuffer containing the data to program.
	 * \param base The offset within the data buffer to start retrieving data.
	 *        If this parameter isn't specified, it defaults to 0.
	 * \pre The PIC should have it's program counter set to the beginning of
	 *      program memory.
	 * \post The program counter is pointing to the address immediatly after
	 *       the last program memory address.
	 * \throws runtime_error Contains a description of the error along with
	 *         the address at which the error occurred.
	 */
	virtual void write_program_memory(DataBuffer& buf, long base=0);

	/** Read the entire contents of program memory from the PIC device.
	 * \param buf A DataBuffer in which to store the read data.
	 * \param base The offset within the data buffer to start storing data.
	 *        If this parameter isn't specified, it defaults to 0.
	 * \param verify If this flag is true, don't store the data in the
	 *        DataBuffer but verify the contents of it.
	 * \pre The PIC should have it's program counter set to the beginning of
	 *      program memory.
	 * \post The program counter is pointing to the address immediatly after
	 *       the last program memory address.
	 * \throws runtime_error Contains a description of the error along with
	 *         the address at which the error occurred.
	 */
	virtual void read_program_memory(DataBuffer& buf, long base=0,
	  bool verify=false);

	/** Program the data EEPROM contents to the PIC device.
	 * \param buf A DataBuffer containing the data to program.
	 * \param base The offset within the data buffer to start retrieving data.
	 * \throws runtime_error Contains a description of the error along with
	 *         the address at which the error occurred.
	 */
	virtual void write_data_memory(DataBuffer& buf, long base);

	/** Read the data EEPROM contents from the PIC device.
	 * \param buf A DataBuffer in which to store the read data.
	 * \param base The offset within the data buffer to start storing data.
	 * \param verify If this flag is true, don't store the data in the
	 *        DataBuffer but verify the contents of it.
	 * \throws runtime_error Contains a description of the error along with
	 *         the address at which the error occurred.
	 */
	virtual void read_data_memory(DataBuffer& buf, long base, bool verify);

	/** Program the ID words to the PIC device.
	 * \param buf A DataBuffer containing the data to program.
	 * \param base The offset within the data buffer to start retrieving data.
	 * \pre The PIC should have it's program counter set to the location of
	 *      the ID words.
	 * \post The program counter is pointing to the address immediatly after
	 *       the last ID word.
	 * \throws runtime_error Contains a description of the error along with
	 *         the address at which the error occurred.
	 */
	virtual void write_id_memory(DataBuffer& buf, long base);

	/** Read the ID words from the PIC device.
	 * \param buf A DataBuffer in which to store the read data.
	 * \param base The offset within the data buffer to start storing data.
	 * \param verify If this flag is true, don't store the data in the
	 *        DataBuffer but verify the contents of it.
	 * \pre The PIC should have it's program counter set to the location of
	 *      the ID words.
	 * \post The program counter is pointing to the address immediatly after
	 *       the last ID word.
	 * \throws runtime_error Contains a description of the error along with
	 *         the address at which the error occurred.
	 */
	virtual void read_id_memory(DataBuffer& buf, long base, bool verify);

	/** Program the config word to the PIC device.
	 * \param data The value of the configuration word to write.
	 * \pre The PIC program counter should be pointing to the location of
	 *      the configuration word.
	 * \post The PIC program counter still points to the configuration word.
	 * \throws runtime_error Contains a description of the error.
	 */
	virtual void write_config_word(uint32_t data);

	/** Read the config word from the PIC device.
	 * \param verify If this flag is true, compare \c verify_data against the
	 *        configuration word.
	 * \param verify_data If \c verify is true, read the configuration word
	 *     and compare it against this value. The comparison will only
	 *     include the bits in \c config_mask.
	 * \pre The PIC program counter should be pointing to the location of
	 *      the configuration word.
	 * \post The PIC program counter still points to the configuration word.
	 * \throws runtime_error Contains a description of the error.
	 * \returns The raw value of the configuration word as read from the PIC
	 *      device.
	 */
	virtual uint32_t read_config_word(bool verify=false, uint32_t verify_data=0);

	/** Applies up to \c program_count programming cycles to the PIC device
	 * and then applies the appropriate number of overprogramming cycles.
	 * This algorithm is probably used for every type of PIC in existance.
	 * \param data The data value to program into memory.
	 * \pre The PIC program counter should be pointing to the location in
	 *      memory that will be programmed.
	 * \returns A boolean value indicating if the programming was successful.
	 */
	virtual bool program_one_location(uint32_t data);

	/** Write a byte to the data EEPROM. The data is read from the location
     * specified by the program counter. The first call to this or
	 * read_ee_data() sets the program counter to the beginning of the EEPROM
	 * and writes to there.
	 * \param data The daya byte to write to the EEPROM.
	 */
	virtual void write_ee_data(uint32_t data);

	/** Read a byte from the data EEPROM. The data is read from the location
	 * specified by the program counter. The first call to this or
	 * write_ee_data() sets the program counter to the beginning of the EEPROM
	 * and reads from there.
	 * \returns The data byte that was read from the EEPROM.
	 */
	uint32_t read_ee_data(void);

	virtual uint32_t read_deviceid(void);

/* Protected data: */
	/** Bitmask for valid bits in the configuration word. */
	unsigned int config_mask;

	/** A bitmask for configuration bits that must be preserved. */
	unsigned int persistent_config_mask;

	/** Code protection config bits. */
	unsigned int cp_mask, cp_all, cp_none;

	/** Data protection bits. */
	unsigned int cpd_mask, cpd_on, cpd_off;
};


/** A class which implements device-specific functions for PIC16F81x devices.
 */
class Pic16f81xDevice : public Pic16Device {
public:
	Pic16f81xDevice(char *name);	/**< Constructor */
	~Pic16f81xDevice();				/**< Destructor */

protected:
        void erase(void);
	void write_program_memory(DataBuffer& buf, long base=0);
        void write_data_memory(DataBuffer& buf, long base);
        void write_id_memory(DataBuffer& buf, long base);

        bool program_cycle(uint32_t data, uint32_t mask);
        bool program4_cycle(uint32_t data[4]);
};


/** A class which implements device-specific functions for PIC16F8xx devices.
 */
class Pic16f8xxDevice : public Pic16Device {
public:
	Pic16f8xxDevice(char *name);	/**< Constructor */
	~Pic16f8xxDevice();				/**< Destructor */

protected:
	void bulk_erase(void);
	void disable_codeprotect(void);
};


/** A class which implements device-specific functions for PIC16F8xx devices.
 */
class Pic16f87xADevice : public Pic16Device {
public:
	const static int
		COMMAND_BEGIN_PROG_ONLY=0x18,	/**< Begin Programming Only (no erase) */
		COMMAND_END_PROG=0x17,		/**< End Programming */
		COMMAND_CHIP_ERASE=0x1f;	/**< Chip Erase */

	Pic16f87xADevice(char *name);	/**< Constructor */
	~Pic16f87xADevice();			/**< Destructor */

protected:
	void disable_codeprotect(void);
	void bulk_erase(void);
	void write_program_memory(DataBuffer& buf, long base=0);
	void write_config_word(uint32_t data);
};


/** A class for device-specific functions of the PIC16F6XX devices.
 */
class Pic16f6xxDevice : public Pic16Device {
public:
	Pic16f6xxDevice(char *name);	/**< Constructor */
	~Pic16f6xxDevice();				/**< Destructor */

protected:
	void bulk_erase(void);
	void disable_codeprotect(void);
};


/** A class for device-specific functions of the PIC12F6XX devices. Although
 * the part numbers have a "12" prefix, they have 14-bit instruction words and
 * look very much like PIC16 devices from a programmers point of view.
 */
class Pic12f6xxDevice : public Pic16Device {
public:
	Pic12f6xxDevice(char *name);	/**< Constructor */
	~Pic12f6xxDevice();				/**< Destructor */

protected:
	void disable_codeprotect(void);
};


/** A class which implements device-specific functions for PIC16F7x devices.
 */
class Pic16f7xDevice : public Pic16Device {
public:
	Pic16f7xDevice(char *name);	/**< Constructor */
	~Pic16f7xDevice();			/**< Destructor */

protected:
	void disable_codeprotect(void);
};


/** A class which implements the programming algorithm PIC18* devices. The
 * PIC18* devices are different in many ways:
 *   * Programming/reading is done in bytes.
 *   * Much larger internal address space
 *   * Programming is accomplished by executing single instructions on the
 *     PIC core.
 *   * Programming can be done 32 bytes at a time.
 *   * Data EEPROM, configuration words, and ID locations  are at different
 *     addresses than the PIC16* devices.
 *   * There are multiple configuration words.
 * This class will provide a base class for devices of this type.
 */
class Pic18Device : public PicDevice {
public:
	/* PIC18* commands */
	const static int
		COMMAND_CORE_INSTRUCTION=0x00,	/**< Execute an instruction */
		COMMAND_SHIFT_OUT_TABLAT=0x02,	/**< Shift out TABLAT register */
		COMMAND_TABLE_READ=0x08,		/**< Table Read */
		COMMAND_TABLE_READ_POSTINC=0x09,/**< Table Read, post-increment */
		COMMAND_TABLE_READ_POSTDEC=0x0a,/**< Table Read, post-decrement */
		COMMAND_TABLE_READ_PREINC=0x0b,	/**< Table Read, pre-increment */
		COMMAND_TABLE_WRITE=0x0c,		/**< Table Write */
		COMMAND_TABLE_WRITE_POSTINC=0x0d,/**< Table Write, post-inc by 2 */
		COMMAND_TABLE_WRITE_POSTDEC=0x0e,/**< Table Write, post-dec by 2 */
		COMMAND_TABLE_WRITE_START=0x0f;	/**< Table Write, start programming */

	Pic18Device(char *name);	/**< Constructor */
	virtual ~Pic18Device();		/**< Destructor */

	virtual void erase(void);
	virtual void program(DataBuffer& buf);
	virtual void read(DataBuffer& buf, bool verify=false);

protected:
	/** Writes data to the program memory. The entire code space is written
	 * to take advantage of multi-panel writes.
	 * \param buf The DataBuffer from which to retrieve the data to write.
	 * \param verify A boolean value indicating if the written data should be
	 *        read back and verified.
	 * \post The \c progress_count is incremented by the number of words
	 *       written to program memory. If \c verify is true then
	 *       \c progress_count will have been incremented by two times the
	 *       number of words written; once for the write and once for the
	 *       verify.
	 * \throws runtime_error Contains a description of the error along with
	 *         the address at which the error occurred.
	 */
	virtual void write_program_memory(DataBuffer& buf, bool verify);

	/** Writes the ID memory locations.
	 * \param buf The DataBuffer from which data is read.
	 * \param addr The byte address of the ID words in the PIC address space.
	 * \param verify A boolean value indicating if the written data should be
	 *        read back and verified.
	 * \post The \c progress_count is incremented by the number of words
	 *       written to the ID locations. If \c verify is true then
	 *       \c progress_count will have been incremented by two times the
	 *       number of words written; once for the write and once for the
	 *       verify.
	 * \throws runtime_error Contains a description of the error along with
	 *         the address at which the error occurred.
	 */
	virtual void write_id_memory(DataBuffer& buf, unsigned long addr, bool verify);

	/** Writes data to the data eeprom.
	 * \param buf The DataBuffer from which to retrieve the data to write.
	 * \param addr The byte offset into the DataBuffer from which to start
	 *             retrieving data.
	 * \param verify A boolean value indicating if the written data should be
	 *        read back and verified.
	 * \post The \c progress_count is incremented by the number of bytes
	 *       written to the data memory. If \c verify is true then
	 *       \c progress_count will have been incremented by two times the
	 *       number of bytes written; once for the write and once for the
	 *       verify.
	 * \throws runtime_error Contains a description of the error along with
	 *         the data memory location at which the error occured.
	 */
	virtual void write_data_memory(DataBuffer& buf, unsigned long addr, bool verify);

	/** Writes the configuration words
	 * \param buf The DataBuffer from which data is read.
	 * \param addr The byte address of the configuration words in the PIC
	 *        address space.
	 * \param verify A boolean value indicating if the written data should be
	 *        read back and verified.
	 * \post The \c progress_count is incremented by the number of
	 *       configuration words written. If \c verify is true then
	 *       \c progress_count will have been incremented by two times the
	 *       number of words written; once for the write and once for the
	 *       verify.
	 * \throws runtime_error Contains a description of the error along with
	 *         the configuration word number where the error occurred.
	 */
	virtual void write_config_memory(DataBuffer& buf, unsigned long addr, bool verify);

	/** Reads a portion of the PIC memory.
	 * \param buf The DataBuffer to store the read data. On a verify, this
	 *        data is compared with the data on the PIC.
	 * \param addr The byte address in the PIC's memory to begin the read.
	 *        Since the PIC's memory is byte-oriented and the DataBuffer is
	 *        16-bit word oriented, the offset into the DataBuffer will be
	 *        1/2 the PIC memory address.
	 * \param len The number of 16-bit words to read into the DataBuffer.
	 * \param verify Boolean flag which indicates a verify operation against
	 *        \c buf.
	 * \post The \c progress_count is incremented by the number of
	 *       program memory words read/verified.
	 * \throws runtime_error Contains a description of the error along with
	 *         the address at which the error occurred.
	 */
	virtual void read_memory(DataBuffer& buf, unsigned long addr, unsigned long len, bool verify);

	/** Reads a portion of the PIC configuration memory. This is basically the
	 * same as \c read_memory except there is some masking of the data.
	 * \param buf The DataBuffer to store the read data. On a verify, this
	 *        data is compared with the data on the PIC.
	 * \param addr The byte address in the PIC's memory to begin the read.
	 *        Since the PIC's memory is byte-oriented and the DataBuffer is
	 *        16-bit word oriented, the offset into the DataBuffer will be
	 *        1/2 the PIC memory address.
	 * \param len The number of 16-bit words to read into the DataBuffer.
	 * \param verify Boolean flag which indicates a verify operation against
	 *        \c buf.
	 * \post The \c progress_count is incremented by the number of
	 *       configuration words read/verified.
	 * \throws runtime_error Contains a description of the error along with
	 *         the configuration word number where the error occurred.
	 */
	virtual void read_config_memory(DataBuffer& buf, unsigned long addr, unsigned long len, bool verify);

	/** Reads the entire PIC data EEPROM. The bytes are packed into the
	 * DataBuffer as 1 byte per 16-bit word.
	 * \param buf The DataBuffer to store the read data. On a verify, this
	 *        data is compared with the data on the PIC.
	 * \param addr The byte address in the DataBuffer to store the EEPROM
	 *        data. Since the DataBuffer consists of 16-bit words, the
	 *        DataBuffer offset will be 1/2 this.
	 * \param verify Boolean flag which indicates a verify operation against
	 *        \c buf.
	 * \post The \c progress_count is incremented by the number of
	 *       bytes read/verified from the data memory.
	 * \throws runtime_error Contains a description of the error along with
	 *         the data memory location at which the error occured.
	 */
	virtual void read_data_memory(DataBuffer& buf, unsigned long addr, bool verify);

	/** Loads the write buffer for a panel and optionally begins the write
	 * sequence.
	 * \param buf The DataBuffer from which to retrieve data.
	 * \param panel The panel number whose buffer to fill.
	 * \param offset The offset into the panel to write the data.
	 * \param last A boolean value indicating if this is the last panel buffer
	 *        to be loaded. If so, the write is initiated after this panel
	 *        buffer is written.
	 * \post The \c progress_count is incremented by the number of words written
	 *       to the write buffer. On success this is 4.
	 * \post If \c last is true, a call to program_wait() or the equivalent
	 *       must be made so the write is properly timed.
	 */
	virtual void load_write_buffer(DataBuffer& buf, unsigned int panel, unsigned int offset, bool last);

	/** Does a custom NOP/program wait. This will output
	 * COMMAND_CORE_INSTRUCTION, hold clk high for the programming time,
	 * and then finish up by clocking out a nop instruction (16 0's).
	 */
	virtual void program_wait(void);

	/** Sets the value of the PIC's internal TBLPTR register. This register
	 * contains the address of the current read/write operation.
	 * \param addr The byte address within the PIC's address space.
	 */
	virtual void set_tblptr(unsigned long addr);

	/** Send a 4-bit command to the PIC.
	 * \param command The 4-bit command to write.
	 */
	virtual void write_command(unsigned int command);

	/** Sends a 4-bit command with a 16-bit operand to the PIC.
	 * \param command The 4-bit command to write.
	 * \param data The 16-bit operand of the command.
	 */
	virtual void write_command(unsigned int command, unsigned int data);

	/** Writes a 4-bit command and read an 8-bit response. The actual
	 * procedure is to write the 4 bit command, strobe 8 times, and then
	 * clock in the 8 data bits.
	 * \param command The 4-bit command to send.
	 * \returns The 8 bits of data that were read back.
	 */
	virtual unsigned int write_command_read_data(unsigned int command);

	virtual uint32_t read_deviceid(void);
	
	virtual void set_config_default(DataBuffer& buf);

/* Protected Data: */
	/** The bitmasks for each configuration word */
	unsigned int config_masks[7];
};

class Pic18f2xx0Device : public Pic18Device {
public:
	/* PIC18* commands */
	const static int
		COMMAND_TABLE_WRITE_START_POSTINC=0x0e;/**< Table Write, start programming, post-inc by 2 */
	
	Pic18f2xx0Device(char *name);	/**< Constructor */
	virtual ~Pic18f2xx0Device();		/**< Destructor */

	virtual void erase(void);

protected:
	virtual void write_program_memory(DataBuffer& buf, bool verify);

	virtual void write_id_memory(DataBuffer& buf, unsigned long addr, bool verify);

	virtual void write_data_memory(DataBuffer& buf, unsigned long addr, bool verify);

	virtual void write_config_memory(DataBuffer& buf, unsigned long addr, bool verify);

	virtual void read_data_memory(DataBuffer& buf, unsigned long addr, bool verify);

	void load_write_buffer(unsigned int word, bool last);
	
	virtual void program_wait(void);

	// Write and erase buffers sizes
    unsigned long write_buffer_size;
	unsigned long erase_buffer_size;
};


#endif
