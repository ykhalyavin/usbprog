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
 * $Id: Pic16Device.cxx,v 1.3 2006/12/31 14:57:35 gaufille Exp $
 */
using namespace std;

#include <stdio.h>
#include <stdexcept>
#include "config.h"
#include "ConfigFile.h"
#include "PicDevice.h"
#include "IO.h"
#include "Util.h"


Pic16Device::Pic16Device(char *name) : PicDevice(name) {
	long tmp;

	/* Read configuration word bits */
	READ_PIC_CONFIG_ENTRY_DEFAULT("configmask", config_mask, this->wordmask);
	READ_PIC_CONFIG_ENTRY_DEFAULT("configsave", persistent_config_mask, 0);
	READ_PIC_CONFIG_ENTRY_DEFAULT("cp_mask", cp_mask, 0);
	READ_PIC_CONFIG_ENTRY_DEFAULT("cp_all", cp_all, 0);
	READ_PIC_CONFIG_ENTRY_DEFAULT("cp_none", cp_none, 0);
	READ_PIC_CONFIG_ENTRY_DEFAULT("cpd_mask", cpd_mask, 0);
	READ_PIC_CONFIG_ENTRY_DEFAULT("cpd_on", cpd_on, 0);
	READ_PIC_CONFIG_ENTRY_DEFAULT("cpd_off", cpd_off, 0);
	if(pic_config->get_integer(name, "bkbug_mask", &tmp))
		this->flags |= PIC_FEATURE_BKBUG;

	/* Create the memory map for this device */
	this->memmap.push_back(IntPair (0, this->codesize));
	this->memmap.push_back(IntPair (0x2000,
	  (this->flags & PIC_FEATURE_BKBUG) ? 5 : 4));
	this->memmap.push_back(IntPair (0x2007, 1));
	if(this->flags & PIC_FEATURE_EEPROM)
		this->memmap.push_back(IntPair (0x2100, this->eesize));
}


Pic16Device::~Pic16Device() {
}


void Pic16Device::erase(void) {
	if(this->memtype != MEMTYPE_FLASH)
		throw runtime_error("Operation not supported by device");

	/* Read the config word, and OSCAL if this PIC has it */
	unsigned long cword, oscal = 0;
	try {
		this->set_program_mode();

		/* If we need to save the oscillator calibration value, increment
		 * to the OSCAL address and read it. */
		if(this->flags & PIC_HAS_OSCAL) {
			for(unsigned int i=0; i<this->codesize-1; i++)
				this->write_command(COMMAND_INC_ADDRESS);
			oscal = this->read_prog_data();
		}

		/* Read the current configuration word to determine if code
		 * protection needs to be disabled. */
		this->write_command(COMMAND_LOAD_CONFIG);
		this->io->shift_bits_out(0x7ffe, 16, 1);	/* Dummy write of all 1's */
		this->io->usleep(1);

		/* Skip to the configuration word */
		for(int i=0; i < 7; i++)
			this->write_command(COMMAND_INC_ADDRESS);

		cword = read_config_word();
		this->pic_off();
	} catch(std::exception& e) {
		this->pic_off();
		throw;
	}

	/* Wait a bit after exiting program mode */
	this->io->usleep(10000);

	/* If code protection or "data code protection" is enabled, we need to do
	 * a disable_codeprotect() */
	if(((cword & this->cp_mask) != this->cp_none) ||
	  ((cword & this->cpd_mask) != this->cpd_off)) {
		this->disable_codeprotect();
	} else {
		this->bulk_erase();
	}

	/* Check if we need to restore some state. */
	if((this->persistent_config_mask != 0) || (this->flags & PIC_HAS_OSCAL)) {
		/* Wait a bit after exiting program mode */
		this->io->usleep(10000);

		try {
			this->set_program_mode();

			/* Restore the OSCAL value */
			if(this->flags & PIC_HAS_OSCAL) {
				/* Increment to the OSCAL address */
				for(unsigned int i=0; i<this->codesize-1; i++)
					this->write_command(COMMAND_INC_ADDRESS);
				if(! this->program_one_location(oscal)) {
					THROW_ERROR(runtime_error,
					  "Couldn't restore OSCAL value of 0x%04lx", oscal);
				}
			}

			if(this->persistent_config_mask != 0) {
				try {
					/* Restore the persistent configuration word bits */
					this->write_command(COMMAND_LOAD_CONFIG);
					this->io->shift_bits_out(0x7ffe, 16, 1);	/* Dummy write of all 1's */
					this->io->usleep(1);

					/* Skip to the configuration word */
					for(int i=0; i < 7; i++)
						this->write_command(COMMAND_INC_ADDRESS);

					write_config_word((cword & this->persistent_config_mask) |
					  (this->wordmask & ~this->persistent_config_mask));
				} catch(std::exception& e) {
					THROW_ERROR(runtime_error,
					  "Couldn't restore configuration bits. Values 0x%04lx",
					  cword & this->persistent_config_mask);
				}
			}
			this->pic_off();
		} catch(std::exception& e) {
			this->pic_off();
			throw;
		}
	}
}


