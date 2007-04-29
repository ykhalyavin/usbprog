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


#define VID 0x1781
#define PID 0x0c62

#define UNKOWN_COMMAND  0x00
#define PORT_DIRECTION  0x01
#define PORT_SET        0x02
#define PORT_GET        0x03
#define PORT_SETBIT     0x04
#define PORT_GETBIT     0x05
#define WRITE_TDI     	0x06
#define READ_TDO     	0x07
#define WRITE_AND_READ 	0x08
#define WRITE_TMS     	0x09

struct usbprog_jtag 
{
  struct usb_dev_handle* usb_handle;
};

struct usbprog_jtag* usbprog_jtag_open();
void usbprog_jtag_close(struct usbprog_jtag *usbprog_jtag);
void usbprog_jtag_init(struct usbprog_jtag *usbprog_jtag);
unsigned char usbprog_jtag_message(struct usbprog_jtag *usbprog_jtag, char *msg, int msglen);


void usbprog_jtag_read_tdo(struct usbprog_jtag *usbprog_jtag, char * buffer, int size);
void usbprog_jtag_write_tdi(struct usbprog_jtag *usbprog_jtag, char * buffer, int size);
void usbprog_jtag_write_and_read(struct usbprog_jtag *usbprog_jtag, char * buffer, int size);
void usbprog_jtag_write_tms(struct usbprog_jtag *usbprog_jtag, char tms_scan);


void usbprog_jtag_set_direction(struct usbprog_jtag *usbprog_jtag, unsigned char direction);
void usbprog_jtag_write_slice(struct usbprog_jtag *usbprog_jtag,unsigned char value);
unsigned char usbprog_jtag_get_port(struct usbprog_jtag *usbprog_jtag);
void usbprog_jtag_set_bit(struct usbprog_jtag *usbprog_jtag,int bit, int value);
int usbprog_jtag_get_bit(struct usbprog_jtag *usbprog_jtag, int bit);

