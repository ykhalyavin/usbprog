/*
 * $Id: usbprog.c,v 1.9 2004/10/17 15:15:59 telka Exp $
 *
 * Macraigor Wiggler JTAG Cable Driver
 * Copyright (C) 2002, 2003 ETC s.r.o.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 * Documentation:
 * [1] http://www.ocdemon.net/
 * [2] http://jtag-arm9.sourceforge.net/hardware.html
 *
 */

#include "sysdep.h"

#include "cable.h"
#include "parport.h"
#include "chain.h"

#include "generic.h"

#include <stdio.h>

#include <usbprogjtag.h>

#define VID 0x1781
#define PID 0x0c62

static int
usbprog_init( cable_t *cable )
{
	printf("usbprog_init\n");	
	cable->usbprogjtag_handle = usbprog_jtag_open();
	if(!cable->usbprogjtag_handle){
	  return -1;
	}
	usbprog_jtag_init(cable->usbprogjtag_handle);
	return 0;
}

static void
usbprog_clock( cable_t *cable, int tms, int tdi )
{
	tms = tms ? 1 : 0;
	tdi = tdi ? 1 : 0;

	usbprog_jtag_write_slice(cable->usbprogjtag_handle,
	  (PARAM_TRST(cable) << PIN_TRST) | (0 << PIN_TCK) | (tms << PIN_TMS) | (tdi << PIN_TDI));

	usbprog_jtag_write_slice(cable->usbprogjtag_handle,
	  (PARAM_TRST(cable) << PIN_TRST) | (1 << PIN_TCK) | (tms << PIN_TMS) | (tdi << PIN_TDI));

}

static int
usbprog_get_tdo( cable_t *cable )
{
   	usbprog_jtag_write_slice(cable->usbprogjtag_handle,
	  (PARAM_TRST(cable) << PIN_TRST) | (0 << PIN_TCK) );
	return (usbprog_jtag_get_port(cable->usbprogjtag_handle) >> PIN_TDO) & 1;
}

static int
usbprog_set_trst( cable_t *cable, int trst )
{
	PARAM_TRST(cable) = trst ? 1 : 0;
	usbprog_jtag_set_bit(cable->usbprogjtag_handle,3,PARAM_TRST(cable));
	return PARAM_TRST(cable);
}

cable_driver_t usbprog_cable_driver = {
	"USBPROG",
	N_("JTAG Interface (http://www.embedded-projects.net/usbprorg)"),
	generic_connect,
	generic_disconnect,
	generic_cable_free,
	usbprog_init,
	generic_done,
	usbprog_clock,
	usbprog_get_tdo,
	usbprog_set_trst,
	generic_get_trst
};



