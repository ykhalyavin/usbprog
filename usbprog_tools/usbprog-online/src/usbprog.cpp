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
// Created by: Alois Flammensboeck <flammensboeck@softsprings.org>
// Created on: Sat Mar 17 18:09:49 2007
//

#include "usbprog.h"
#include "wx/log.h"
#include <stdexcept> //runtime_error

#define __WXDEBUG__

using namespace std;

using std::runtime_error;

usbprog::usbprog()
{
	// TODO: put constructor code here
}


usbprog::~usbprog()
{
	// TODO: put destructor code here
}

void usbprog::open(usbprogMode mode)
{
	if (!opened){
		
		int deviceID = getUSBDeviceID();
		
		if (mode == update) {
			wxLogDebug(_T("Try to switch usbprog to update mode"));
			if (deviceID != AVRUPDATE){
				wxLogInfo(_T("usbprog found with: %s"),getUSBDeviceName(deviceID).c_str());
				wxLogInfo(_T("start update mode"));
				avrupdate_start_with_vendor_request(getVendorID(deviceID),getProductID(deviceID));
				#if _WIN32
				Sleep(7000);
				#else
				sleep(3);
				#endif
				
				//now the deviceID should be AVRUPDATE
				deviceID = getUSBDeviceID();
				if (deviceID != AVRUPDATE) {
						throw runtime_error("Could not switch usbprog to update mode");
				}
			}else{
				wxLogDebug(_T("usbprog is already in update mode"));
			}
		}
				
		usb_handle = avrupdate_open(getVendorID(deviceID),getProductID(deviceID));
		
		opened = true;
	}else{
		throw runtime_error("usbprog already opened");
	}
}

void usbprog::close()
{
	checkIfOpened();
	avrupdate_close(usb_handle);
	opened = false;
}

usbprogMode usbprog::getCurrentMode()
{
	if (getUSBDeviceID() != AVRUPDATE)
		return online;
	else
		return update;
	
}

bool usbprog::getOpened(){
	return opened;
}
	
void usbprog::flashFile(wxString filename)
{
	checkIfOpened();
	checkIfMode(update);
	
	
	//at the moment we have to copy the filename string to provide
	//a non const char* as filename
	int wxchar_len = filename.Len()+1; 
	int char_len = wxchar_len*sizeof(wxChar)/sizeof(char);

	char * buf  = new char[char_len];
	try{
		memcpy( buf, filename.mb_str() , char_len*sizeof(char) ); 
	
		wxLogInfo(_T("Starting to flash %s"),buf);
		avrupdate_flash_bin(usb_handle,buf);
	}catch(...){
		delete(buf);
		throw;
	}
	delete(buf);
	
	sleep(3);
	wxLogInfo(_T("Completed flashing"),filename.c_str());
}

void usbprog::startApplication()
{	
	checkIfOpened();
	checkIfMode(update);
	avrupdate_startapp(usb_handle);
}

int usbprog::getUSBDeviceID()
{
	int deviceID = avrupdate_find_usbdevice();
	if (deviceID >= 0){
		return deviceID;
	}else{
		throw runtime_error("Could not get USB Device ID");
	}
}

void usbprog::checkIfOpened()
{
	if (!opened){
		wxLogError(_T("The USB-Device is not open"));
		throw UsbProgNotOpenException();
	}	
}
	
void usbprog::checkIfMode(usbprogMode mode)
{	
	if (mode != getCurrentMode()) {
		wxLogError(_T("The USB-Device is in wrong Mode."));
		throw UsbProgWrongModeException();
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
			return _T("usprog Adapter in Update Mode");
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
