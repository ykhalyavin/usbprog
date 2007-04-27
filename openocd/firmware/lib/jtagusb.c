/*
 * Copyright (C) 2007 Benedikt Sauter 
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
#include "usbprog_jtag.h"

#include <usb.h>

struct usbprog_jtag* usbprog_jtag_open()
{
  struct usb_bus *busses;
  struct usb_dev_handle* usb_handle;
  struct usb_bus *bus;
  struct usb_device *dev;

  struct usbprog_jtag * tmp;

  tmp = (struct usbprog_jtag*)malloc(sizeof(struct usbprog_jtag));


  usb_init();
  usb_find_busses();
  usb_find_devices();

  busses = usb_get_busses();

  /* find usbprog_jtag device in usb bus */

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


void usbprog_jtag_close(struct usbprog_jtag *usbprog_jtag)
{
  usb_close(usbprog_jtag->usb_handle);
  free(usbprog_jtag);
}


unsigned char usbprog_jtag_message(struct usbprog_jtag *usbprog_jtag, char *msg, int msglen)
{
  int res = usb_bulk_write(usbprog_jtag->usb_handle,3,msg,msglen,100);
  if(res == msglen) {
    res =  usb_bulk_read(usbprog_jtag->usb_handle,2, msg, 2, 100);
    if (res > 0)
      return (unsigned char)msg[1];
    else 
      return -1;
  }
  else
    return -1;
}

void usbprog_jtag_set_direction(struct usbprog_jtag *usbprog_jtag, unsigned char direction)
{
  char tmp[2];
  tmp[0] = PORT_DIRECTION;
  tmp[1] = (char)direction;
  usbprog_jtag_message(usbprog_jtag,tmp,2);
}

void usbprog_jtag_write_slice(struct usbprog_jtag *usbprog_jtag,unsigned char value)
{
  char tmp[3];
  tmp[0] = PORT_SET;
  tmp[1] = (char)value;
  tmp[2] = (char)mask;
  usbprog_jtag_message(usbprog_jtag,tmp,3);
}

unsigned char usbprog_jtag_get_port(struct usbprog_jtag *usbprog_jtag)
{
  char tmp[2];
  tmp[0] = PORT_GET;
  tmp[1] = 0x00;
  return usbprog_jtag_message(usbprog_jtag,tmp,2);
}


void usbprog_jtag_set_bit(struct usbprog_jtag *usbprog_jtag,int bit, int value)
{
  char tmp[3];
  tmp[0] = PORT_SETBIT;
  tmp[1] = (char)bit;
  if(value==1)  
    tmp[2] = 0x01;
  else
    tmp[2] = 0x00;
  usbprog_jtag_message(usbprog_jtag,tmp,3);
}

int usbprog_jtag_get_bit(struct usbprog_jtag *usbprog_jtag, int bit)
{
  char tmp[2];
  tmp[0] = PORT_GETBIT;
  tmp[1] = (char)bit;

  if(usbprog_jtag_message(usbprog_jtag,tmp,2)>0)
    return 1;
  else
    return 0;
}