void Pic16Device::program(DataBuffer& buf) {
	uint32_t data;

	switch(this->memtype) {
	case MEMTYPE_EPROM:
	case MEMTYPE_FLASH:
		break;
	default:
		throw runtime_error("Unsupported memory type in device");
	}

	this->progress_total = this->codesize + this->eesize + 4;
	this->progress_count = 0;

	try {
		this->set_program_mode();

		/* Write the program memory */
		this->write_program_memory(buf);

		/* Write the data EEPROM if this PIC has one */
		if(this->flags & PIC_FEATURE_EEPROM)
			this->write_data_memory(buf, 0x2100);

		/* Write the ID locations */
		this->write_command(COMMAND_LOAD_CONFIG);
		this->io->shift_bits_out(0x7ffe, 16, 1);
		this->io->usleep(1);
		this->write_id_memory(buf, 0x2000);

		/* Write the debugger interrupt location if this PIC has one */
		if(this->flags & PIC_FEATURE_BKBUG) {
			if(!program_one_location((uint32_t)buf[0x2004]))
				throw runtime_error("Couldn't write the debugger " \
				  "interrupt vector at address 0x2004.");
		}

		/* Skip past the next 3 addresses */
		for(int i=4; i < 7; i++)
			this->write_command(COMMAND_INC_ADDRESS);

		/* Program the config word, keeping the persistent bits. */
		progress(0x2007);
		data = buf[0x2007] & ~this->persistent_config_mask;
		data |= (read_config_word() & this->persistent_config_mask);
		this->write_config_word(data);
		this->progress_count++;

		this->pic_off();
	} catch(std::exception& e) {
		this->pic_off();
		throw;
	}
}


void Pic16Device::read(DataBuffer& buf, bool verify) {
	this->progress_total = this->codesize + this->eesize + 4;
	this->progress_count = 0;

	try {
		this->set_program_mode();

		/* Read the program memory */
		this->read_program_memory(buf, 0, verify);

		/* Read the data EEPROM if this PIC has one */
		if(this->flags & PIC_FEATURE_EEPROM)
			this->read_data_memory(buf, 0x2100, verify);

		/* Read the ID locations */
		this->write_command(COMMAND_LOAD_CONFIG);
		this->io->shift_bits_out(0x7ffe, 16, 1);	/* Dummy write of all 1's */
		this->io->usleep(1);
		this->read_id_memory(buf, 0x2000, verify);

		/* Read the debugger interrupt location if this PIC has one */
		if(this->flags & PIC_FEATURE_BKBUG) {
			if(verify) {
				if((uint32_t)buf[0x2004] != this->read_prog_data())
					throw runtime_error("Verification failed at address 0x2004");
			} else {
				buf[0x2004] = this->read_prog_data();
			}
		}

		/* Skip past the next 3 addresses */
		for(int i=4; i < 7; i++)
			this->write_command(COMMAND_INC_ADDRESS);

		progress(0x2007);
		if(verify) {
			this->read_config_word(true, buf[0x2007]);
		} else {
			buf[0x2007] = this->read_config_word();
		}
		this->progress_count++;

		this->pic_off();
	} catch(std::exception& e) {
		this->pic_off();
		throw;
	}
}


