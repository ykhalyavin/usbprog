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


#define VID 0x1781
#define PID 0x0c62


// pins from avr
#define TDO_BIT		6
#define TDI_BIT		5
#define TCK_BIT		7
#define TMS_BIT		0	

#define JTAG_SEQUENCE	1

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

struct timespec usbprog_zzzz;


jtag_interface_t usbprog_interface = 
{
	.name = "usbprog",
	
	.execute_queue = bitbang_execute_queue,

	.support_pathmove = 1,

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

// read tdo
int usbprog_read(void)
{
	//command message
	char buf[1];
	if(usb_bulk_read(usbprog_handle,0x83,buf,1,10)==1) {;
	  WARNING("usb successfull read");
	  if (buf[0] == 0)
	    return 0;
	  else
	    return 1;
	} else {
	  WARNING("usb timeout while reading");
	}

}

void usbprog_write(int tck, int tms, int tdi)
{
	char output_value;	
	output_value = 0x00;	
	
	if (tck) 
		output_value |= (1<<TCK_BIT);
	else
		output_value &= ~(1<<TCK_BIT);
	
	if (tms)
		output_value |= (1<<TMS_BIT);
	else
		output_value &= ~(1<<TMS_BIT);
	
	if (tdi)
		output_value |= (1<<TDI_BIT);
	else
		output_value &= ~(1<<TDI_BIT);

	WARNING("seq %02X",output_value);

	char cmd[2];
	cmd[0] = JTAG_SEQUENCE;
	cmd[1] = output_value;
	nanosleep(usbprog_zzzz);
	usb_bulk_write(usbprog_handle,2,cmd,2,10);
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
  
	usbprog_zzzz.tv_sec = 1;
  usbprog_zzzz.tv_nsec = 10000000;

	struct usb_bus *busses;
	struct usb_bus *bus;

	usb_init();
	usb_find_busses();
	usb_find_devices();

	busses = usb_get_busses();
	for (bus = busses; bus; bus = bus->next) {
		struct usb_device *dev;
		for (dev = bus->devices; dev; dev = dev->next){
			if(dev->descriptor.idVendor==VID && dev->descriptor.idProduct==PID ) {
			        usbprog_handle = usb_open(dev);
				usb_set_configuration (usbprog_handle,1);
				usb_claim_interface(usbprog_handle,0);
				usb_set_altinterface(usbprog_handle,0);
				WARNING("Find usbprog adapter!");
				return ERROR_OK;
			}	
		}
	}
	
	WARNING("Can't find usbprog adapter! Please check connection and permissions.");
	return ERROR_JTAG_INIT_FAILED;
}

int usbprog_quit(void)
{
	usb_close(usbprog_handle);
	return ERROR_OK;
}
