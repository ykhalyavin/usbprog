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
#include "../fifo.h"

void _USBNAddToList( void* data, uint8_t len, 
  uint8_t type,uint8_t conf,uint8_t interf,uint8_t index);

uint8_t USBNAddToStringList( void* data);

/// initial global data structures
void USBNInit(void);

/// define vendor/product/device id for usb function
void USBNDeviceIDs(unsigned short idVendor, unsigned short idProduct, unsigned short bcdDevice);

/// define vendor id
void USBNDeviceVendorID(unsigned short idVendor);

/// define product id
void USBNDeviceProductID(unsigned short idProduct);

/// define device id e.g. 1.02 ( 0x0201 )
void USBNDeviceBCDDevice(unsigned short bcdDevice);

/// set manufacture string for device
void USBNDeviceManufacture(char *manufature);

/// set product string for device
void USBNDeviceProduct(char *product);

/// set serial number for device
void USBNDeviceSerialNumber(char *serialnumber);

/// usb class code
void USBNDeviceClass(unsigned char classcode); 

/// usb subclass code
void USBNDeviceSubClass(unsigned char subclasscode);

/// usb protocol code
void USBNDeviceProtocol(unsigned char protocolcode);

/// create a new configuration
int USBNAddConfiguration(void);

/// set a string for the configuration
void USBNConfigurationName(int configuration, char *name);

/// set powerlimit for the usb function
void USBNConfigurationPower(int configuration, int power);

/// create a new interface
int USBNAddInterface(int configuration, int number); 

/// create new class interface
int USBNAddInterfaceClass (int configuration, int number, char class, char subclass, char protocol);

/// set interface name
void USBNInterfaceName(int configuration, int interface, char *name);

/// define alternate setting for an interface
void USBNAlternateSetting(int configuration, int interface, int setting); 

/// define new out enpoint ( from host to function )
void USBNAddOutEndpoint(int configuration, int interface, int epnr, 
		  int epadr, char attr, int fifosize, int intervall,void *fkt);

/// define new in enpoint ( from function to host )
void USBNAddInEndpoint(int configuration, int interface, int epnr, 
		int epadr, char attr, int fifosize, int intervall,void *fkt);

/// add endpoint
void _USBNAddEndpoint(int configuration, int interface, int epnr, 
		int epadr,char attr, int fifosize, int intervall);


/// transmit data to host
void USBNSendData(int fifonumber, char *data);

/// move descriptor in a linear field and remove descr hierarchy
void _USBNCreateConfDescrField(void);

/// move descriptor in a linear field and remove string descriptors
void _USBNCreateStringField(void);

/// start usb system after configuration
void USBNStart(void);

/// handle usb chip interrupt
void USBNInterrupt(void);


/// add new string descriptor and get index
int _USBNAddStringDescriptor(char *string);

#endif /* __USBNAPI_H__ */
