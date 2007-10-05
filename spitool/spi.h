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

#define VID 0x03eb  //atmel
#define PID 0x2104  //avrisp mkII

#define CMD_SPI_MULTI 0x1D

// Success
#define STATUS_CMD_OK            0x00

// Warnings
#define STATUS_CMD_TOUT          0x80
#define STATUS_RDY_BSY_TOUT      0x81
#define STATUS_SET_PARAM_MISSING 0x82

// Errors
#define STATUS_CMD_FAILED        0xC0


struct spi
{
  struct usb_dev_handle* usb_handle;
};



struct spi* spi_open();
int spi_close(struct spi *spi);
int spi_multi(struct spi *spi, char * send_buf, int send_size, char * recv_buf, int recv_size);
int spi_speed(struct spi *spi,int speed);
