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
// Class: usbprog
//
// Created by: Alois Flammensböck <flammensboeck@softsprings.org>
// Created on: Sat Mar 17 18:09:49 2007
//

#include "usbprog.h"
#include "wx/log.h"

usbprog::usbprog()
{
	// TODO: put constructor code here
}


usbprog::~usbprog()
{
	// TODO: put destructor code here
}

void usbprog::open()
{
	int deviceID = getUSBDeviceID();
	usb_handle = avrupdate_open(getVendorID(deviceID),getProductID(deviceID));
	opened = true;
}
void usbprog::close()
{
	checkIfOpened();
	avrupdate_close(usb_handle);
	opened = false;
}
void usbprog::flashFile(wxString filename)
{
	bool wasOpen = opened;
	if (opened) {
		close();
	}
	
	wxLogInfo(_T("Activate Update Mode"));
	avrupdate_start_with_vendor_request(UPDATE_VENDOR_ID, UPDATE_PRODUCT_ID);
	wxLogInfo(_T("Update Mode is running"));
	sleep(3);
	
	//at the moment we have to copy the filename string to provide
	//a non const char* as filename
	int wxchar_len = filename.Len()+1; 
	int char_len = wxchar_len*sizeof(wxChar)/sizeof(char);

	char * buf  = new char[char_len];
	memcpy( buf , filename.c_str() , char_len*sizeof(char) ); 
	wxLogInfo(_T("Starting to flash %s"),filename.c_str());
	avrupdate_flash_bin(usb_handle,buf);
	wxLogInfo(_T("Completed flashing"),filename.c_str());
	sleep(3);
	
	wxLogInfo(_T("Deactivate Update Mode"));
	open();
		
	wxLogInfo(_T("Starting Application"));
	avrupdate_startapp(usb_handle);
	wxLogInfo(_T("Application started"));
	if (!wasOpen) {
		close();
	}
}
void usbprog::startApplication()
{	
	checkIfOpened();
	avrupdate_startapp(usb_handle);
}

int usbprog::getUSBDeviceID()
{
	return avrupdate_find_usbdevice();
}

void usbprog::checkIfOpened()
{
	if (!opened){
		wxLogError(_T("The USB-Device is not open"));
	}
}
wxString usbprog::getUSBDeviceName()
{
	return getUSBDeviceName(getUSBDeviceID());
}

wxString usbprog::getUSBDeviceName(int deviceID)
{
	switch(deviceID){
		case AVRISPMKII:
			return _T("AVRISP mkII Klon");
		break;

		case USBPROG:
			return _T("usbprog (Benes ISP + RS232 Interface)");
		break;
			
		case AVRUPDATE:
			return _T("usprog Adapter with no firmware");
		break;
			
		case BLINKDEMO:
			return _T("Blink Demo");
		break;

		default:
			return _T("Unknown");
	}
}

int usbprog::getVendorID()
{
	return getVendorID(getUSBDeviceID());
}

int usbprog::getVendorID(int deviceID)
{
	switch(deviceID){
		case AVRISPMKII:
			return 0x03eb;
		break;

		case USBPROG:
			return 0x1781;
		break;
			
		case AVRUPDATE:
			return 0x1781;
		break;
			
		case BLINKDEMO:
			return 0x1781;
		break;

		default:
			return 0;
	}
}

int usbprog::getProductID()
{
	return getProductID(getUSBDeviceID());
}


int usbprog::getProductID(int deviceID)
{
	switch(deviceID){
		case AVRISPMKII:
			return 0x2104;
		break;

		case USBPROG:
			return 0x0c62;
		break;
			
		case AVRUPDATE:
			return 0x0c62;
		break;
			
		case BLINKDEMO:
			return 0x0c62;
		break;

		default:
			return 0;
	}
}
