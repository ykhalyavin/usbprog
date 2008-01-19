/***************************************************************************
 *   Copyright (C) 2008 by Benedikt Sauter                                 *
 *   sauter@ixbat.de                                                       *
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

#include <usb.h>

/* vendor requests */

#define FIRMWARE_UPDATE	0x01
#define SET_SPEED	0x02
#define GET_SPEED	0x03
#define USER_INTERFACE	0x04
#define GET_VERSION	0x05


/* USER_INTERFACE wValues */
#define LED_ON	    0x01
#define LED_OFF	    0x02
#define GET_JUMPER  0x03


/* SPEED wValues  */
#define SPEED_6KHZ	  0x0006
#define SPEED_100KHZ	  0x0064
#define SPEED_250KHZ	  0x00FA
#define SPEED_500KHZ	  0x01F4
#define SPEED_1MHZ	  0x03EB
#define SPEED_5MHZ	  0x1388


/* JTAG COMMANDS for command buffer */
					      /* 0 0 1 T B R W V (T:1=TDI;T:0=TMS, B:1=Byte;B:0=Bit) */
#define CLOCK_DATA_BYTES_OUT		0x3A  /* 0 0 1 1 1 0 1 0 */
#define CLOCK_DATA_BITS_OUT		0x32  /* 0 0 1 1 0 0 1 0 */
#define CLOCK_DATA_BYTES_IN		0x3C  /* 0 0 1 1 1 1 0 0 */
#define CLOCK_DATA_BITS_IN		0x34  /* 0 0 1 1 0 1 0 0 */
#define CLOCK_DATA_BYTES_OUT_IN		0x3E  /* 0 0 1 1 1 1 1 0 */
#define CLOCK_DATA_BITS_OUT_IN		0x36  /* 0 0 1 1 0 1 1 0 */
#define CLOCK_DATA_BIT_TMS_TDI_1	0x23  /* 0 0 1 0 0 0 1 1 */
#define CLOCK_DATA_BIT_TMS_TDI_0	0x22  /* 0 0 1 0 0 0 1 0 */

#define CLOCK_DATA_TMS_TDI_1_WITH_READ	0x27  /* 0 0 1 0 0 1 1 1 */
#define CLOCK_DATA_TMS_TDI_0_WITH_READ	0x26  /* 0 0 1 0 0 1 1 0 */

/* GPIO COMMANDS for command buffer */

#define SET_TDI(x)  (0x05 | (1<<1) | (x > 0) ? 1:0)
#define SET_TMS(x)  (0x05 | (2<<1) | (x > 0) ? 1:0)
#define SET_TCK(x)  (0x05 | (3<<1) | (x > 0) ? 1:0)
#define SET_TRST(x) (0x05 | (4<<1) | (x > 0) ? 1:0)
#define SET_SRST(x) (0x05 | (5<<1) | (x > 0) ? 1:0)

#define GET_TDO() (0x04 | (6<<1))

#define USB_VENDOR_REQUEST  0xC0


static char *usbprog_buffer = NULL;
static int usbprog_buffer_size = 0;
static int usbprog_read_pointer = 0;
static int usbprog_expect_read = 0;

#define USBPROG_BUFFER_SIZE  320
#define BUFFER_ADD usbprog_buffer[usbprog_buffer_size++]
#define BUFFER_READ usbprog_buffer[usbprog_read_pointer++]


/* open connection */
usb_dev_handle *usbprog_locate(void);

/* close connection */
int usbprog_close(usb_dev_handle * usbprog_handle);

/* transmit and receive command buffer */
int usbprog_command_buffer(usb_dev_handle * usbprog_handle, char *read_buffer, int read_length, char *write_buffer, int write_length);

/* control trst signal 0:low, 1:high */
int usbprog_trst(usb_dev_handle * usbprog_handle, int value);

/* control trst signal 0:low, 1:high */
int usbprog_srst(usb_dev_handle * usbprog_handle, int value);

/* control led 0:off, 1:on */
int usbprog_led(usb_dev_handle * usbprog_handle, int value);

/* get jumper postion, return 1=close,0=open */
int usbprog_jumper(usb_dev_handle * usbprog_handle);

/* value = kHz (6 = kHz, 5000 = 5 MHz) */
int usbprog_speed(usb_dev_handle * usbprog_handle, short value);

