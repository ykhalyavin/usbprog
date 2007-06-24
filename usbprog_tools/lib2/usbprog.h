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



int usbprog_init();

struct usb_dev_handle* usbprog_open(short vendorid, short productid);
struct usb_dev_handle* usbprog_open_number(int number);
struct usb_dev_handle* usbprog_open_serial(short vendorid, short productid, char *serial);


int usbprog_close(usb_dev_handle* dev);
int usbprog_print_devices();
int usbprog_vendor_mode(usb_dev_handle* dev);

/**
 * ask the usb device for the version id to check if the vendor request
 * is implemented and the device is so an usbprog compatible.
 */
int usbprog_vendor_ask(usb_dev_handle* dev);


void usbprog_flash_bin(struct usb_dev_handle* dev,char *file);

void usbprog_print_netlist(char * url);
int usbprog_download(char * url, int filenumber);








