/*
 * avrdude - A Downloader/Uploader for AVR device programmers
 * Copyright (C) 2005,2006 Joerg Wunsch
 * Copyright (C) 2006 David Moore
 * Copyright (C) 2006 Benedikt Sauter (based on usb_libusb.c and stk500v2.c)
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

/* $Id: usb_libusb.c,v 1.10 2006/12/11 12:47:35 joerg_wunsch Exp $ */

#include "ac_cfg.h"
#if defined(HAVE_LIBUSB)


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

#include <usb.h>

#include "serial.h"
#include "usbprog.h"


#include "avr.h"
#include "pgm.h"
#include "stk500_private.h"	// temp until all code converted
#include "stk500v2_private.h"
#include "stk500v2_headers.h"

/*
static int stk500v2_initialize(PROGRAMMER * pgm, AVRPART * p);

static void stk500v2_display(PROGRAMMER * pgm, char * p);
static void stk500v2_enable(PROGRAMMER * pgm);
static void stk500v2_disable(PROGRAMMER * pgm);
static int stk500v2_program_enable(PROGRAMMER * pgm, AVRPART * p);
static int stk500v2_chip_erase(PROGRAMMER * pgm, AVRPART * p);
static int stk500v2_cmd(PROGRAMMER * pgm, unsigned char cmd[4],
                        unsigned char res[4]);
static void stk500v2_close(PROGRAMMER * pgm);
static int stk500v2_paged_write(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m,
                              int page_size, int n_bytes);
static int stk500v2_paged_load(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m,
                             int page_size, int n_bytes);
static void stk500v2_print_parms(PROGRAMMER * pgm, char * p);
static int stk500v2_set_sck_period_mk2(PROGRAMMER * pgm, double v);
static int stk500v2_perform_osccal(PROGRAMMER * pgm);
*/
extern int    verbose;
extern char * progname;
extern int do_cycles;


/********** serial device emulation *********************/

extern char *progname;
extern int verbose;

static char usbbuf[USBPROG_MAX_XFER];
static int buflen = -1, bufptr;

static int usb_interface;
/*
 * The "baud" parameter is meaningless for USB devices, so we reuse it
 * to pass the desired USB device ID.
 */
static void usbprog_open(char * port, long baud, union filedescriptor *fd)
{
  char string[256];
  char product[256];
  struct usb_bus *bus;
  struct usb_device *dev;
  usb_dev_handle *udev;
  char *serno, *cp2;
  size_t x;

  usb_init();

  usb_find_busses();
  usb_find_devices();

  for (bus = usb_get_busses(); bus; bus = bus->next){
		for (dev = bus->devices; dev; dev = dev->next){
	  	udev = usb_open(dev);
	  	if (udev){
	    	if (dev->descriptor.idVendor == USBPROG_VID &&
		  		dev->descriptor.idProduct == USBPROG_PID){
		  			/* yeah, we found something */
		  
		
					/* check if device has an configuration like AVR069 */
		  		if (dev->config == NULL){
		      	fprintf(stderr,
			      	"%s: usbprog_open(): USB device has no configuration\n",
			      	progname);
		      	goto trynext;
		    	}

					/* select first configuation */
		  		if (usb_set_configuration(udev, dev->config[0].bConfigurationValue)){
		      	fprintf(stderr,
			      	"%s: usbprog_open(): error setting configuration %d: %s\n",
			      	progname, dev->config[0].bConfigurationValue,
			      	usb_strerror());
		      	goto trynext;
		    	}

		 
		 			/* specify interface */
					usb_interface = dev->config[0].interface[0].altsetting[0].bInterfaceNumber;
		  		if (usb_claim_interface(udev, usb_interface)) {
		      	fprintf(stderr,
			      	"%s: usbprog_open(): error claiming interface %d: %s\n",
			      	progname, usb_interface, usb_strerror());
		      	goto trynext;
		    	}

		  		fd->pfd = udev;
          return;
				}
	      trynext:
	      usb_close(udev);
	    }
		}
  }

  fprintf(stderr, "%s: usbprog_open(): did not find any USB device\n",
	  progname);
  exit(1);
}

static void usbprog_close(union filedescriptor *fd)
{
  usb_dev_handle *udev = (usb_dev_handle *)fd->pfd;

  (void)usb_release_interface(udev, usb_interface);

  /*
   * Without this reset, the AVRISP mkII seems to stall the second
   * time we try to connect to it.
   */
  usb_reset(udev);

  usb_close(udev);
}


