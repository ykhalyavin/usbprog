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
#include "usbprogjtag.h"

#include <usb.h>

struct usbprog_jtag* usbprog_jtag_open()
{
  struct usb_bus *busses;
  struct usb_dev_handle* usb_handle;
  struct usb_bus *bus;
  struct usb_device *dev;

  struct usbprog_jtag * tmp;

  tmp = (struct usbprog_jtag*)malloc(sizeof(struct usbprog_jtag));


  usb_init();
  usb_find_busses();
  usb_find_devices();

  busses = usb_get_busses();

  /* find usbprog_jtag device in usb bus */

  for (bus = busses; bus; bus = bus->next){
    for (dev = bus->devices; dev; dev = dev->next){
      /* condition for sucessfully hit (too bad, I only check the vendor id)*/
      if (dev->descriptor.idVendor == VID && dev->descriptor.idProduct == PID) {
	tmp->usb_handle = usb_open(dev);

	usb_set_configuration (tmp->usb_handle,dev->config[0].bConfigurationValue);
	usb_claim_interface(tmp->usb_handle, 0);
	usb_set_altinterface(tmp->usb_handle,0);

	return tmp;
      }
    } 
  }
  return 0;
}


void usbprog_jtag_close(struct usbprog_jtag *usbprog_jtag)
{
  usb_close(usbprog_jtag->usb_handle);
  free(usbprog_jtag);
}


unsigned char _usbprog_jtag_message(struct usbprog_jtag *usbprog_jtag, char *msg, int msglen)
{
  int res = usb_bulk_write(usbprog_jtag->usb_handle,3,msg,msglen,100);
    if(msg[0]==2)
      return 1;  
  if(res == msglen) {
    res =  usb_bulk_read(usbprog_jtag->usb_handle,2, msg, 2, 100);
    if (res > 0)
      return (unsigned char)msg[1];
    else 
      return -1;
  }
  else
    return -1;
  return 0;
}

void usbprog_jtag_init(struct usbprog_jtag *usbprog_jtag)
{
  usbprog_jtag_set_direction(usbprog_jtag, PORT_TDI|PORT_TMS|PORT_TCK|PORT_SRST|PORT_TRST);  // all out only TDO is in
}


void usbprog_jtag_write_and_read(struct usbprog_jtag *usbprog_jtag, char * buffer, int size)
{
  char tmp[64];	// fastes packet size for usb controller
  int send_bits,bufindex=0,fillindex=0,i,j,complete=size,loops;

  char swap;
  // 61 byte can be transfered (488 bit)
    
  while(size > 0) {  
    
    if(size > 488) {
      send_bits = 488;
      size = size - 488;
      loops = 61;
    } else {
      send_bits = size;
      loops = size/8;
      //if(loops==0)
			loops++;
      size = 0;
    }
    tmp[0] = WRITE_AND_READ;
    tmp[1] = (char)(send_bits>>8); // high 
    tmp[2] = (char)(send_bits);    // low
    i=0; 

    for(i=0;i < loops ;i++) {
      tmp[3+i]=buffer[bufindex];
      bufindex++;
    }
    
    usb_bulk_write(usbprog_jtag->usb_handle,3,tmp,64,1000);
    
    while(usb_bulk_read(usbprog_jtag->usb_handle,2, tmp, 64, 100) < 1);

    for(i=0;i<loops ;i++) {
      swap =  tmp[3+i];
      buffer[fillindex++] = swap;
    } 
  }
}


void usbprog_jtag_read_tdo(struct usbprog_jtag *usbprog_jtag, char * buffer, int size)
{
  char tmp[64];	// fastes packet size for usb controller
  int send_bits,bufindex=0,fillindex=0,i,j,complete=size,loops;

  char swap;
  // 61 byte can be transfered (488 bit)
    
  while(size > 0) {  
    
    if(size > 488) {
      send_bits = 488;
      size = size - 488;
      loops = 61;
    } else {
      send_bits = size;
      loops = size/8;
      //if(loops==0)
			loops++;
      size = 0;
    }
    tmp[0] = WRITE_AND_READ;
    tmp[1] = (char)(send_bits>>8); // high 
    tmp[2] = (char)(send_bits);    // low
    
    usb_bulk_write(usbprog_jtag->usb_handle,3,tmp,3,1000);
    
    while(usb_bulk_read(usbprog_jtag->usb_handle,2, tmp, 64, 10) < 1);

    for(i=0;i<loops ;i++) {
      swap =  tmp[3+i];
      buffer[fillindex++] = swap;
    } 
  }
}

