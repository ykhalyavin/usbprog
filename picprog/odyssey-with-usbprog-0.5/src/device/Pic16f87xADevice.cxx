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
 * $Id: Pic16f87xADevice.cxx,v 1.3 2006/12/31 14:57:35 gaufille Exp $
 */
using namespace std;

#include <stdio.h>
#include <stdexcept>
#include "PicDevice.h"
#include "Util.h"


Pic16f87xADevice::Pic16f87xADevice(char *name) : Pic16Device(name) {
}


Pic16f87xADevice::~Pic16f87xADevice() {
}


void Pic16f87xADevice::disable_codeprotect(void) {
	try {
		this->set_program_mode();

		/* Set PC to configuration memory so the test row (ID locations) are
		 * erased. */
		this->write_command(COMMAND_LOAD_CONFIG);
		this->io->shift_bits_out(0x7ffe, 16);
		this->io->usleep(1);

		this->write_command(COMMAND_CHIP_ERASE);
		this->io->usleep(this->erase_time);

		this->pic_off();
	} catch(std::exception& e) {
		this->pic_off();
		throw;
	}
}


void Pic16f87xADevice::bulk_erase(void) {
	this->disable_codeprotect();
}


/* ignores program_count and program_multiplier. no verify is done on data */
void Pic16f87xADevice::write_program_memory(DataBuffer& buf, long base) {
	unsigned int offset;

	if((base % 8) != 0) {
		THROW_ERROR(runtime_error,
		  "Base address doesn't fall on 3-bit boundary, 0x%04lx", base);
	}

	try {
		for(offset=0; offset < this->codesize; offset++) {
			progress(base+offset);

			this->write_prog_data((uint32_t)buf[base+offset]);
			/* 8 words loaded, now program it. */
			if(((offset+base) % 8) == 7) {
				this->write_command(COMMAND_BEGIN_PROG);
				this->io->usleep(this->program_time);
				if(this->flags & PIC_REQUIRE_EPROG)
					this->write_command(COMMAND_END_PROG);
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


void Pic16f87xADevice::write_config_word(uint32_t data) {
	this->write_prog_data(data);
	this->write_command(COMMAND_BEGIN_PROG);
	this->io->usleep(this->program_time);

	if((read_prog_data() & this->config_mask) != (data & this->config_mask))
		throw runtime_error("Couldn't write the configuration word");
}
