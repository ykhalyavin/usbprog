/* JTAG GNU/Linux Xilinx USB IO

Copyright (C) 2006 inisyn research

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */



#ifndef IOXUSB_H
#define IOXUSB_H

#include <usb.h>

#include "iobase.h"

#define USB_WRITE_BUF_LEN 512

class IOXUSB : public IOBase
{
 protected:
  int usb_jtag_write_real(void);
  int usb_jtag_write(unsigned char val);
  unsigned char usb_jtag_read(void);

  char usb_write_buf[USB_WRITE_BUF_LEN];
  int usb_write_len;

  usb_dev_handle *dh;
  bool error;

 public:
  IOXUSB(const char *device_name);
  ~IOXUSB();
  virtual bool txrx(bool tms, bool tdi);
  virtual void tx(bool tms, bool tdi);
  inline bool checkError(){return error;}
};


#endif // IOXUSB_H

