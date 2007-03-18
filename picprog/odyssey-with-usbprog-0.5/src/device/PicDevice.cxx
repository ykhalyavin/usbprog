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
 * $Id: PicDevice.cxx,v 1.5 2006/12/31 14:57:35 gaufille Exp $
 */
using namespace std;

#include <stdio.h>
#include <stdexcept>
#include "config.h"
#include "PicDevice.h"
#include "IO.h"
#include "Util.h"


/* XXX: this never gets freed. */
ConfigFile *pic_config;


void PicDevice::list(vector<string> *v) {
	vector<string> pics;
	bool experimental;

	if(pic_config == NULL)
		pic_config = new ConfigFile("pic.conf", PACKAGE, DATADIR "/pic.conf");

	pics = pic_config->get_sections();
	for(unsigned long i=0; i < pics.size(); i++) {
		string name = pics[i];

		if(! pic_config->get_boolean(name, "experimental", &experimental)) {
			experimental = false;
		}
		if(experimental) name.insert(0, "*");
		v->push_back(name);
	}
}


Device *PicDevice::load(char *name) {
	if(pic_config == NULL)
		pic_config = new ConfigFile("pic.conf", PACKAGE, DATADIR "/pic.conf");

	if(Util::regex_match("^PIC16F87[3467]A", name)) {
		return new Pic16f87xADevice(name);
	} else if(Util::regex_match("^PIC16F81[89]", name)) {
		return new Pic16f81xDevice(name);
	} else if(Util::regex_match("^PIC16F8", name) ||
	         (strcmp(name, "PIC16C84") == 0)) {
		return new Pic16f8xxDevice(name);
	} else if(Util::regex_match("^PIC16F7[3467]$", name)) {
		return new Pic16f7xDevice(name);
	} else if(Util::regex_match("^PIC16F6.*", name)) {
		return new Pic16f6xxDevice(name);
	} else if(Util::regex_match("^PIC12F6.*", name)) {
		return new Pic12f6xxDevice(name);
	} else if(Util::regex_match("^PIC16.*", name)) {
		return new Pic16Device(name);
	} else if(Util::regex_match("^PIC18F((2..[05])|(2.21)|(4..[05])|(4.21))", name)) {
		return new Pic18f2xx0Device(name);
    } else if(Util::regex_match("^PIC18.*", name)) {
        return new Pic18Device(name);
	}
	return NULL;
}


PicDevice::PicDevice(char *name) : Device(name) {
	char memtypebuf[10];
	long value;

	this->flags = 0;
	/* Fill in this PIC's common attributes */
	if(! pic_config->get_string(name, "memtype", memtypebuf, sizeof(memtypebuf)))
		throw runtime_error("PIC device is missing memtype configuration parameter");
	if(strcmp(memtypebuf, "eprom") == 0) {
		this->memtype = MEMTYPE_EPROM;
		this->flags |= PIC_REQUIRE_EPROG;
	} else if(strcmp(memtypebuf, "flash") == 0) {
		this->memtype = MEMTYPE_FLASH;
	} else if(strcmp(memtypebuf, "flashe") == 0) {
		this->memtype = MEMTYPE_FLASH;
		this->flags |= PIC_REQUIRE_EPROG;
	} else {
		throw runtime_error("PIC device has an unknown memory type");
	}

	if(! pic_config->get_integer(name, "wordsize", &value))
		throw runtime_error("PIC device has incomplete configuration entry");
	this->wordsize = value;

	if(! pic_config->get_integer(name, "codesize", &value))
		throw runtime_error("PIC device has incomplete configuration entry");
	this->codesize = value;

	if(pic_config->get_integer(name, "eesize", &value) && (value > 0)) {
		this->flags |= PIC_FEATURE_EEPROM;
		this->eesize = value;
	} else {
		this->eesize = 0;
	}

	/* Calculate the word mask */
	this->wordmask = 0;
	for(int i=0; i<this->wordsize; i++) {
		this->wordmask <<= 1;
		this->wordmask |= 0x0001;
	}

	/* Get the device ID value if this device supports it. */
	if(pic_config->get_integer(name, "deviceid", &value)) {
		this->deviceid = value;
		if(pic_config->get_integer(name, "deviceidmask", &value)) {
			this->deviceidmask = value;
			this->flags |= PIC_HAS_DEVICEID;
		}
	}

	/* Read programming parameters with defaults for an EPROM device */
	READ_PIC_CONFIG_ENTRY_DEFAULT("progcount", program_count, 25);
	READ_PIC_CONFIG_ENTRY_DEFAULT("progmult", program_multiplier, 3);
	READ_PIC_CONFIG_ENTRY_DEFAULT("progtime", program_time, 100);
	READ_PIC_CONFIG_ENTRY_DEFAULT("erasetime", erase_time, 0);
}


