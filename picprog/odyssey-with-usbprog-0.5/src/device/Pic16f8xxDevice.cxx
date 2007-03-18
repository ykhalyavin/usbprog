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
 * $Id: Pic16f8xxDevice.cxx,v 1.3 2006/12/31 14:57:35 gaufille Exp $
 */
using namespace std;

#include <stdio.h>
#include <stdexcept>
#include "PicDevice.h"
#include "Util.h"


Pic16f8xxDevice::Pic16f8xxDevice(char *name) : Pic16Device(name) {
	this->program_time += 100;
}


Pic16f8xxDevice::~Pic16f8xxDevice() {
}


void Pic16f8xxDevice::bulk_erase(void) {
	try {
		this->set_program_mode();

		/* This clears program memory and the config word. */
		this->write_command(COMMAND_LOAD_CONFIG);
		this->io->shift_bits_out(0x7ffe, 16);
		this->io->usleep(1);
		for(int i=0; i<7; i++)
			this->write_command(COMMAND_INC_ADDRESS);
		this->write_command(0x01);	/* Bulk Erase Setup1 */
		this->write_command(0x07);	/* Bulk Erase Setup2 */
		this->write_command(COMMAND_BEGIN_PROG);
		this->io->usleep(this->erase_time);
		this->write_command(0x01);
		this->write_command(0x07);

		if(this->flags & PIC_FEATURE_EEPROM) {
			/* This clears the data EEPROM */
			this->write_ee_data(0x3fff);
			this->write_command(0x01);
			this->write_command(0x07);
			this->write_command(COMMAND_BEGIN_PROG);
			this->io->usleep(this->erase_time);
			this->write_command(0x01);
			this->write_command(0x07);
		}

		this->pic_off();
	} catch(std::exception& e) {
		this->pic_off();
		throw;
	}
}


void Pic16f8xxDevice::disable_codeprotect(void) {
	try {
		this->set_program_mode();

		this->write_command(COMMAND_LOAD_CONFIG);
		this->io->shift_bits_out(0x7ffe, 16);
		this->io->usleep(1);
		for(int i=0; i<7; i++)
			this->write_command(COMMAND_INC_ADDRESS);
		this->write_command(0x01);	/* Bulk Erase Setup1 */
		this->write_command(0x07);	/* Bulk Erase Setup2 */
		this->write_command(COMMAND_BEGIN_PROG);
		this->io->usleep(this->erase_time);
		this->write_command(0x01);
		this->write_command(0x07);

		this->pic_off();
	} catch(std::exception& e) {
		this->pic_off();
		throw;
	}
}
