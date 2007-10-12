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
#include "simpleport.h"

#include <usb.h>

struct simpleport* simpleport_open()
{
  struct usb_bus *busses;
  struct usb_dev_handle* usb_handle;
  struct usb_bus *bus;
  struct usb_device *dev;

  struct simpleport * tmp;

  tmp = (struct simpleport*)malloc(sizeof(struct simpleport));


  usb_init();
  usb_find_busses();
  usb_find_devices();

  busses = usb_get_busses();

  /* find simpleport device in usb bus */

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


void simpleport_close(struct simpleport *simpleport)
{
  usb_close(simpleport->usb_handle);
  free(simpleport);
}


unsigned char simpleport_message(struct simpleport *simpleport, char *msg, int msglen, int answerlen)
{
  int res = usb_bulk_write(simpleport->usb_handle,3,msg,msglen,100);
  if(answerlen>0 && res > 0) {
    res =  usb_bulk_read(simpleport->usb_handle,2, msg, answerlen, 100);
    if (res > 0)
      return (unsigned char)msg[1];
    else 
      return -1;
  }
  else
    return -1;
}

void simpleport_set_direction(struct simpleport *simpleport, unsigned char direction)
{
  char tmp[2];
  tmp[0] = PORT_DIRECTION;
  tmp[1] = (char)direction;
  simpleport_message(simpleport,tmp,2,0);
}

void simpleport_set_port(struct simpleport *simpleport,unsigned char value, unsigned char mask)
{
  char tmp[3];
  tmp[0] = PORT_SET;
  tmp[1] = (char)value;
  tmp[2] = (char)mask;
  simpleport_message(simpleport,tmp,3,0);
}

unsigned char simpleport_get_port(struct simpleport *simpleport)
{
  char tmp[2];
  tmp[0] = PORT_GET;
  tmp[1] = 0x00;
  return simpleport_message(simpleport,tmp,2,2);
}


void simpleport_set_pin(struct simpleport *simpleport,int pin, int value)
{
  char tmp[3];
  tmp[0] = PORT_SETPIN;
  tmp[1] = (char)pin;
  if(value==1)  
    tmp[2] = 0x01;
  else
    tmp[2] = 0x00;
  simpleport_message(simpleport,tmp,3,0);
}

void simpleport_set_pin_dir(struct simpleport *simpleport,int pin, int value)
{
  char tmp[3];
  tmp[0] = PORT_SETPINDIR;
  tmp[1] = (char)pin;
  if(value==1)  
    tmp[2] = 0x01;
  else
    tmp[2] = 0x00;
  simpleport_message(simpleport,tmp,3,0);
}

int simpleport_get_pin(struct simpleport *simpleport, int pin)
{
  char tmp[2];
  tmp[0] = PORT_GETPIN;
  tmp[1] = (char)pin;

  if(simpleport_message(simpleport,tmp,2,2)>0)
    return 1;
  else
    return 0;
}

