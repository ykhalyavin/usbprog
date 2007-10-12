/* usbn960x.c
* Copyright (C) 2006  Benedikt Sauter
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "usbnapi.h"


// setup global datastructure
void USBNInit(void)
{
  // one and only device descriptor
  DeviceDescriptor.bLength=0x12; // length of device descriptor
  DeviceDescriptor.bDescriptorType=0x01;
  DeviceDescriptor.bcdUSB=0x0110;
  DeviceDescriptor.bDeviceClass=0x00;
  DeviceDescriptor.bDeviceSubClass=0x00;
  DeviceDescriptor.bDeviceProtocol=0x00;
  DeviceDescriptor.bMaxPacketSize0=0x08;
  DeviceDescriptor.idVendor=0x0000;
  DeviceDescriptor.idProduct=0x0000;
  DeviceDescriptor.bcdDevice=0x0000;
  DeviceDescriptor.iManufacturer=0x00;
  DeviceDescriptor.iProduct=0x00;
  DeviceDescriptor.iSerialNumber=0x00;
  DeviceDescriptor.bNumConfigurations=0;

  // function caller for incomming data
  rxfifos.rx1 = 0;
  rxfifos.rx2 = 0;
  rxfifos.rx3 = 0;

  txfifos.tx1 = 0;
  txfifos.tx2 = 0;
  txfifos.tx3 = 0;


  DescriptorList=NULL;
  StringList=NULL;
}



// set all device ids vendor/product/device
void USBNDeviceIDs(unsigned short idVendor, unsigned short idProduct, unsigned short bcdDevice)
{
  DeviceDescriptor.idVendor = idVendor;
  DeviceDescriptor.idProduct = idProduct;
  DeviceDescriptor.bcdDevice = bcdDevice;
}


// set vendor id
void USBNDeviceVendorID(unsigned short idVendor)
{
  DeviceDescriptor.idVendor = idVendor;
}


//set product id
void USBNDeviceProductID(unsigned short idProduct)
{
  DeviceDescriptor.idProduct = idProduct;
}


//set device id
void USBNDeviceBCDDevice(unsigned short bcdDevice)
{
  DeviceDescriptor.bcdDevice = bcdDevice;
}


// define manufacture string
void USBNDeviceManufacture (char *manufacture)
{
  int index=0;
  // add new string to string list an get actual index
  index = _USBNAddStringDescriptor(manufacture);

  // save index in device descriptor
  if(index>0)
    DeviceDescriptor.iManufacturer = index;
}



// define manufacture string
void USBNDeviceProduct (char *product)
{
  int index=0;
  // add new item to string list an get actual index
  index = _USBNAddStringDescriptor(product);

  // save index in device descriptor
  if(index>0)
    DeviceDescriptor.iProduct = index;
}


// define manufacture string
void USBNDeviceSerialNumber (char *serialnumber)
{
  int index=0;
  // add new item to string list an get actual index
  index = _USBNAddStringDescriptor(serialnumber);

  // save index in device descriptor
  if(index>0)
    DeviceDescriptor.iSerialNumber = index;
}

// define class code
void USBNDeviceClass(unsigned char classcode)
{
  DeviceDescriptor.bDeviceClass = classcode;
}

// define sub class code
void USBNDeviceSubClass(unsigned char subclasscode)
{
  DeviceDescriptor.bDeviceSubClass = subclasscode;
}

// define protocol 
void USBNDeviceProtocol(unsigned char protocolcode)
{
  DeviceDescriptor.bDeviceProtocol = protocolcode;
}

// add new configuration to device
int USBNAddConfiguration(void)
{
  int index;
  
  index = ++DeviceDescriptor.bNumConfigurations;

  struct usb_configuration_descriptor* conf =   
      (struct usb_configuration_descriptor*)malloc(sizeof(struct usb_configuration_descriptor));

  conf->bLength=sizeof(struct usb_configuration_descriptor);
  //conf->wTotalLength=sizeof(struct usb_configuration_descriptor);
  conf->wTotalLength=0x09; // will be updated when confarray has been built
  conf->bDescriptorType=0x02;
  conf->bNumInterfaces=0x00;
  conf->bConfigurationValue=index; // number of configuration
  conf->iConfiguration=0x00; // string index for configuration 
  conf->bmAttributes=0xA0;  // bus powered
  conf->MaxPower=0x1A;  // max power 

  _USBNAddToList((void*)conf, 0x09,0x02,0,0,0);

  return index;
}

// give a name the configuration
void USBNConfigurationName(int configuration, char *name)
{
  int index=0;
  // add new item to string list an get actual index
  index = _USBNAddStringDescriptor(name);
  // find configuration id  
  struct list_entry *ptr = DescriptorList;
  char *values; 	      
  while(ptr != NULL) {
    values = (char*)ptr->data;	      
    if(ptr->type==0x02 && values[5]==configuration) // if descr = confi 
    {
      values[6]=index;
    }
    ptr=ptr->next;
  }
}

void USBNConfigurationPower (int configuration, int power)
{
  // find configuration id  
  struct list_entry *ptr = DescriptorList;
  char *values; 	      
  while(ptr != NULL) {
    values = (char*)ptr->data;	      
    if(ptr->type==0x02 && values[5]==configuration) // if descr = confi 
    {
      values[8]=power/2;
    }
    ptr=ptr->next;
  }
}

// add new interface to configuration
int USBNAddInterfaceClass (int configuration, int number, char class, char subclass, char protocol)
{
  int index=0;
  // get new interface number
  struct list_entry *ptr = DescriptorList;
  char *values; 	      
  while(ptr != NULL) {
    values = (char*)ptr->data;	      
    if(values[5]==(char)configuration && ptr->type==0x02) // if descr = confi 
    {
      values[4]++;
      index =  values[4];
    }
    ptr=ptr->next;
  }
  
  struct usb_interface_descriptor* interf;
  interf = (struct usb_interface_descriptor*)malloc(sizeof(struct usb_interface_descriptor));
  interf->bLength = 0x09;  
  interf->bDescriptorType     = INTERFACE;
  interf->bInterfaceNumber     = number; // FIXME
  interf->bAlternateSetting   = 0x00;
  interf->bNumEndpoints       = 0x00;
  interf->bInterfaceClass     = class;
  interf->bInterfaceSubClass   = subclass;
  interf->bInterfaceProtocol   = protocol;
  interf->iInterface           = 0x00;
  
  _USBNAddToList((void*)interf, 0x09,INTERFACE,(uint8_t)configuration,0,index);
  return index;
}



// add new interface to configuration
int USBNAddInterface (int configuration, int number)
{
  int index=0;
  // get new interface number
  struct list_entry *ptr = DescriptorList;
  char *values; 	      
  while(ptr != NULL) {
    values = (char*)ptr->data;	      
    if(values[5]==(char)configuration && ptr->type==0x02) // if descr = confi 
    {
      values[4]++;
      index =  values[4];
    }
    ptr=ptr->next;
  }
  
  struct usb_interface_descriptor* interf;
  interf = (struct usb_interface_descriptor*)malloc(sizeof(struct usb_interface_descriptor));
  interf->bLength = 0x09;  
  interf->bDescriptorType     = INTERFACE;
  interf->bInterfaceNumber     = number; // FIXME
  interf->bAlternateSetting   = 0x00;
  interf->bNumEndpoints       = 0x00;
  interf->bInterfaceClass     = 0x00;
  interf->bInterfaceSubClass   = 0x00;
  interf->bInterfaceProtocol   = 0x00;
  interf->iInterface           = 0x00;
  
  _USBNAddToList((void*)interf, 0x09,INTERFACE,(uint8_t)configuration,0,index);
  return index;
}


void USBNInterfaceName(int configuration, int interface, char *name)
{
  int index=0;
  // add new item to string list an get actual index
  index = _USBNAddStringDescriptor(name);
  // find configuration id  
  struct list_entry *ptr = DescriptorList;
  char *values; 	      
  while(ptr != NULL) {
    values = (char*)ptr->data;	      
    //if(ptr->type==0x04 && ptr->conf==configuration && values[2]==interface) // if descr = confi 
    if(ptr->type==0x04 && ptr->conf==configuration && ptr->index==interface) // if descr = confi 
    {
      values[8]=index;
    }
    ptr=ptr->next;
  }
}

// TODO
void USBNAlternateSetting(int configuration, int interface, int setting)
{
  // find configuration id  
  struct list_entry *ptr = DescriptorList;
  char *values; 	      
  while(ptr != NULL) {
    values = (char*)ptr->data;	      
    if(ptr->type==0x04 && ptr->conf==configuration && ptr->index==interface) // if descr = confi 
    {
      values[3]=setting;
    }
    ptr=ptr->next;
  }
}

// add out endpoint
void USBNAddOutEndpoint(int configuration, int interface, int epnr, 
			int epadr, char attr, int fifosize, int intervall, void *fkt)
{
  // FIXME (must be defined when set configuration is called)
  switch(epnr)
  {
    case 1: rxfifos.rx1=1; rxfifos.func1 = fkt; break;
    case 2: rxfifos.rx2=1; rxfifos.func2 = fkt; break;
    case 3: rxfifos.rx3=1; rxfifos.func3 = fkt; break;
  }

  _USBNAddEndpoint(configuration,interface,epnr,epadr,attr,fifosize,intervall);
}

// add in endpoint
void USBNAddInEndpoint(int configuration, int interface, int epnr, 
			int epadr,char attr, int fifosize, int intervall, void *fkt)
{
  // FIXME (must be defined when set configuration is called)
  if(fkt!=NULL)
  {
    switch(epnr)
    {
      case 1: txfifos.tx1=1; txfifos.func1 = fkt; break;
      case 2: txfifos.tx2=1; txfifos.func2 = fkt; break;
      case 3: txfifos.tx3=1; txfifos.func3 = fkt; break;
    }
  }

  _USBNAddEndpoint(configuration,interface,epnr,epadr+0x80,attr,fifosize,intervall);
}

void _USBNAddEndpoint(int configuration, int interface, int epnr, int epadr,char attr, int fifosize, int intervall)
{

  // update  interface endpoint list
  struct list_entry *ptr = DescriptorList;
  char *values; 	      
  while(ptr != NULL) {
    values = (char*)ptr->data;	      
    if(configuration==ptr->conf && ptr->index==interface && ptr->type==0x04) // if descr = confi 
      values[4]++;
    ptr=ptr->next;
  }
  

  struct usb_endpoint_descriptor* endpoint;
  endpoint = (struct usb_endpoint_descriptor*)malloc(sizeof(struct usb_endpoint_descriptor));
  endpoint->bLength=sizeof(struct usb_endpoint_descriptor);
  endpoint->bDescriptorType   = ENDPOINT;
  endpoint->bEndpointAddress   = (unsigned char)epadr;
  endpoint->bmAttributes       = attr;
  endpoint->wMaxPacketSize    = (short int)fifosize;  
  endpoint->bIntervall        = (unsigned char)intervall;  
  
  _USBNAddToList((void*)endpoint, 0x07,ENDPOINT,
		    (uint8_t)configuration,(uint8_t)interface,0);
}

// *******************************************************************
// put configuration/interface and ep descr in a linear field
// ********************************************************************
void _USBNCreateConfDescrField(void)
{
  struct list_entry *ptr;
  
  int conf=0;
  int i,j,x;
  int desclen=0x09;
  int index,len;
  char *values=NULL;
  char *actualconf=NULL;


  // number of configurations
  ptr = DescriptorList;
  while(ptr != NULL) {
    if(ptr->type==0x02)
      conf++;
    ptr=ptr->next;
  }
  
  // now get the array with the numbers of configurations 
  FinalConfigurationArray = (char**)malloc(sizeof(char*)*conf);

  // get space and copy configuration to finalarray
  // length for every configuration
  for(i=1;i<=conf;i++)
  {
    ptr = DescriptorList;
    desclen=0x09;
    while(ptr != NULL) 
    {
      values = (char*)ptr->data;	      
      if(ptr->conf==i)
	desclen=desclen+ptr->len;

      // bConfigurationValue = value[5] = number of configuration
      if((values[5]==(char)i) && (ptr->type==0x02))
      {
	actualconf = values;
      }	
      ptr=ptr->next;
    }
    // reserve memory for every configuration descriptor
    FinalConfigurationArray[conf-1] = (char*)malloc(sizeof(char)*desclen);
    
    // configuration descriptor
    
    len = 9; 
    for(index=0;index<len;index++)
      FinalConfigurationArray[conf-1][index] = actualconf[index];
      
    // update configuration total length
    FinalConfigurationArray[conf-1][2] = desclen+9; // FIXME
  }

  // next index start`s here ( confi descr is always 9 signs long )
  index = 9;
  int numofinterf=0;
  struct list_entry *findep;
  char *endpoint;
  // copy interface and endpoint descr
  for(i=1;i<=conf;i++)
  {
    // get number of interfaces
    ptr = DescriptorList;
    while(ptr != NULL) 
    {
      values = (char*)ptr->data;	      
      if(values[1]==0x02 && values[5]==i)
      {
	numofinterf = values[4];
	break;
      }
      ptr=ptr->next;
    }

    // go through interfaces
    for(j=1;j<=numofinterf;j++)
    {
      // find interface
      ptr = DescriptorList;
      while(ptr != NULL) 
      {
	values = (char*)ptr->data;	      
	//if(values[1]==0x04 && values[2]==j && ptr->conf==i)
	if(values[1]==0x04 && ptr->index==j && ptr->conf==i)
	{
	  // *** copy interface
	  for(x=0;x<9;x++)
	  {
	    FinalConfigurationArray[i-1][index] = values[x];
	    index++;
	  }
	  // get number of endpoints = values[4]
	  //for(k=0;k<values[4];k++)
	  //{
	    // *** copy endpoints 
	    findep = DescriptorList;
	    while(findep!=NULL)
	    {
	      if(findep->type==0x05 && findep->conf==i && findep->interf==j)
	      {
		//USBNDebug("copy ep\r\n");
		endpoint = (char*)findep->data;	      
		for(x=0;x<7;x++)
		{
		  FinalConfigurationArray[i-1][index] = endpoint[x];
		  index++;
		}
		
	      }
	      findep = findep->next;
	    }
	  
	  //}
	}
	ptr=ptr->next;
      }
    }

  }

//  for(i=0;i<32;i++)
//    SendHex(FinalConfigurationArray[0][i]);
}



void _USBNCreateStringField(void)
{
  struct string_entry *tmp;
  int index=0;
  
  tmp = StringList;
  while(tmp->next != NULL)
  {
    tmp=tmp->next;
    index++;
  } 
  // memory for all string descriptors
  index++;
  FinalStringArray = (char**)malloc(sizeof(char*)*index);

  int x;
  char *ptr;
  tmp = StringList;
  //USBNDebug("\r\n\r\n");
  int first=1;
  do
  {
    if(!first) 
      tmp=tmp->next;
    first=0;

    ptr = (char*)tmp->data;
    //USBNDebug("string index: ");
    //SendHex(tmp->index);
    //USBNDebug(" length: ");
    //SendHex(ptr[0]);
    //USBNDebug("\r\n\r\n");
    FinalStringArray[tmp->index] = (char*)malloc(sizeof(char)*(int)ptr[0]);

     for(x=0;x<(int)ptr[0];x++)
       FinalStringArray[tmp->index][x] = ptr[x];


  } while(tmp->next != NULL);

}
// ********************************************************************
// start USB Chip
// ********************************************************************

void USBNStart(void)
{
  _USBNCreateConfDescrField();
  _USBNCreateStringField();

  _USBNInitEP0();
                                    // usbn starts here

  USBNWrite(MCNTRL,SRST);           // clear all registers
  while(USBNRead(MCNTRL)&SRST);

  USBNWrite(CCONF, 0x80);           // clock output off, divisor to 4 MHz
  USBNWrite(NAKMSK,0xFF);
  USBNWrite(NAKMSK,NAK_OUT0);
  USBNWrite(FAR,AD_EN+0x00);            // set default address
  USBNWrite(EPC0,DEF);
  USBNWrite(TXC0,FLUSH);            // FLUSHTX0;

  USBNWrite(RXC0,RX_EN+FLUSH);            // enable EP0 receive
  //USBNWrite(RXC1,RX_EN);            // enable EP0 receive
 
  USBNWrite(RXMSK, RX_FIFO0+RX_FIFO1+RX_FIFO2+RX_FIFO3);            // data incoming EP0
  USBNWrite(TXMSK, TX_FIFO0+TX_FIFO1+TX_FIFO2+TX_FIFO3);            // data incoming EP0
 
  USBNWrite(ALTMSK, ALT_RESET+ALT_SD3+ALT_EOP);
  USBNWrite(MAMSK, (INTR_E+RX_EV+ALT+TX_EV+NAK) );
 
  
  USBNWrite(NFSR,OPR_ST);
  USBNWrite(MCNTRL, VGE+NAT+INT_L_P);     // VGE, no NAT, interrupt on high
 
  return ;
}


// ********************************************************************
// Interrupt Routine for USBN960x
// ********************************************************************


void USBNInterrupt(void)
{
  UARTWrite("irq\r\n");
  unsigned char maev,mask;
  
  maev = USBNRead(MAEV);

  if(maev & RX_EV)  _USBNReceiveEvent();
  else if(maev & TX_EV) _USBNTransmitEvent();
  else if(maev & ALT)   _USBNAlternateEvent();
  else if(maev & NAK)   _USBNNackEvent();

  mask = USBNRead(MAMSK);
  USBNWrite(MAMSK,0x00);                  // disable irq
  USBNWrite(MAMSK,mask);

}


// ********************************************************************
// helper functions 
// ********************************************************************


// string descriptor functions 
int _USBNAddStringDescriptor(char *string)
{
  int i;
  int j=2;
  int index;
  
  
  char *newstring = (char*)malloc(strlen(string)*2+2);
  
  newstring[0]=(strlen(string)*2)+2; // length;
  newstring[1]=0x03; //Descriptor Type

  // build string like it is defined in usb spec
  for(i=0;i<strlen(string);i++)
  {
    newstring[j]=string[i];
    newstring[j+1]=0x00;
    j = j+2;
  }

  index = USBNAddToStringList((void*)newstring);

  // return index   
  return index;
}



uint8_t USBNAddToStringList( void* data)
{
  uint8_t index=0;
  struct string_entry *tmp;

  //USBNDebug("add to string list\r\n");
  if(StringList == NULL)
  {
    //USBNDebug("first string elemennt\r\n");
    StringList = (struct string_entry*)malloc(sizeof(struct string_entry));

    StringList->data = data;
    StringList->index = 0;
    StringList->next = NULL;
  }
  else
  {
    //USBNDebug("next string element\r\n");
    
    index++;
    tmp = StringList;
    while(tmp->next != NULL)
    {
      tmp=tmp->next;
      index++;
    } 
    tmp->next = (struct string_entry*)malloc(sizeof(struct string_entry));
    tmp=tmp->next;

    tmp->data = data;
    tmp->index = index;
    tmp->next=NULL;
  }
  //SendHex(index);    
  return index;
}




void _USBNAddToList( void* data, uint8_t len,uint8_t type,uint8_t conf,uint8_t interf,uint8_t index)
{
  struct list_entry *tmp;
  //USBNDebug("add to list\r\n");
  if(DescriptorList == NULL)
  {
    //USBNDebug("first elemennt\r\n");
    DescriptorList = (struct list_entry*)malloc(sizeof(struct list_entry));

    DescriptorList->data = data;
    DescriptorList->len = len;
    DescriptorList->next = NULL;
    DescriptorList->type = type;
    DescriptorList->conf = conf;
    DescriptorList->interf = interf;
    DescriptorList->index = index;
  }
  else
  {
    //USBNDebug("next elemennt\r\n");
    tmp = DescriptorList;

    while(tmp->next != NULL)
      tmp=tmp->next;

    tmp->next = (struct list_entry*)malloc(sizeof(struct list_entry));
  
    tmp=tmp->next;
    tmp->data = data;
    tmp->len = len;
    tmp->type = type;
    tmp->conf = conf;
    tmp->interf = interf;
    tmp->index = index;
    tmp->next=NULL;
  }
}