void usbprog_jtag_write_tdi(struct usbprog_jtag *usbprog_jtag, char * buffer, int size)
{
  char tmp[64];	// fastes packet size for usb controller
  int send_bits,bufindex=0,fillindex=0,i,j,complete=size,loops;
  char swap;
  // 61 byte can be transfered (488 bit)
  while(size > 0) {  
    if(size > 488) {
      send_bits = 488;
      size = size - 488;
      loops = 61;
    } else {
      send_bits = size;
      loops = size/8;
      //if(loops==0)
			loops++;
      size = 0;
    }
    tmp[0] = WRITE_TDI;
    tmp[1] = (char)(send_bits>>8); // high 
    tmp[2] = (char)(send_bits);    // low
    i=0; 

    for(i=0;i < loops ;i++) {
      tmp[3+i]=buffer[bufindex];
      bufindex++;
    }
    usb_bulk_write(usbprog_jtag->usb_handle,3,tmp,64,1000);
  }
  //usb_bulk_read(usbprog_jtag->usb_handle,2, tmp, 2, 100);
}


void usbprog_jtag_write_tms(struct usbprog_jtag *usbprog_jtag, char tms_scan)
{
  char tmp[2];	// fastes packet size for usb controller
  tmp[0] = WRITE_TMS;
  tmp[1] = tms_scan;
  usb_bulk_write(usbprog_jtag->usb_handle,3,tmp,2,1000);
}


void usbprog_jtag_set_direction(struct usbprog_jtag *usbprog_jtag, unsigned char direction)
{
  char tmp[2];
  tmp[0] = PORT_DIRECTION;
  tmp[1] = (char)direction;
  _usbprog_jtag_message(usbprog_jtag,tmp,2);
}

void usbprog_jtag_write_slice(struct usbprog_jtag *usbprog_jtag,unsigned char value)
{
  char tmp[2];
  tmp[0] = PORT_SET;
  tmp[1] = (char)value;
  _usbprog_jtag_message(usbprog_jtag,tmp,2);
}

unsigned char usbprog_jtag_get_port(struct usbprog_jtag *usbprog_jtag)
{
  char tmp[2];
  tmp[0] = PORT_GET;
  tmp[1] = 0x00;
  return _usbprog_jtag_message(usbprog_jtag,tmp,2);
}


void usbprog_jtag_set_bit(struct usbprog_jtag *usbprog_jtag,int bit, int value)
{
  char tmp[3];
  tmp[0] = PORT_SETBIT;
  tmp[1] = (char)bit;
  if(value==1)  
    tmp[2] = 0x01;
  else
    tmp[2] = 0x00;
  _usbprog_jtag_message(usbprog_jtag,tmp,3);
}

int usbprog_jtag_get_bit(struct usbprog_jtag *usbprog_jtag, int bit)
{
  char tmp[2];
  tmp[0] = PORT_GETBIT;
  tmp[1] = (char)bit;

  if(_usbprog_jtag_message(usbprog_jtag,tmp,2)>0)
    return 1;
  else
    return 0;
}


void usbprog_jtag_tap_goto_reset(struct usbprog_jtag *usbprog_jtag)
{
//#define TAP_RESET       0x0B

}

void usbprog_jtag_tap_goto_capture_dr(struct usbprog_jtag *usbprog_jtag)
{

//#define TAP_CAPTURE_DR  0x0D

}

void usbprog_jtag_tap_goto_capture_ir(struct usbprog_jtag *usbprog_jtag)
{

//#define TAP_CAPTURE_IR  0x0E
}
void usbprog_jtag_tap_shift_register_final(struct usbprog_jtag *usbprog_jtag,char * in, char * out, int size)
{
  char tmp[size+2];	// fastes packet size for usb controller
  int i;

  tmp[0] = TAP_SHIFT_FINAL;
  tmp[1] = (char)(size); // high 
  for(i=0;i<size;i++)
    tmp[2+i] = in[i];
    
  //usb_bulk_write(usbprog_jtag->usb_handle,3,tmp,size+2,1000);
  //while(usb_bulk_read(usbprog_jtag->usb_handle,2, tmp, 64, 100) < 1);
  
  if(usb_bulk_write(usbprog_jtag->usb_handle,3,tmp,size+2,1000)>1)                 {
    usleep(1);
    int timeout=0;
    while(usb_bulk_read(usbprog_jtag->usb_handle,0x82, tmp, 64, 1000) < 1){
      timeout++;
      if(timeout>10)                                         
	break;
    }

    for(i=0;i<size ;i++) {
      in[i] = tmp[2+i];
    }
  }

//#define TAP_SHIFT       0x0C
}
void usbprog_jtag_tap_shift_register(struct usbprog_jtag *usbprog_jtag,char * in, int inlen, char * out, int outlen)
{
  char tmp[inlen+2];	// fastes packet size for usb controller
  int i;

  tmp[0] = TAP_SHIFT;
  tmp[1] = (char)(inlen); // high 
  for(i=0;i<inlen;i++)
    tmp[2+i] = in[i];
    
  if(usb_bulk_write(usbprog_jtag->usb_handle,3,tmp,64,1000)>1)                 {
    usleep(1);
    int timeout=0;
    while(usb_bulk_read(usbprog_jtag->usb_handle,0x82, tmp, 64, 1000) < 1){
      timeout++;
      if(timeout>100)                                         
	break;
    }

    for(i=0; i<outlen ;i++) {
      out[i] = tmp[2+i];
    }
 
  }
  //#define TAP_SHIFT       0x0C
}
