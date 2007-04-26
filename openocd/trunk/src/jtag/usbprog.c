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

#include <simpleport.h>


#define VID 0x1781
#define PID 0x0c62


// pins from avr
#define TDO_BIT		0
#define TDI_BIT		3
#define TCK_BIT		2
#define TMS_BIT		1


#include <usb.h>

/* low level command set
 */

// need for communication with usbprog
struct simpleport * sp_handle;


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
  int i = simpleport_get_bit(sp_handle,0);
  if(i==0)
    return 0;
  else
    return 1;
}

void usbprog_write(int tck, int tms, int tdi)
{
	char output_value=0x00;	
	
	if (tms)
		output_value |= (1<<TMS_BIT);
		//simpleport_set_bit(sp_handle,TMS_BIT,1);
	//else
		//simpleport_set_bit(sp_handle,TMS_BIT,0);
		//output_value &= ~(1<<TMS_BIT);
	
	if (tdi)
		//simpleport_set_bit(sp_handle,TDI_BIT,1);
		output_value |= (1<<TDI_BIT);
	//else
		//simpleport_set_bit(sp_handle,TDI_BIT,0);
		//output_value &= ~(1<<TDI_BIT);
	
	if (tck) 
		output_value |= (1<<TCK_BIT);
		//simpleport_set_bit(sp_handle,TCK_BIT,1);
	//else
		//simpleport_set_bit(sp_handle,TCK_BIT,0);
		//output_value &= ~(1<<TCK_BIT);

	simpleport_set_port(sp_handle,output_value);
}

/* (1) assert or (0) deassert reset lines */
void usbprog_reset(int trst, int srst)
{
	if(trst)
		simpleport_set_bit(sp_handle,5,1);
	else
		simpleport_set_bit(sp_handle,5,0);
 
	if(srst)
		simpleport_set_bit(sp_handle,4,1);
	else
		simpleport_set_bit(sp_handle,4,0);

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
	sp_handle = simpleport_open();

        if(sp_handle==0){
	      WARNING("Can't find usbprog adapter! Please check connection and permissions.");
	      return ERROR_JTAG_INIT_FAILED;
	}
	
	INFO("Find usbprog (SimplePort) adapter!");
	
	bitbang_interface = &usbprog_bitbang;

	simpleport_set_direction(sp_handle,0xFE);
	
	return ERROR_OK;
}

int usbprog_quit(void)
{
	simpleport_set_bit(sp_handle,6,1);
	simpleport_close(sp_handle);
	return ERROR_OK;
}