PicDevice::~PicDevice() {
}


bool PicDevice::program_cycle(uint32_t data, uint32_t mask) {
	this->write_prog_data(data);
	this->write_command(COMMAND_BEGIN_PROG);
	this->io->usleep(this->program_time);
	if(this->flags & PIC_REQUIRE_EPROG)
		this->write_command(COMMAND_END_PROG);

	if((read_prog_data() & mask) == (data & mask)) return true;
	return false;
}


void PicDevice::set_program_mode(void) {
	/* Power up the PIC and put it in program/verify mode */
	this->io->set_lvp(false);	/* Disable LVP mode */
	this->io->set_clk(false);	/* Set RB6 low */
	this->io->set_data(false);/* Set RB7 low */
	this->io->set_pwr(true);
	this->io->usleep(1000);
	this->io->set_vpp(true);	/* Raise Vpp while RB6 & RB7 are low */
	this->io->usleep(1000);	/* Wait a bit */
}


bool PicDevice::check(void) {
	uint32_t devid;

	if(! (this->flags & PIC_HAS_DEVICEID)) {
		/* Device doesn't have a device ID to check */
		return true;
	}

	try {
		this->set_program_mode();
		devid = read_deviceid();
		this->pic_off();

		/* Wait a bit to make sure program mode is off before continuing
		 * with other operations on the device. */
		this->io->usleep(1000);
	} catch(std::exception& e) {
		this->pic_off();
		throw;
	}

	if((devid & this->deviceidmask) == (this->deviceid & this->deviceidmask)) {
		/* Keep the chip rev in case it's needed later */
		this->deviceid = devid;
		printf("Chip Rev: 0x%lx\n", (unsigned long)
		  (devid & ~this->deviceidmask));
		return true;
	}

	/* XXX: there's no way to pass back an error string without using
	 * exceptions! */
	THROW_ERROR(runtime_error,
	  "Device ID 0x%lx is wrong (expected 0x%lx, mask 0x%lx)",
	  (unsigned long)devid, (unsigned long)this->deviceid,
	  (unsigned long)this->deviceidmask);
	return false;
}


uint32_t PicDevice::read_deviceid(void) {
	fprintf(stderr, "Warning: read_deviceid is not implemented for this device.\n");
	/* Fake it out. */
	return (this->deviceid & this->deviceidmask);
}


void PicDevice::pic_off(void) {
	/* Shut everything down */
	this->io->set_clk(false);
	this->io->set_data(false);
	this->io->set_vpp(false);
	this->io->set_pwr(false);
}


void PicDevice::write_command(uint32_t command) {
	this->io->shift_bits_out(command, 6);
	this->io->usleep(1);
}


void PicDevice::write_prog_data(uint32_t data) {
	data = (data & this->wordmask) << 1;
	this->write_command(COMMAND_LOAD_PROG_DATA);
	this->io->shift_bits_out(data, 16);
	this->io->usleep(1);
}


uint32_t PicDevice::read_prog_data(void) {
	uint32_t data;

	this->write_command(COMMAND_READ_PROG_DATA);
	data = this->io->shift_bits_in(16);
	this->io->usleep(1);
	return (data >> 1) & this->wordmask;
}

void PicDevice::set_config_default(DataBuffer& buf)
{
	/* On most PICs, the configuration memory is only = 0xffff but it is not always the case */
	/* TODO should be done through the pic.conf configuration */
}
