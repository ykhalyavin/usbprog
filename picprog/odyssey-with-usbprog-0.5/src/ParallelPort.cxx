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
 * $Id: ParallelPort.cxx,v 1.2 2005/11/25 22:16:34 uid20104 Exp $
 */
using namespace std;

#include <stdio.h>
#include <stdexcept>
#include "ParallelPort.h"
#include "ConfigFile.h"
#include "Util.h"


#define READPINDATA(cname, prefix) \
	if(!config->get_integer("io", cname, &value)) \
		throw runtime_error("Missing I/O configuration parameter io." cname); \
	this->prefix##invert = 0; \
	if(value < 0) { \
		this->prefix##invert = 1; \
		value *= -1; \
	} \
	if((value > 25) || (value == 0) || (pin2reg[value-1] == -1)) \
		throw runtime_error("Invalid value for configuration parameter io." cname); \
	if(hw_invert[value-1]) \
		this->prefix##invert ^= 1; \
	this->prefix##reg = pin2reg[value-1]; \
	this->prefix##bit = pin2bit[value-1];


/* Mapping of parallel port pin #'s to I/O register offset. */
static signed char pin2reg[25] = {
	2, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 1, 2, 2,
	-1, -1, -1, -1, -1, -1, -1, -1
};

/* Mapping of parallel port pin #'s to I/O register bit positions. */
static signed char pin2bit[25] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 6, 7, 5, 4, 1, 3, 2, 3,
	-1, -1, -1, -1, -1, -1, -1, -1
};

/* Flags for each pin, indicating if the parallel port hardware inverts the
 * value. (BSY, STB, ALF, DSL) */
static signed char hw_invert[25] = {
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 0, 0
};


ParallelPort::ParallelPort(int port) : IO(port) {
	long value;

	READPINDATA("rdatapin", rdata);
	READPINDATA("wdatapin", wdata);
	READPINDATA("clkpin", clk);
	READPINDATA("vpppin", vpp);
	READPINDATA("pwrpin", pwr);

	/* The rdata2pin is optional */
	this->rdata2reg = -1;
	if(config->get_integer("io", "rdata2pin", &value) && (value != 0)) {
		READPINDATA("rdata2pin", rdata2);
	}

	/* The lvppin is optional */
	this->lvpreg = -1;
	if(config->get_integer("io", "lvppin", &value) && (value != 0)) {
		READPINDATA("lvppin", lvp);
	}
}


ParallelPort::~ParallelPort() {
}


void ParallelPort::set_clk(bool state) {
	bool old_state;

	old_state = this->get_clk();
	this->set_pin_state("clk", this->clkreg, this->clkbit, this->clkinvert,
	  state);
	this->post_set_delay(&(this->clk_delays), old_state, state);
}


bool ParallelPort::get_clk(void) {
	return this->get_pin_state("clk", this->clkreg, this->clkbit,
	  this->clkinvert);
}


void ParallelPort::set_data(bool state) {
	bool old_state;

	old_state = this->get_data();
	this->set_pin_state("wdata", this->wdatareg, this->wdatabit,
	  this->wdatainvert, state);
	this->post_set_delay(&(this->data_delays), old_state, state);
}


bool ParallelPort::get_data(void) {
	return this->get_pin_state("wdata", this->wdatareg, this->wdatabit,
	  this->wdatainvert);
}


bool ParallelPort::read_data(void) {
	bool data, data2;

	this->pre_read_delay(&(this->data_delays));
	data = this->get_pin_state("rdata", this->rdatareg, this->rdatabit,
	  this->rdatainvert);
	if(this->rdata2reg >= 0) {
		data2 = this->get_pin_state("rdata2", this->rdata2reg, this->rdata2bit,
		  this->rdata2invert);
		if(data != data2) {
			THROW_ERROR(runtime_error, "Discrepancy: rdata %d != rdata2 %d",
			  data, data2);
		}
	}
	return data;
}


void ParallelPort::set_vpp(bool state) {
	bool old_state;

	old_state = this->get_vpp();
	this->set_pin_state("vpp", this->vppreg, this->vppbit, this->vppinvert,
	  state);
	this->post_set_delay(&(this->vpp_delays), old_state, state);
}


bool ParallelPort::get_vpp(void) {
	return this->get_pin_state("vpp", this->vppreg, this->vppbit,
	  this->vppinvert);
}


void ParallelPort::set_pwr(bool state) {
	bool old_state;

	old_state = this->get_pwr();
	this->set_pin_state("pwr", this->pwrreg, this->pwrbit, this->pwrinvert,
	  state);
	this->post_set_delay(&(this->pwr_delays), old_state, state);
}


bool ParallelPort::get_pwr(void) {
	return this->get_pin_state("pwr", this->pwrreg, this->pwrbit,
	  this->pwrinvert);
}


void ParallelPort::set_lvp(bool state) {
	bool old_state;

	if(this->lvpreg >= 0) {
		old_state = this->get_lvp();
		this->set_pin_state("lvp", this->lvpreg, this->lvpbit, this->lvpinvert,
		  state);
		this->post_set_delay(&(this->lvp_delays), old_state, state);
	}
}


bool ParallelPort::get_lvp(void) {
	if(this->lvpreg >= 0) {
		return this->get_pin_state("lvp", this->lvpreg, this->lvpbit,
		  this->lvpinvert);
	} else {
		return false;
	}
}
