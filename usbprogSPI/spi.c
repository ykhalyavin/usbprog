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
#include "spi.h"

#include <usb.h>

struct spi* spi_open()
{
  struct usb_bus *busses;
  struct usb_dev_handle* usb_handle;
  struct usb_bus *bus;
  struct usb_device *dev;

  struct spi * tmp;

  tmp = (struct spi*)malloc(sizeof(struct spi));


  usb_init();
  usb_find_busses();
  usb_find_devices();

  busses = usb_get_busses();

  /* find spi device in usb bus */

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


int spi_close(struct spi *spi)
{
  usb_close(spi->usb_handle);
  free(spi);
}



int spi_multi(struct spi *spi, char * send_buf, int send_size, char * recv_buf, int recv_size)
{
  char * cmd = (char*)malloc(sizeof(char)*send_size+4);  

  // setup command
  cmd[0] = CMD_SPI_MULTI;
  cmd[1] = (char)send_size;
  cmd[2] = (char)recv_size;
  cmd[3] = 0x00; //obsolete

  // copy data into command packet
  int i;
  for(i=0;i<send_size;i++){
    cmd[i+4] = send_buf[i];
  }

  int res = usb_bulk_write(spi->usb_handle,3,cmd,send_size+4, 100);

  if(res > 0) {
    char * answer = (char*)malloc(sizeof(char)*recv_size+3);
    res = usb_bulk_read(spi->usb_handle,0x82,answer,recv_size + 3, 100);
    if(res > 0){
      if(answer[1]==STATUS_CMD_OK){
	for(i=0;i<recv_size;i++){
	  recv_buf[i] = answer[2+i];
	}
      }

    } else {
      // can't get answer
    }
    free(answer);

  } else {
    // can't send cmd
  }

  free(cmd);

  return 1;
}


int spi_speed(struct spi *spi,int speed)
{
  //PARAM_SCK_DURATION

}

