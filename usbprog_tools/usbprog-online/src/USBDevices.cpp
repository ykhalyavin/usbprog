/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


//
// Class: USBDevices
// Created by: Alois Flammensböck <flammensboeck@softsprings.org>
// Created on: Tue Jul 03 18:12 2007
//
#include "USBDevices.h"
#include <wx/log.h>
#include <stdexcept>

USBDevices::USBDevices()
{
	// TODO: put constructor code here
}


USBDevices::~USBDevices()
{
	ClearUSBDevices();
}

void USBDevices::Update()
{
	ClearUSBDevices();
	struct usb_bus *busses;

	//usb_set_debug(2);
	usb_init();
	usb_find_busses();
	usb_find_devices();
	
	busses = usb_get_busses();
	
	struct usb_dev_handle* usb_handle;
	struct usb_bus *bus;
	
	
	unsigned char send_data=0xff;

    for (bus = busses; bus; bus = bus->next)
    {
		struct usb_device *dev;
		wxString a;
		for (dev = bus->devices; dev; dev = dev->next){
		  
			if (dev->descriptor.idVendor > 0){
				
				char * manufacturer  = new char[255];
				char * product = new char[255];
				
				usb_handle = usb_open(dev);
				
				int stat = usb_set_configuration (usb_handle,dev->config[0].bConfigurationValue);
				//wxLogInfo(_T("stat:%d from usb_set_config to %x"),stat,dev->config[0].bConfigurationValue);
		
				stat = usb_claim_interface(usb_handle, 0);
			//	wxLogInfo(_T("stat:%d from claim"),stat);
		
				stat = usb_set_altinterface(usb_handle,0);
				//wxLogInfo(_T("alt_stat=%d"),stat);
				int manufacturerLen = usb_get_string_simple(usb_handle, 1, manufacturer, 255);
				int productLen = usb_get_string_simple(usb_handle, 2, product, 255);
			//	wxLogInfo(_T("descrLen: %d"),descrLen);
				USBDeviceInfo *info = new USBDeviceInfo;
				devices.push_back(info);
				info->VendorID = dev->descriptor.idVendor;
				info->ProductID = dev->descriptor.idProduct;
				info->BcdDevice = dev->descriptor.bcdDevice;
				info->Manufacturer = wxString(manufacturer,wxConvLocal,manufacturerLen);
				info->Product = wxString(product,wxConvLocal,productLen);
				usb_close(usb_handle);
				delete manufacturer;
				delete product;
			}
		}
	}  
	
}

void USBDevices::ClearUSBDevices()
{
	for (int i=0;i < devices.size();i++){
		delete devices[i];
	}
	devices.clear();
}
