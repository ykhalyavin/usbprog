/* usbnapi.c
* Copyright (C) 2005  Benedikt Sauter
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

#ifndef _USBNAPI_H
#define _USBNAPI_H

#include "usbn960x.h"
#include "../usbn960xreg.h"

/// initial global data structures
void USBNInit(unsigned char *_DeviceDescriptor,unsigned char *_ConfigurationDescriptor);

void USBNCallbackFIFORX1(void *fct);

/// start usb system after configuration
void USBNStart(void);

/// handle usb chip interrupt
void USBNInterrupt(void);

/// move descriptor in a linear field and remove string descriptors
void _USBNCreateStringField(void);

uint8_t USBNAddToStringList( void* data);


/// add new string descriptor and get index
int _USBNAddStringDescriptor(char *string);


#endif /* __USBNAPI_H__ */