static int usbprog_send(union filedescriptor *fd, unsigned char *bp, size_t mlen)
{
  usb_dev_handle *udev = (usb_dev_handle *)fd->pfd;
  int rv;
  int i = mlen;
  unsigned char * p = bp;
  int tx_size;

  /*
   * Split the frame into multiple packets.  It's important to make
   * sure we finish with a short packet, or else the device won't know
   * the frame is finished.  For example, if we need to send 64 bytes,
   * we must send a packet of length 64 followed by a packet of length
   * 0.
   */
  do {
    tx_size = (mlen < USBPROG_MAX_XFER)? mlen: USBPROG_MAX_XFER;
    rv = usb_bulk_write(udev, USBPROG_BULK_EP_WRITE, (char *)bp, tx_size, 5000);
    if (rv != tx_size)
    {
        fprintf(stderr, "%s: usbprog_send(): wrote %d out of %d bytes, err = %s\n",
                progname, rv, tx_size, usb_strerror());
        return -1;
    }
    bp += tx_size;
    mlen -= tx_size;
  } while (tx_size == USBPROG_MAX_XFER);

  if (verbose > 3)
  {
      fprintf(stderr, "%s: Sent: ", progname);

      while (i) {
        unsigned char c = *p;
        if (isprint(c)) {
          fprintf(stderr, "%c ", c);
        }
        else {
          fprintf(stderr, ". ");
        }
        fprintf(stderr, "[%02x] ", c);

        p++;
        i--;
      }
      fprintf(stderr, "\n");
  }
  return 0;
}

/*
 * As calls to usb_bulk_read() result in exactly one USB request, we
 * have to buffer the read results ourselves, so the single-char read
 * requests performed by the upper layers will be handled.  In order
 * to do this, we maintain a private buffer of what we've got so far,
 * and transparently issue another USB read request if the buffer is
 * empty and more data are requested.
 */
static int
usb_fill_buf(usb_dev_handle *udev)
{
  int rv;

  rv = usb_bulk_read(udev, USBPROG_BULK_EP_READ, usbbuf, USBPROG_MAX_XFER, 5000);
  if (rv < 0)
    {
      if (verbose > 1)
	fprintf(stderr, "%s: usb_fill_buf(): usb_bulk_read() error %s\n",
		progname, usb_strerror());
      return -1;
    }

  buflen = rv;
  bufptr = 0;

  return 0;
}

static int usbprog_recv(union filedescriptor *fd, unsigned char *buf, size_t nbytes)
{
  usb_dev_handle *udev = (usb_dev_handle *)fd->pfd;
  int i, amnt;
  unsigned char * p = buf;

  for (i = 0; nbytes > 0;)
    {
      if (buflen <= bufptr)
	{
	  if (usb_fill_buf(udev) < 0)
	    return -1;
	}
      amnt = buflen - bufptr > nbytes? nbytes: buflen - bufptr;
      memcpy(buf + i, usbbuf + bufptr, amnt);
      bufptr += amnt;
      nbytes -= amnt;
      i += amnt;
    }

  if (verbose > 3)
  {
      fprintf(stderr, "%s: Recv: ", progname);

      while (i) {
        unsigned char c = *p;
        if (isprint(c)) {
          fprintf(stderr, "%c ", c);
        }
        else {
          fprintf(stderr, ". ");
        }
        fprintf(stderr, "[%02x] ", c);

        p++;
        i--;
      }
      fprintf(stderr, "\n");
  }

  return 0;
}

/*
 * This version of recv keeps reading packets until we receive a short
 * packet.  Then, the entire frame is assembled and returned to the
 * user.  The length will be unknown in advance, so we return the
 * length as the return value of this function, or -1 in case of an
 * error.
 *
 * This is used for the AVRISP mkII device.
 */
