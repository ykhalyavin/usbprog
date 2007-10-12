/*
 * $Id: usb.c,v 1.7 2003/08/19 09:05:25 telka Exp $
 *
 * Direct Parallel Port Connection Driver
 * Copyright (C) 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 * Ported to NetBSD/i386 by Jachym Holecek <freza@psi.cz>, 2003.
 *
 */

#include "sysdep.h"

#include <stdlib.h>
#include <string.h>

#include "parport.h"
#include "cable.h"

#if defined(HAVE_IOPERM) || defined(HAVE_I386_SET_IOPERM)

#if defined(HAVE_IOPERM)
#include <sys/io.h>
#elif defined(HAVE_I386_SET_IOPERM)
#include <sys/types.h>
#include <machine/sysarch.h>
#include <err.h>
#endif


parport_driver_t usb_parport_driver;

typedef struct port_node_t port_node_t;

struct port_node_t {
	parport_t *port;
	port_node_t *next;
};

static port_node_t *ports = NULL;		/* usb parallel ports */

typedef struct {
	unsigned int port;
} usb_params_t;

static parport_t *
usb_parport_alloc( unsigned int port )
{
	usb_params_t *params = malloc( sizeof *params );
	parport_t *parport = malloc( sizeof *parport );
	port_node_t *node = malloc( sizeof *node );

	if (!node || !parport || !params) {
		free( node );
		free( parport );
		free( params );
		return NULL;
	}

	params->port = port;

	parport->params = params;
	parport->driver = &usb_parport_driver;
	parport->cable = NULL;

	node->port = parport;
	node->next = ports;

	ports = node;

	return parport;
}

static void
usb_parport_free( parport_t *port )
{
	port_node_t **prev;

	for (prev = &ports; *prev; prev = &((*prev)->next))
		if ((*prev)->port == port)
			break;

	if (*prev) {
		port_node_t *pn = *prev;
		*prev = pn->next;
		free( pn );
	}

	free( port->params );
	free( port );
}

static cable_t *
usb_connect( const char **par, int parnum )
{
	int i;
	unsigned int port;
	port_node_t *pn = ports;
	parport_t *parport;
	cable_t *cable;

	if (parnum != 2) {
		printf( _("Syntax error!\n") );
		return NULL;
	}

	if ((sscanf( par[0], "0x%x", &port ) != 1) && (sscanf( par[0], "%d", &port ) != 1)) {
		printf( _("Invalid port address!\n") );
		return NULL;
	}

	while (pn)
		for (pn = ports; pn; pn = pn->next) {
			unsigned int aport;

			aport = ((usb_params_t*) pn->port->params)->port;
			if (abs( aport - port ) < 3) {
				printf( _("Disconnecting %s from parallel port at 0x%x\n"), _(pn->port->cable->driver->description), aport );
				pn->port->cable->driver->disconnect( pn->port->cable );
				break;
			}
		}

	if (strcmp( par[1], "none" ) == 0) {
		printf( _("Changed cable to 'none'\n") );
		return NULL;
	}

	for (i = 0; cable_drivers[i]; i++)
		if (strcmp( par[1], cable_drivers[i]->name ) == 0)
			break;

	if (!cable_drivers[i]) {
		printf( _("Unknown cable: %s\n"), par[1] );
		return NULL;
	}

	printf( _("Initializing %s on parallel port at 0x%x\n"), _(cable_drivers[i]->description), port );

	parport = usb_parport_alloc( port );
	if (!parport) {
		printf( _("%s(%d) Out of memory.\n"), __FILE__, __LINE__ );
		return NULL;
	}

	cable = cable_drivers[i]->connect( cable_drivers[i], parport );
	if (!cable)
		usb_parport_free( parport );

	return cable;
}

static int
usb_open( parport_t *parport )
{
	unsigned int port = ((usb_params_t *) parport->params)->port;
	return ((port + 3 <= 0x400) && ioperm( port, 3, 1 )) || ((port + 3 > 0x400) && iopl( 3 ));
}

static int
usb_close( parport_t *parport )
{
	unsigned int port = ((usb_params_t *) parport->params)->port;
	return (port + 3 <= 0x400) ? ioperm( port, 3, 0 ) : iopl( 0 );
}

static int
usb_set_data( parport_t *parport, uint8_t data )
{
	unsigned int port = ((usb_params_t *) parport->params)->port;
	outb( data, port );
	return 0;
}

static int
usb_get_data( parport_t *parport )
{
	unsigned int port = ((usb_params_t *) parport->params)->port;
	return inb( port );
}

static int
usb_get_status( parport_t *parport )
{
	unsigned int port = ((usb_params_t *) parport->params)->port;
	return inb( port + 1 ) ^ 0x80;		/* BUSY is inverted */
}

static int
usb_set_control( parport_t *parport, uint8_t data )
{
	unsigned int port = ((usb_params_t *) parport->params)->port;
	outb( data ^ 0x0B, port + 2 );		/* SELECT, AUTOFD, and STROBE are inverted */
	return 0;
}

parport_driver_t usb_parport_driver = {
	"usb",
	usb_connect,
	usb_parport_free,
	usb_open,
	usb_close,
	usb_set_data,
	usb_get_data,
	usb_get_status,
	usb_set_control
};

#endif /* defined(HAVE_IOPERM) || defined(HAVE_I386_SET_IOPERM) */
