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
 * $Id: Device.cxx,v 1.2 2005/11/25 22:16:34 uid20104 Exp $
 */
using namespace std;

#include <string.h>
#include <stdexcept>
#include "Device.h"
#include "device/PicDevice.h"


vector<string> *Device::list(void) {
	vector<string> *v = new vector<string>;
	PicDevice::list(v);
	return v;
}


Device *Device::load(char *name) {
	Device *d = NULL;

	try {
		if(strncasecmp(name, "PIC", 3) == 0) {
			d = PicDevice::load(name);
		}
	} catch(std::exception& e) {
		d = NULL;
	}
	return d;
}


Device::Device(char *name) {
	this->wordsize = 8;
	this->set_iodevice(NULL);
	this->set_progress_cb(NULL);
	this->progress_count = 0;
	this->progress_total = 1;
	this->name = string(name);
}


Device::~Device() {
}


string Device::get_name(void) {
	return this->name;
}


IntPairVector& Device::get_mmap(void) {
	return this->memmap;
}


int Device::get_wordsize(void) {
	return this->wordsize;
}


void Device::set_iodevice(IO *iodev) {
	this->io = iodev;
}


void Device::set_progress_cb(bool (*cb)(void *data, long addr, int percent),
  void *data) {
	this->progress_cb = cb;
	this->progress_cb_data = data;
}


bool Device::progress(unsigned long addr) {
	if(this->progress_cb) {
		return this->progress_cb(this->progress_cb_data, addr,
		  (100*this->progress_count)/this->progress_total);
	}
	return true;
}
