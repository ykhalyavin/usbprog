/* 
   usbprog - Change easy the firmware on the usbprog adapter.

   Copyright (C) 2007 Benedikt Sauter

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  

*/

#include <stdio.h>
#include <string.h>

#include "usbprog.h"

#define usbprog_error_return(code, str) do {  \
	  usbprog->error_str = str;             \
	  return code;                       \
	} while(0);

/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_init(struct usbprog_context *usbprog)
{
  if(usbprog!=NULL) {
    usbprog->error_str	= NULL; 
    usbprog->url	= NULL; 
  } 
  
  usb_init();
}

/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_close(struct usbprog_context *usbprog)
{

}

/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_get_numberof_devices(struct usbprog_context *usbprog)
{
  struct usb_bus *busses;
  struct usb_dev_handle* usb_handle;
  struct usb_bus *bus;
  struct usb_device *dev;

  usb_find_busses();
  usb_find_devices();
  busses = usb_get_busses();
  int i=0;
  
  char vendor[255];
  char product[255];
  int vendorlen=0, productlen=0;

  for (bus = busses; bus; bus = bus->next) {
    for (dev = bus->devices; dev; dev = dev->next){

	#ifndef _WIN32
	if(dev->descriptor.bDeviceClass==0x09) // hub devices
	  continue;
	#endif

	usb_dev_handle * tmp_handle = usb_open(dev);

	vendor[0]=0x00; product[0]=0x00;
	vendorlen = usb_get_string_simple(tmp_handle, 1, vendor, 255);
	productlen = usb_get_string_simple(tmp_handle, 2, product, 255);

	if(vendorlen<=0 && productlen<=0){
	  usb_close(tmp_handle);
	  continue;
	}

	i++;

	usb_close(tmp_handle);
    }
  }
  return i;
}


/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_print_devices(struct usbprog_context *usbprog, char** buf)
{
  struct usb_bus *busses;
  struct usb_dev_handle* usb_handle;
  struct usb_bus *bus;
  struct usb_device *dev;

  usb_find_busses();
  usb_find_devices();
  busses = usb_get_busses();
  int i=0;
  
  char vendor[255];
  char product[255];
  char serial[255];
  int vendorlen=0, productlen=0, seriallen=0;

  for (bus = busses; bus; bus = bus->next) {
    for (dev = bus->devices; dev; dev = dev->next){

	#ifndef _WIN32
	if(dev->descriptor.bDeviceClass==0x09) // hub devices
	  continue;
	#endif

	if(dev->descriptor.bDescriptorType !=1)
	  continue;

	usb_dev_handle * tmp_handle = usb_open(dev);

	vendor[0]=0x00; product[0]=0x00;serial[0]=0x00;
	vendorlen = usb_get_string_simple(tmp_handle, 1, vendor, 255);
	productlen = usb_get_string_simple(tmp_handle, 2, product, 255);
	seriallen = usb_get_string_simple(tmp_handle, 3, serial, 255);

	if(vendorlen<=0) sprintf(vendor,"unkown vendor");
	if(productlen<=0) sprintf(product,"unkown product");
	if(seriallen<=0) sprintf(serial,"none");

	if(vendorlen<=0 && productlen<=0){
	  usb_close(tmp_handle);
	  continue;
	}

	char * complete = (char*)malloc(sizeof(char)*(strlen(vendor)+strlen(product)+strlen(serial)+20)); 
	sprintf(complete,"%s from %s (Serial: %s)",product,vendor,serial);
	buf[i++]=complete;

	usb_close(tmp_handle);
    }
  }

}

/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_update_mode(struct usbprog_context *usbprog, short vendorid, short productid)
{

}


/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_update_mode_number(struct usbprog_context *usbprog, int number)
{

}

/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_update_mode_serial(struct usbprog_context *usbprog, short vendorid, short productid, char *serial)
{

}

/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_set_url(struct usbprog_context *usbprog)
{

}

/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_print_netlist(struct usbprog_context *usbprog, char *buf)
{


}


/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_get_file(struct usbprog_context *usbprog, char *file)
{

}


/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
int usbprog_flash_file(struct usbprog_context *usbprog,char *file)
{

}


/**
 *     Get string representation for last error code
 *
 *         \param usbprog pointer to ftdi_context
 *
 *         \retval Pointer to error string 
 */
char *usbprog_get_error_string (struct usbprog_context *usbprog)
{     
  return usbprog->error_str;
}

