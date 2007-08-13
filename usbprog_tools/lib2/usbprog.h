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

#include <usb.h>

struct usbprog_context{
  char * error_str;
  char * url;
  char * versions_xml;
  usb_dev_handle *usb_handle;
};

int usbprog_init(struct usbprog_context* usbprog);
int usbprog_close(struct usbprog_context* usbprog);


/* get number of available usb devices */
int usbprog_get_numberof_devices(struct usbprog_context* usbprog);

/* get array for device select field */
int usbprog_print_devices(struct usbprog_context *usbprog, char** buf);


/* get newest versions.xml */
int usbprog_online_get_netlist(struct usbprog_context *usbprog,char *url);

/* get number of firmwares */
int usbprog_online_numberof_firmwares(struct usbprog_context* usbprog, char** buf);

/* get array for firmware select field */
int usbprog_online_print_netlist(struct usbprog_context* usbprog, char** buf);




int usbprog_update_mode(struct usbprog_context* usbprog, short vendorid, short productid);
int usbprog_update_mode_number(struct usbprog_context* usbprog, int number);
int usbprog_update_mode_serial(struct usbprog_context* usbprog, short vendorid, short productid, char* serial);

int usbprog_get_file(struct usbprog_context* usbprog, char* file);
int usbprog_flash_file(struct usbprog_context* usbprog,char* file);

char* usbprog_get_error_string (struct usbprog_context* usbprog);

