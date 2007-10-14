/* usbn960x.c
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

#ifndef _USBN960X_H
#define _USBN960X_H

#define DEBUG 0

#include "../usbn960xreg.h"
#include "../usb11spec.h"

unsigned char *DeviceDescriptor;
unsigned char *ConfigurationDescriptor;
struct string_entry*  StringList;
char** FinalStringArray;



void *RX1Callback;



struct list_entry
{
  void *data;   uint8_t type;
  uint8_t len;
  uint8_t conf;
  uint8_t interf;
  uint8_t index;
  struct list_entry *next;
};

struct string_entry
{   void *data;
  uint8_t index;
  struct string_entry *next;
};



/*-------------------------------------------
 * global data structs
 * ------------------------------------------*/

typedef struct functioninfo FunctionInfo;
struct functioninfo {
  unsigned char Address;
  unsigned char ConfigurationIndex;
};


typedef struct devicereq DeviceRequest;
struct devicereq {
  unsigned char	  bmRequestType;
  unsigned char	  bRequest;
  unsigned short  wValue;
  unsigned short  wIndex;
  unsigned short  wLength;
};


typedef struct epinfo EPInfo;
struct epinfo {
  unsigned char	  usbnData;
  unsigned char	  usbnCommand;
  unsigned char	  usbnControl;
  unsigned char	  DataPid; // 0 = data0, 1 = data1
  int		  usbnfifo;
  int		  Index;
  int		  Size;
  unsigned char*  Buf;
}; 

unsigned char EP0RXBuf[8];


// system functions

void _USBNReceiveEvent(void);
void _USBNTransmitEvent(void);
void _USBNNackEvent(void);
void _USBNAlternateEvent(void);


/// usb default requests set address
void _USBNGetDescriptor(DeviceRequest *req);
void _USBNSetConfiguration(DeviceRequest *req);
void _USBNClearFeature(void);

void _USBNToggle(EPInfo* ep);
void _USBNTransmit(EPInfo* ep);
void _USBNReceive(EPInfo* ep);


void _USBNTransmitFIFO0(void);
void _USBNReceiveFIFO0(void);


unsigned char USBNRead(unsigned char Adr);
void USBNWrite(unsigned char Adr,unsigned char Data);


void _USBNInitEP0(void);

void USBNDebug(char *msg);

//only for compiler
void USBNDecodeVendorRequest(DeviceRequest *req);
void USBNDecodeClassRequest(DeviceRequest *req,EPInfo* ep);



#endif /* __USBN960X_H__ */
