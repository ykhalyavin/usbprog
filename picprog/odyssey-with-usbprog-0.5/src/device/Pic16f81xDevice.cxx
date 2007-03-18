/* Copyright (C) 2002-2004  Mark Andrew Aikens <marka@desert.cx>
 * Copyright (C) 2006       Andrew M. Bishop <amb@gedanken.demon.co.uk>
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
 * $Id: Pic16f81xDevice.cxx,v 1.2 2006/12/31 14:57:35 gaufille Exp $
 */
using namespace std;

#include <stdio.h>
#include <stdexcept>
#include "PicDevice.h"
#include "Util.h"


Pic16f81xDevice::Pic16f81xDevice(char *name) : Pic16Device(name) {
}


Pic16f81xDevice::~Pic16f81xDevice() {
}


void Pic16f81xDevice::erase(void) {

	try {
		/* Perform a Chip erase */
		/*   Set PC to config memory */
		/*   Do a "Bulk Erase Program Memory" command. */
		/*   Do a "Begin Programming" command. */
		/*   Wait erase_time to complete bulk erase. */
		this->set_program_mode();
		this->write_command(COMMAND_LOAD_CONFIG);
		this->io->shift_bits_out(0x7ffe, 16);
		this->io->usleep(1);
		this->write_command(COMMAND_CHIP_ERASE);
		this->io->usleep(10000); // Internally timed, must be 8 ms, allow 10 ms
		this->pic_off();
	} catch(std::exception& e) {
		this->pic_off();
		throw;
	}
}


void Pic16f81xDevice::write_program_memory(DataBuffer& buf, long base) {
	unsigned int offset;

	for(offset=0; offset < this->codesize; offset+=4) {
		progress(base+offset);

		/* Skip blank locations to save time (unable to perform step by step verification) */
		if(buf.isblank(base+offset+0) && buf.isblank(base+offset+1) &&
                   buf.isblank(base+offset+2) && buf.isblank(base+offset+3)) {
                         this->write_command(COMMAND_INC_ADDRESS);
                         this->write_command(COMMAND_INC_ADDRESS);
                         this->write_command(COMMAND_INC_ADDRESS);
                } else {

                         /* Unable to perform step by step verification */
                         this->program4_cycle((uint32_t*)&buf[base+offset]);
                }

		this->write_command(COMMAND_INC_ADDRESS);
		this->progress_count+=4;
	}
}


void Pic16f81xDevice::write_data_memory(DataBuffer& buf, long base) {
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
				this->write_command(COMMAND_BEGIN_PROG_ONLY);
				this->io->usleep(this->program_time);
                                this->write_command(COMMAND_END_PROG_16F81X);
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


void Pic16f81xDevice::write_id_memory(DataBuffer& buf, long base) {
	unsigned int offset;

        for(offset=0; offset < 4; offset+=4) {
		progress(base+offset);

		/* Skip blank locations to save time (unable to perform step by step verification) */
		if(buf.isblank(base+offset+0) && buf.isblank(base+offset+1) &&
                   buf.isblank(base+offset+2) && buf.isblank(base+offset+3)) {
                         this->write_command(COMMAND_INC_ADDRESS);
                         this->write_command(COMMAND_INC_ADDRESS);
                         this->write_command(COMMAND_INC_ADDRESS);
                } else {

                         /* Unable to perform step by step verification */
                         this->program4_cycle((uint32_t*)&buf[base+offset]);
                }

		this->write_command(COMMAND_INC_ADDRESS);
		this->progress_count+=4;
	}
}


bool Pic16f81xDevice::program_cycle(uint32_t data, uint32_t mask) {
	this->write_prog_data(data);
	this->write_command(COMMAND_BEGIN_PROG_ONLY);
	this->io->usleep(this->program_time);
	if(this->flags & PIC_REQUIRE_EPROG)
		this->write_command(COMMAND_END_PROG_16F81X);

	if((read_prog_data() & mask) == (data & mask)) return true;
	return false;
}


bool Pic16f81xDevice::program4_cycle(uint32_t data[4]) {
	this->write_prog_data(data[0]);
        this->write_command(COMMAND_INC_ADDRESS);
	this->write_prog_data(data[1]);
        this->write_command(COMMAND_INC_ADDRESS);
	this->write_prog_data(data[2]);
        this->write_command(COMMAND_INC_ADDRESS);
	this->write_prog_data(data[3]);

	this->write_command(COMMAND_BEGIN_PROG_ONLY);
	this->io->usleep(this->program_time);
        this->write_command(COMMAND_END_PROG_16F81X);

        /* Unable to perform step by step verification */
	return true;
}
