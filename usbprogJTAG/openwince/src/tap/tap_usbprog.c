/*
 * $Id: tap.c,v 1.11 2003/08/19 08:42:20 telka Exp $
 *
 * Copyright (C) 2002 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 */

#include "sysdep.h"

#include <stdio.h>

#include "register.h"
#include "tap.h"
#include "state.h"
#include "chain.h"

void
tap_reset( chain_t *chain )
{
	//printf("tap_reset\n");
	tap_state_reset( chain );

	chain_clock( chain, 1, 0 );
	chain_clock( chain, 1, 0 );
	chain_clock( chain, 1, 0 );
	chain_clock( chain, 1, 0 );
	chain_clock( chain, 1, 0 );				/* Test-Logic-Reset */

	chain_clock( chain, 0, 0 );				/* Run-Test/Idle */
}

void
tap_shift_register( chain_t *chain, const tap_register *in, tap_register *out, int exit )
{
	int i;
/*
	if(out)
		printf("tap_shift_register len%i %i\n", in->len, out->len);
	else
		printf("tap_shift_register len%i\n", in->len);
*/
	if (!(tap_state( chain ) & TAPSTAT_SHIFT))
		printf( _("%s: Invalid state: %2X\n"), "tap_shift_register", tap_state( chain ) );

	/* Capture-DR, Capture-IR, Shift-DR, Shift-IR, Exit2-DR or Exit2-IR state */
	if (tap_state( chain ) & TAPSTAT_CAPTURE)
		chain_clock( chain, 0, 0 );	/* save last TDO bit :-) */

	//char buf[(in->len/8)+1];	
#if 0
	/* build send buffer */
	//char buf[255];
	for (i = 0; i < in->len; i++) {
		if(in->data[i]==1)
			buf[(i)/8] |= 1 << ((i) % 8);
		else
			buf[(i)/8] &= ~(1 << ((i) % 8));
	}

	usbprog_jtag_write_and_read(chain->cable->usbprogjtag_handle, buf, in->len);
	
	for (i = 0; i < in->len; i++) {
		if ((buf[(i)/8] >> ((i) % 8)) & 0x1) //tdi 1
			out->data[i] = 1;
		else
			out->data[i] = 0;
	}
#endif
#if 1 
	i =0;
	if(in->len<=1){
	  if (out && (i < out->len))
			out->data[i] = cable_get_tdo( chain->cable );
		chain_clock( chain, (exit && ((i + 1) == in->len)) ? 1 : 0, in->data[i] );

	} else {

/* geht
	for (i=0; i < (in->len-1); i++) {
		//if (out && (i < out->len))
	//		out->data[i] = cable_get_tdo( chain->cable );
		//chain_clock( chain, 0, in->data[i] );
		if (out && (i < out->len))
		usbprog_jtag_tap_shift_register(chain->cable->usbprogjtag_handle,&in->data[i],1,&out->data[i],1);
		else
		usbprog_jtag_tap_shift_register(chain->cable->usbprogjtag_handle,&in->data[i],1,NULL,0);
		
		//if (out && (i < out->len))
		//  out->data[i] = &in->data[i];

	} 
	*/
   
	if (out)
	usbprog_jtag_tap_shift_register(chain->cable->usbprogjtag_handle,in->data,in->len-1,out->data,out->len-1);
	else
	usbprog_jtag_tap_shift_register(chain->cable->usbprogjtag_handle,in->data,in->len-1,NULL,0);

	i = in->len-1;

	if (out && (i < out->len))
		out->data[i] = cable_get_tdo( chain->cable );
	chain_clock( chain, (exit && ((i + 1) == in->len)) ? 1 : 0, in->data[i] );
	
	}
#endif

#if 0
/* originale */
	for (i = 0; i < in->len; i++) {
		if (out && (i < out->len))
			out->data[i] = cable_get_tdo( chain->cable );
		chain_clock( chain, (exit && ((i + 1) == in->len)) ? 1 : 0, in->data[i] );	/* Shift (& Exit1) */
	}
#endif

	/* Shift-DR, Shift-IR, Exit1-DR or Exit1-IR state */
	if (exit) {
		//printf("exit\n");
		chain_clock( chain, 1, 0 );	/* Update-DR or Update-IR */
		chain_clock( chain, 0, 0 );	/* Run-Test/Idle */
	}

}

void
tap_capture_dr( chain_t *chain )
{
	//printf("tap_capture_dr\n");
	if ((tap_state( chain ) & (TAPSTAT_RESET | TAPSTAT_IDLE)) != TAPSTAT_IDLE)
		printf( _("%s: Invalid state: %2X\n"), "tap_capture_dr", tap_state( chain ) );

	/* Run-Test/Idle or Update-DR or Update-IR state */
	chain_clock( chain, 1, 0 );		/* Select-DR-Scan */
	chain_clock( chain, 0, 0 );		/* Capture-DR */
}

void
tap_capture_ir( chain_t *chain )
{
	//printf("tap_capture_ir\n");
	if ((tap_state( chain ) & (TAPSTAT_RESET | TAPSTAT_IDLE)) != TAPSTAT_IDLE)
		printf( _("%s: Invalid state: %2X\n"), "tap_capture_ir", tap_state( chain ) );

	/* Run-Test/Idle or Update-DR or Update-IR state */
	chain_clock( chain, 1, 0 );		/* Select-DR-Scan */
	chain_clock( chain, 1, 0 );		/* Select-IR-Scan */
	chain_clock( chain, 0, 0 );		/* Capture-IR */
}
