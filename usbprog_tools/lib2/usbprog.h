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
#include "xmlParser.h"

#define STARTAPP       0x01
#define WRITEPAGE      0x02
#define GETVERSION     0x03
#define SETVERSION     0x04
#define STOPPROGMODE   0x05

struct usbprog_context{
  char * error_str;
  char status_str[40];
  char * url;
  char * versions_xml;
  XMLNode xMainNode;
  usb_dev_handle *usb_handle;
  struct usb_device *devList[20];
};

int usbprog_init(struct usbprog_context* usbprog);

/* closes USB handle for usbprog */
int usbprog_close(struct usbprog_context* usbprog);

/* opens USB Handle for usbprog */
int usbprog_open(struct usbprog_context *usbprog, int number);

/* get number of available usb devices */
int usbprog_get_numberof_devices(struct usbprog_context* usbprog);

/* get array for device select field */
int usbprog_print_devices(struct usbprog_context *usbprog, char** buf);


/* get newest versions.xml */
int usbprog_online_get_netlist(struct usbprog_context *usbprog,char *url);

/* get number of firmwares */
int usbprog_online_numberof_firmwares(struct usbprog_context* usbprog);

/* get array for firmware select field */
int usbprog_online_print_netlist(struct usbprog_context* usbprog, char** buf,int numberof_firmwares);

/* activate update mode */
int usbprog_update_mode_number(struct usbprog_context* usbprog, int number);


/* flash online firmware */
int usbprog_flash_netfirmware(struct usbprog_context* usbprog, int number);

/* flash local firmware .bin */
int usbprog_flash_firmware(struct usbprog_context* usbprog, char *file);

/* flash buffer */
int usbprog_flash_buffer(struct usbprog_context* usbprog, char *buffer, int len);


/* quit update mode */
int usbprog_start_updatemode(struct usbprog_context* usbprog, int number);
int usbprog_stop_updatemode(struct usbprog_context* usbprog);


/* is usbprog in update mode */
int is_usbprog_in_update_mode(struct usbprog_context* usbprog);

int usbprog_update_mode_device(struct usbprog_context* usbprog, int number);




//int usbprog_update_mode(struct usbprog_context* usbprog, short vendorid, short productid);
//int usbprog_update_mode_serial(struct usbprog_context* usbprog, short vendorid, short productid, char* serial);
//int usbprog_get_file(struct usbprog_context* usbprog, char* file);

char* usbprog_get_error_string (struct usbprog_context* usbprog);

