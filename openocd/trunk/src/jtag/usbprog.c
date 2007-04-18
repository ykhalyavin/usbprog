/***************************************************************************
 *   Copyright (C) 2005 by Dominic Rath                                    *
 *   Dominic.Rath@gmx.de                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "jtag.h"
#include "bitbang.h"

/* system includes */
#include <sys/io.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <usb.h>

/* low level command set
 */

// need for communication with usbprog
struct usb_dev_handle* usbprog_handle;


int usbprog_read(void);
void usbprog_write(int tck, int tms, int tdi);
void usbprog_reset(int trst, int srst);

int usbprog_speed(int speed);
int usbprog_register_commands(struct command_context_s *cmd_ctx);
int usbprog_init(void);
int usbprog_quit(void);

jtag_interface_t usbprog_interface = 
{
	.name = "usbprog",
	
	.execute_queue = bitbang_execute_queue,

	.support_pathmove = 0,

	.speed = usbprog_speed,	
	.register_commands = usbprog_register_commands,
	.init = usbprog_init,
	.quit = usbprog_quit,
};

bitbang_interface_t usbprog_bitbang =
{
	.read = usbprog_read,
	.write = usbprog_write,
	.reset = usbprog_reset
};

int usbprog_read(void)
{
	//return !!(*gpio_data_register & TDO_BIT);
	return 0;
}

void usbprog_write(int tck, int tms, int tdi)
{
	#if 0	
	if (tck)
		output_value |= TCK_BIT;
	else
		output_value &= TCK_BIT;
	
	if (tms)
		output_value |= TMS_BIT;
	else
		output_value &= TMS_BIT;
	
	if (tdi)
		output_value |= TDI_BIT;
	else
		output_value &= TDI_BIT;
	
	#endif
}

/* (1) assert or (0) deassert reset lines */
void usbprog_reset(int trst, int srst)
{

}

int usbprog_speed(int speed)
{
	return ERROR_OK;
}

int usbprog_register_commands(struct command_context_s *cmd_ctx)
{
	return ERROR_OK;
}

int usbprog_init(void)
{
	int ret;

	bitbang_interface = &usbprog_bitbang;	

	struct usb_bus *busses;
	struct usb_bus *bus;

	usb_init();
	usb_find_busses();
	usb_find_devices();

	busses = usb_get_busses();
	for (bus = busses; bus; bus = bus->next) {
		struct usb_device *dev;
		for (dev = bus->devices; dev; dev = dev->next){
			if(dev->descriptor.idVendor==0x0000 && dev->descriptor.idProduct==0x0c62 ) {
			        usbprog_handle = usb_open(dev);
				usb_set_configuration (usbprog_handle,1);
				usb_claim_interface(usbprog_handle,0);
				usb_set_altinterface(usbprog_handle,0);
				return ERROR_OK;
			}	
		}
	}
	return ERROR_JTAG_INIT_FAILED;
}

int usbprog_quit(void)
{
	usb_close(usbprog_handle);
	return ERROR_OK;
}
