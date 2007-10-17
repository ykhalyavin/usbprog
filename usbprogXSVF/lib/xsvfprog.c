/*
 * Copyright (C) 2007 Sven Luetkemeier
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
 */

#include "xsvfprog.h"
#include "../firmware/defines.h"

#include <stdio.h>
#include <usb.h>

struct xsvfprog* xsvfprog_open()
{
	struct usb_bus *busses;
	struct usb_dev_handle* usb_handle;
	struct usb_bus *bus;
	struct usb_device *dev;
	
	struct xsvfprog * tmp;
	
	tmp = (struct xsvfprog*)malloc(sizeof(struct xsvfprog));
	
	usb_init();
	usb_find_busses();
	usb_find_devices();
	
	busses = usb_get_busses();
	
	/* find xsvfprog device in usb bus */
	
	for (bus = busses; bus; bus = bus->next){
		for (dev = bus->devices; dev; dev = dev->next){
			/* condition for sucessfully hit (too bad, I only check the vendor id)*/
			if (dev->descriptor.idVendor == VID && dev->descriptor.idProduct == PID) {
				tmp->usb_handle = usb_open(dev);
			
				usb_set_configuration (tmp->usb_handle,dev->config[0].bConfigurationValue);
				usb_claim_interface(tmp->usb_handle, 0);
				usb_set_altinterface(tmp->usb_handle,0);
			
				return tmp;
			}
		} 
	}
	return 0;
}


void xsvfprog_close(struct xsvfprog *xsvfprog)
{
	usb_close(xsvfprog->usb_handle);
	free(xsvfprog);
}


int xsvfprog_message(struct xsvfprog *xsvfprog, char *msg, int msglen, char *answer, int answerlen)
{
	int res = usb_bulk_write(xsvfprog->usb_handle,3,msg,msglen,100);
	if(res < 0) {
		return -1;
	} else if (answerlen > 0) {
		/* Note: We need a long timeout here, because some XSVF instructions take very long
		 * because they run internal device tests (see Xilinx XAPP503, XRUNTEST and other
		 * instructions).
		 */
		res =  usb_bulk_read(xsvfprog->usb_handle,2, answer, answerlen, 20000);
		return res;
	} else {
		return 0;
	}
}

int xsvfprog_init(struct xsvfprog *xsvfprog) {
	char tmp[2];
	tmp[0] = XSVF_INIT;
	if(xsvfprog_message(xsvfprog, tmp, 1, tmp, 2) != -1) {
		printf("%02x %02x\n", (unsigned char) tmp[0], (unsigned char) tmp[1]);
		return (unsigned char) tmp[0];
	} else {
		return -1;
	}
}

/* execute a single XSVF instruction */
int xsvfprog_exec(struct xsvfprog *xsvfprog, char* buf, int size) {
	char tmp[2];
	if(xsvfprog_message(xsvfprog, buf, size, tmp, 2) != -1) {
		return (unsigned char) tmp[0];
	} else {
		return -1;
	}
}
