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

#define VID 0x1781
#define PID 0x0c62

static int
usbprog_init( cable_t *cable )
{
	
	return 0;
}

static void
usbprog_clock( cable_t *cable, int tms, int tdi )
{
	tms = tms ? 1 : 0;
	tdi = tdi ? 1 : 0;

	//simpleport_set_port(sp_handle,(PARAM_TRST(cable) << nTRST) | (0 << TCK) | (tms << TMS) | (tdi << TDI) ,0xFF);
	//simpleport_set_port(sp_handle,(PARAM_TRST(cable) << nTRST) | (1 << TCK) | (tms << TMS) | (tdi << TDI) ,0xFF);

}

static int
usbprog_get_tdo( cable_t *cable )
{
	//simpleport_set_port(sp_handle, (PARAM_TRST(cable) << nTRST) | (0 << TCK),(1<<nTRST)|(1<<TCK));
	//return (simpleport_get_port(sp_handle) >> TDO) & 1;
	return 0;
}

static int
usbprog_set_trst( cable_t *cable, int trst )
{
	PARAM_TRST(cable) = trst ? 1 : 0;

	//simpleport_set_port(sp_handle, (PARAM_TRST(cable) << nTRST),0xff);
	return PARAM_TRST(cable);
}

cable_driver_t usbprog_cable_driver = {
	"USBPROG",
	N_("JTAG Adaptor (http://www.embedded-projects.net/usbprorg)"),
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



