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
#ifndef _USBPROGJTAG_H_
#define _USBPROGJTAG_H_

#define VID 0x1781
#define PID 0x0c62

#define PIN_TDI	  7 
#define PIN_SRST  6
#define PIN_TRST  5
#define PIN_TMS   4
#define PIN_TCK   2
#define PIN_TDO   0

#define PORT_TDI   0x80
#define PORT_SRST  0x40
#define PORT_TRST  0x20
#define PORT_TMS   0x10
#define PORT_TCK   0x04
#define PORT_TDO   0x01




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
#define INIT_JTAG       0x0A
#define TAP_RESET       0x0B
#define TAP_SHIFT       0x0C
#define TAP_CAPTURE_DR  0x0D
#define TAP_CAPTURE_IR  0x0E
#define TAP_SHIFT_FINAL 0x0F


struct usbprog_jtag 
{
  struct usb_dev_handle* usb_handle;
};

struct usbprog_jtag* usbprog_jtag_open();
void usbprog_jtag_close(struct usbprog_jtag *usbprog_jtag);


/* activate all jtag signals */
void usbprog_jtag_init(struct usbprog_jtag *usbprog_jtag);


/* low level functions */
void usbprog_jtag_read_tdo(struct usbprog_jtag *usbprog_jtag, char * buffer, int size);
void usbprog_jtag_write_tdi(struct usbprog_jtag *usbprog_jtag, char * buffer, int size);
void usbprog_jtag_write_and_read(struct usbprog_jtag *usbprog_jtag, char * buffer, int size);
void usbprog_jtag_write_tms(struct usbprog_jtag *usbprog_jtag, char tms_scan);

/* single io function! An emulated JTAG connection is very slow!!! */
void usbprog_jtag_set_direction(struct usbprog_jtag *usbprog_jtag, unsigned char direction);
void usbprog_jtag_write_slice(struct usbprog_jtag *usbprog_jtag,unsigned char value);
unsigned char usbprog_jtag_get_port(struct usbprog_jtag *usbprog_jtag);
void usbprog_jtag_set_bit(struct usbprog_jtag *usbprog_jtag,int bit, int value);
int usbprog_jtag_get_bit(struct usbprog_jtag *usbprog_jtag, int bit);


/* basic jtag tap functions */
void usbprog_jtag_tap_goto_reset(struct usbprog_jtag *usbprog_jtag);
void usbprog_jtag_tap_goto_capture_dr(struct usbprog_jtag *usbprog_jtag);
void usbprog_jtag_tap_goto_capture_ir(struct usbprog_jtag *usbprog_jtag);

void usbprog_jtag_tap_shift_register(struct usbprog_jtag *usbprog_jtag,char * in, int inlen, char * out, int outlen);
void usbprog_jtag_tap_shift_register_final(struct usbprog_jtag *usbprog_jtag,char * in, char * out, int size);


/* internal function for lib */

unsigned char _usbprog_jtag_message(struct usbprog_jtag *usbprog_jtag, char *msg, int msglen);

#endif //_USBPROGJTAG_H_