void Pic16Device::bulk_erase(void) {
	try {
		/* Bulk Erase Program Memory */
		/*   Set PC to config memory (to erase ID locations) */
		/*   Do a "Load Data All 1's" command. */
		/*   Do a "Bulk Erase Program Memory" command. */
		/*   Do a "Begin Programming" command. */
		/*   Wait erase_time to complete bulk erase. */
		this->set_program_mode();
		this->write_command(COMMAND_LOAD_CONFIG);
		this->io->shift_bits_out(0x7ffe, 16, 1);
		this->io->usleep(1);
		this->write_command(COMMAND_ERASE_PROG_MEM);
		this->write_command(COMMAND_BEGIN_PROG);
		this->io->usleep(this->erase_time);

		if(this->flags & PIC_FEATURE_EEPROM) {
			/* Bulk Erase Data Memory */
			/*   Do a "Load Data All 1's" command. */
			/*   Do a "Bulk Erase Data Memory" command. */
			/*   Do a "Begin Programming" command. */
			/*   Wait erase_time to complete bulk erase. */
			this->write_ee_data(0x00ff);
			this->write_command(COMMAND_ERASE_DATA_MEM);
			this->write_command(COMMAND_BEGIN_PROG);
			this->io->usleep(this->erase_time);
		}
		this->pic_off();
	} catch(std::exception& e) {
		this->pic_off();
		throw;
	}
}


void Pic16Device::disable_codeprotect(void) {
	throw runtime_error("Disabling code protecion on this device isn't supported.");
}


void Pic16Device::write_program_memory(DataBuffer& buf, long base) {
	unsigned int offset;

	try {
		for(offset=0; offset < this->codesize; offset++) {
			progress(base+offset);

			/* Skip but verify blank locations to save time */
			if(buf.isblank(base+offset)) {
				/* Don't verify the OSCAL location. */
				if(! ((this->flags & PIC_HAS_OSCAL) &&
				  (offset == this->codesize-1))) {
					if(read_prog_data() != (uint32_t)buf[base+offset])
						break;
				}
			} else {
				if(!this->program_one_location((uint32_t)buf[base+offset]))
					break;
			}
			this->write_command(COMMAND_INC_ADDRESS);
			this->progress_count++;
		}
		if(offset < this->codesize) throw runtime_error("");
	} catch(std::exception& e) {
		THROW_ERROR(runtime_error,
		  "Couldn't write program memory at address 0x%04lx", base+offset);
	}
}


void Pic16Device::read_program_memory(DataBuffer& buf, long base, bool verify) {
	unsigned int offset;

	try {
		for(offset=0; offset < this->codesize; offset++) {
			progress(base+offset);

			if(verify) {
				/* Don't verify the OSCAL location. */
				if(! ((this->flags & PIC_HAS_OSCAL) &&
				  (offset == this->codesize-1))) {
					if((uint32_t)buf[base+offset] != this->read_prog_data())
						throw runtime_error("");
				}
			} else {
				buf[base+offset] = this->read_prog_data();
			}
			this->write_command(COMMAND_INC_ADDRESS);
			this->progress_count++;
		}
	} catch(std::exception& e) {
		THROW_ERROR(runtime_error, "%s at address 0x%04lx",
		  verify ? "Verification failed" : "Couldn't read program memory",
 		  base+offset);
	}
}


void Pic16Device::write_data_memory(DataBuffer& buf, long base) {
	unsigned int offset;

	try {
		for(offset=0; offset < this->eesize; offset++) {
			progress(base+offset);

			/* Skip but verify blank locations to save time */
			if((buf[base+offset] & 0xff) == 0xff) {
				if(read_ee_data() != (uint32_t)(buf[base+offset] & 0xff))
					break;
			} else {
				this->write_ee_data(buf[base+offset]);
				this->write_command(COMMAND_BEGIN_PROG);
				this->io->usleep(this->program_time);
				if(read_ee_data() != (uint32_t)(buf[base+offset] & 0xff))
					break;
			}
			this->write_command(COMMAND_INC_ADDRESS);
			this->progress_count++;
		}
		if(offset < this->eesize) throw runtime_error("");
	} catch(std::exception& e) {
		THROW_ERROR(runtime_error,
		  "Couldn't write data memory at address 0x%04lx", base+offset);
	}
}


void Pic16Device::read_data_memory(DataBuffer& buf, long base, bool verify) {
	unsigned int offset;

	try {
		for(offset=0; offset < this->eesize; offset++) {
			progress(base+offset);

			if(verify) {
				if((uint32_t)(buf[base+offset] & 0xff) != this->read_ee_data())
					throw runtime_error("");
			} else {
				buf[base+offset] = this->read_ee_data();
			}
			this->write_command(COMMAND_INC_ADDRESS);
			this->progress_count++;
		}
	} catch(std::exception& e) {
		THROW_ERROR(runtime_error, "%s at address 0x%04lx",
		  verify ? "Verification failed" : "Couldn't read data memory",
 		  base+offset);
	}
}


