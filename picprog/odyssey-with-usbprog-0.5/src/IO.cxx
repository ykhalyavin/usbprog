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
 * $Id: IO.cxx,v 1.3 2006/06/07 19:54:29 gaufille Exp $
 */
using namespace std;

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdexcept>
#include "config.h"
#include "IO.h"
#ifdef ENABLE_LINUX_PPDEV
#  include "LinuxPPDevIO.h"
#endif
#ifdef ENABLE_DIRECTIO
#  include "DirectPPIO.h"
#endif
#include "Util.h"


IO *IO::acquire(char *name, int port) {
	IO *io;

#ifdef ENABLE_LINUX_PPDEV
	if(strcasecmp(name, "LinuxPPDev") == 0) {
		io = new LinuxPPDevIO(port);
	} else
#endif
#ifdef ENABLE_DIRECTIO
	if(strcasecmp(name, "DirectPP") == 0) {
		io = new DirectPPIO(port);
	} else
#endif
	{
		throw runtime_error("Unknown IO driver selected");
	}
	io->off();
	return io;
}


IO::IO(int port) {
	long default_delay, additional_delay;
	struct signal_delays tmp_delays;

	/* Read the signal delay values */
	if(! config->get_integer("signaldelay", "default", &default_delay)) {
		default_delay = 0;
	}

	if(! config->get_integer("signaldelay", "additional", &additional_delay)) {
		additional_delay = 0;
	}

	read_signaldelay("clk", &(this->clk_delays), default_delay, additional_delay);
	read_signaldelay("wdata", &tmp_delays, default_delay, additional_delay);
	this->data_delays.high_to_low = tmp_delays.high_to_low;
	this->data_delays.low_to_high = tmp_delays.low_to_high;
	read_signaldelay("rdata", &tmp_delays, default_delay, additional_delay);
	this->data_delays.read = tmp_delays.read;

	read_signaldelay("pwr", &(this->pwr_delays), default_delay, additional_delay);
	read_signaldelay("vpp", &(this->vpp_delays), default_delay, additional_delay);
	read_signaldelay("lvp", &(this->lvp_delays), default_delay, additional_delay);
}


IO::~IO() {
}


void IO::read_signaldelay(const char *name, struct signal_delays *delays,
  nanotime_t default_delay, nanotime_t additional_delay)
{
	char keybuf[32];
	long value;

	/* Get the input delay */
	snprintf(keybuf, sizeof(keybuf), "read_%s", name);
	if(config->get_integer("signaldelay", keybuf, &value)) {
		delays->read = value;
	} else if(config->get_integer("signaldelay", "read", &value)) {
		delays->read = value;
	} else {
		delays->read = default_delay;
	}
	delays->read += additional_delay;

	/* Get the output delays */
	snprintf(keybuf, sizeof(keybuf), "set_%s", name);
	if(config->get_integer("signaldelay", keybuf, &value)) {
		delays->high_to_low = value;
		delays->low_to_high = value;
	} else if(config->get_integer("signaldelay", "set", &value)) {
		delays->high_to_low = value;
		delays->low_to_high = value;
	} else {
		delays->high_to_low = default_delay;
		delays->low_to_high = default_delay;
	}

	snprintf(keybuf, sizeof(keybuf), "set_%s_hl", name);
	if(config->get_integer("signaldelay", keybuf, &value)) {
		delays->high_to_low = value;
	}
	delays->high_to_low += additional_delay;

	snprintf(keybuf, sizeof(keybuf), "set_%s_lh", name);
	if(config->get_integer("signaldelay", keybuf, &value)) {
		delays->low_to_high = value;
	}
	delays->low_to_high += additional_delay;
}


void IO::pre_read_delay(struct signal_delays *delays) {
	microtime_t us;

	/* Convert nanoseconds to microseconds, rounding up */
	us = (delays->read + 999) / 1000;
	this->usleep(us);
}


void IO::post_set_delay(struct signal_delays *delays, bool prev, bool current) {
	microtime_t us;

	/* Convert nanoseconds to microseconds, rounding up */
	if((prev == false) && (current == true)) {
		us = (delays->low_to_high + 999) / 1000;
	} else if((prev == true) && (current == false)) {
		us = (delays->high_to_low + 999) / 1000;
	} else {
		return;
	}
	this->usleep(us);
}


void IO::off(void) {
	this->set_vpp(false);
	this->set_clk(false);
	this->set_data(false);
	this->set_pwr(false);
	this->set_lvp(false);
}


void IO::usleep(microtime_t us) {
	struct timeval now, later;

	if(us == 0)
		return;

	if(us >= 10000) {	/* 10ms */
		later.tv_sec = us / 1000000;
		later.tv_usec = us % 1000000;
		select(0, NULL, NULL, NULL, &later);
		return;
	}

	/* Busy-wait for SPEED */
	gettimeofday(&later, NULL);
	later.tv_sec += us / 1000000;
	later.tv_usec += us % 1000000;
	while(later.tv_usec > 1000000) {
		later.tv_usec -= 1000000;
		later.tv_sec += 1;
	}
	while(1) {
		gettimeofday(&now, NULL);
		if((now.tv_sec > later.tv_sec) ||
		  ((now.tv_sec == later.tv_sec) && (now.tv_usec > later.tv_usec)))
			break;
	}
}


void IO::shift_bits_out(uint32_t bits, unsigned int numbits, microtime_t tset,
  microtime_t thold)
{
	while(numbits > 0) {
		this->set_clk(true);
		this->set_data(bits & 0x01);
		this->usleep(tset);                      /* Delay for data setup time */
		this->set_clk(false);                    /* Falling edge */
		this->usleep(thold);                     /* Delay for data hold time */
		bits >>= 1;
		numbits--;
	}
}


uint32_t IO::shift_bits_in(unsigned int numbits, microtime_t tdly,
  microtime_t tlow)
{
	uint32_t data, mask;

	data = 0;
	mask = 0x00000001;
	this->set_data(true);
	while(numbits > 0) {
		this->set_clk(true);
		this->usleep(tdly);
		if(this->read_data())
			data |= mask;
		this->set_clk(false);
		this->usleep(tdly);
		mask <<= 1;
		numbits--;
	}
	this->set_data(false);
	return data;
}
