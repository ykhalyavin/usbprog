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

struct simpleport 
{
  struct usb_dev_handle* usb_handle;
};

struct simpleport* simpleport_open();
void simpleport_open(struct simpleport *simpleport);


void simpleport_set_direction(struct simpleport *simpleport, uint8_t direction);
void simpleport_set_port(struct simpleport *simpleport,uint8_t value);
uint8_t simpleport_get_port(struct simpleport *simpleport);
void simpleport_set_bit(struct simpleport *simpleport,uint8_t bit, uint8_t value);
uint8_t simpleport_get_bit(struct simpleport *simpleport, uint8_t bit);