void Pic16Device::write_id_memory(DataBuffer& buf, long base) {
	unsigned int offset;

	try {
		for(offset=0; offset < 4; offset++) {
			progress(base+offset);

			/* Skip but verify blank locations to save time */
			if(buf.isblank(base+offset)) {
				if(read_prog_data() != (uint32_t)buf[base+offset])
					break;
			} else {
				if(!this->program_one_location((uint32_t)buf[base+offset]))
					break;
			}
			this->write_command(COMMAND_INC_ADDRESS);
			this->progress_count++;
		}
		if(offset < 4) throw runtime_error("");
	} catch(std::exception& e) {
		THROW_ERROR(runtime_error,
		  "Couldn't write ID memory at address 0x%04lx", base+offset);
	}
}


void Pic16Device::read_id_memory(DataBuffer& buf, long base, bool verify) {
	int offset;

	try {
		for(offset=0; offset < 4; offset++) {
			progress(base+offset);

			if(verify) {
				if((uint32_t)buf[base+offset] != this->read_prog_data())
					throw runtime_error("");
			} else {
				buf[base+offset] = this->read_prog_data();
			}
			this->write_command(COMMAND_INC_ADDRESS);
			this->progress_count++;
		}
	} catch(std::exception& e) {
		THROW_ERROR(runtime_error, "%s at address 0x%04lx",
		  verify ? "Verification failed" : "Couldn't read ID memory",
 		  base+offset);
	}
}


void Pic16Device::write_config_word(uint32_t data) {
	int count;

	switch(this->memtype) {
	case MEMTYPE_EPROM:
		count = 100;
		break;
	case MEMTYPE_FLASH:
		count = 1;
		break;
	default:
		throw logic_error("I'm confused");
	}

	/* Now do it! */
	bool ok = false;
	while(count > 0) {
		ok = this->program_cycle(data, this->config_mask);
		count--;
	}
	if(!ok) throw runtime_error("Couldn't write the configuration word");
}


uint32_t Pic16Device::read_config_word(bool verify, uint32_t verify_data) {
	uint32_t data;

	try {
		data = read_prog_data();
		if(verify) {
			/* We don't include persistent config bits in a verify */
			uint32_t mask = this->config_mask & ~this->persistent_config_mask;
			if((verify_data & mask) != (data & mask))
				throw runtime_error("");
		}
	} catch(std::exception& e) {
		THROW_ERROR(runtime_error, "Couldn't %s config word.",
		  verify ? "verify" : "read");
	}
	return data;
}


bool Pic16Device::program_one_location(uint32_t data) {
	unsigned int count;

	/* Program up to the initial count */
	for(count=1; count <= this->program_count; count++) {
		if(this->program_cycle(data))
			break;
	}
	if(count > this->program_count)
		return false;

	/* Program count*multiplier more times */
	count *= this->program_multiplier;
	while(count > 0) {
		if(!this->program_cycle(data))
			return false;
		count--;
	}
	return true;
}


void Pic16Device::write_ee_data(uint32_t data) {
	data = (data & this->wordmask) << 1;
	this->write_command(COMMAND_LOAD_DATA_DATA);
	this->io->shift_bits_out(data, 16, 1);
	this->io->usleep(1);
}


uint32_t Pic16Device::read_ee_data(void) {
	uint32_t data;

	this->write_command(COMMAND_READ_DATA_DATA);
	data = this->io->shift_bits_in(16, 0);
	this->io->usleep(1);

	return (data >> 1) & 0xff;
}


uint32_t Pic16Device::read_deviceid(void) {
	uint32_t devid;

	/* Enter config memory space. The device ID is at address 0x2006 */
	this->write_command(COMMAND_LOAD_CONFIG);
	this->io->shift_bits_out(0x7ffe, 16);	/* Dummy write of all 1's */
	this->io->usleep(1);

	/* Increment up to 0x2006 */
	for(int i=0; i < 6; i++)
		this->write_command(COMMAND_INC_ADDRESS);

	devid = read_prog_data();
	return devid;
}
