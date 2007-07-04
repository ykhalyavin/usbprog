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

#ifndef _USBDEVICES_H_
#define _USBDEVICES_H_
#include <wx/string.h>
#include <vector>

extern "C"
{
	#include <usb.h>
}

struct USBDeviceInfo {
		int VendorID;
		int ProductID;
	    int BcdDevice;
		wxString Manufacturer;
		wxString Product;
};


class USBDevices
{
	std::vector<USBDeviceInfo*> devices;
	
	public:
		USBDevices();
		 ~USBDevices();
		 
		int Count(){return this->devices.size();}
		
		USBDeviceInfo *operator[](unsigned int position){return this->devices[position];}
		
		void Update();
		
		
	
	protected:
		 
		
	private:

		void ClearUSBDevices();
		
	
};


#endif	//_USBDEVICES_H_
