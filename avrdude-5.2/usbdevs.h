/*
 * avrdude - A Downloader/Uploader for AVR device programmers
 * Copyright (C) 2006 Joerg Wunsch
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

/* $Id: usbdevs.h,v 1.1 2006/01/12 23:13:50 joerg_wunsch Exp $ */

/*
 * defines for the USB interface
 */

#ifndef usbdevs_h
#define usbdevs_h

#define USB_VENDOR_ATMEL 0x0400
#define USB_DEVICE_JTAGICEMKII 0xc35d
#define USB_DEVICE_AVRISPMKII  0xc35d
/*
 * Should we query the endpoint number and max transfer size from USB?
 * After all, the JTAG ICE mkII docs document these values.
 */
#define USBDEV_BULK_EP_WRITE 0x02
#define USBDEV_BULK_EP_READ  0x83
#define USBDEV_MAX_XFER 64

#endif  /* usbdevs_h */