static int usbprog_recv_frame(union filedescriptor *fd, unsigned char *buf, size_t nbytes)
{
  usb_dev_handle *udev = (usb_dev_handle *)fd->pfd;
  int rv, n;
  int i;
  unsigned char * p = buf;

  n = 0;
  do
    {
      rv = usb_bulk_read(udev, USBPROG_BULK_EP_READ, usbbuf,
			 USBPROG_MAX_XFER, 10000);
      if (rv < 0)
	{
	  if (verbose > 1)
	    fprintf(stderr, "%s: usbprog_recv_frame(): usb_bulk_read(): %s\n",
		    progname, usb_strerror());
	  return -1;
	}

      if (rv <= nbytes)
	{
	  memcpy (buf, usbbuf, rv);
	  buf += rv;
	}

      n += rv;
      nbytes -= rv;
    }
  while (rv == USBPROG_MAX_XFER);

  if (nbytes < 0)
    return -1;

  if (verbose > 3)
  {
      i = n;
      fprintf(stderr, "%s: Recv: ", progname);

      while (i) {
        unsigned char c = *p;
        if (isprint(c)) {
          fprintf(stderr, "%c ", c);
        }
        else {
          fprintf(stderr, ". ");
        }
        fprintf(stderr, "[%02x] ", c);

        p++;
        i--;
      }
      fprintf(stderr, "\n");
  }
  return n;
}

static int usbprog_drain(union filedescriptor *fd, int display)
{
  usb_dev_handle *udev = (usb_dev_handle *)fd->pfd;
  int rv;

  do {
    rv = usb_bulk_read(udev, USBPROG_BULK_EP_READ, usbbuf, USBPROG_MAX_XFER, 100);
    if (rv > 0 && verbose >= 4)
      fprintf(stderr, "%s: usbprog_drain(): flushed %d characters\n",
	      progname, rv);
  } while (rv > 0);

  return 0;
}

/*
 * Device descriptor for the usbprog
 */

struct serial_device usb_serdev_usbprog =
{
  .open = usbprog_open,
  .close = usbprog_close,
  .send = usbprog_send,
  .recv = usbprog_recv_frame,
  .drain = usbprog_drain,
  .flags = SERDEV_FL_NONE,
};


/******************** stk500 device ********************/



/*
 * Open usbprog in ISP mode.
 */
static int stk500v2_usbprog_isp_open(PROGRAMMER * pgm, char * port)
{
  long baud;

  if (verbose >= 2)
    fprintf(stderr, "%s: stk500v2_usbprog_isp_open()\n", progname);

  /*
   * The JTAG ICE mkII always starts with a baud rate of 19200 Bd upon
   * attaching.  If the config file or command-line parameters specify
   * a higher baud rate, we switch to it later on, after establishing
   * the connection with the ICE.
   */
  baud = 19200;

  /*
   * If the port name starts with "usb", divert the serial routines
   * to the USB ones.  The serial_open() function for USB overrides
   * the meaning of the "baud" parameter to be the USB device ID to
   * search for.
   */
  //if (strncmp(port, "usb", 3) == 0) {
#if defined(HAVE_LIBUSB)
    serdev = &usb_serdev_usbprog;
    baud = 0;
#else
    fprintf(stderr, "avrdude was compiled without usb support.\n");
    return -1;
#endif
  //}

  strcpy(pgm->port, port);
  serial_open(port, baud, &pgm->fd);

  /*
   * drain any extraneous input
   */
  stk500v2_drain(pgm, 0);

  if (stk500v2_getsync(pgm) != 0) {
    fprintf(stderr, "%s: failed to sync with the usbprog in ISP mode\n",
            progname);
    pgm->close(pgm);		/* sign off correctly */
    exit(1);
  }
}

void usbprog_initpgm(PROGRAMMER * pgm)
{
  strcpy(pgm->type, "USBPROG_ISP");

  /*
   * mandatory functions
   */
  pgm->initialize     = stk500v2_initialize;
  pgm->display        = stk500v2_display;
  pgm->enable         = stk500v2_enable;
  pgm->disable        = stk500v2_disable;
  pgm->program_enable = stk500v2_program_enable;
  pgm->chip_erase     = stk500v2_chip_erase;
  pgm->cmd            = stk500v2_cmd;
  pgm->open           = stk500v2_usbprog_isp_open;
  pgm->close          = stk500v2_close;
  pgm->read_byte      = avr_read_byte_default;
  pgm->write_byte     = avr_write_byte_default;

  /*
   * optional functions
   */
  pgm->paged_write    = stk500v2_paged_write;
  pgm->paged_load     = stk500v2_paged_load;
  pgm->print_parms    = stk500v2_print_parms;
  pgm->set_sck_period = stk500v2_set_sck_period_mk2;
  pgm->perform_osccal = stk500v2_perform_osccal;
  pgm->page_size      = 256;
}


#endif  /* HAVE_LIBUSB */
