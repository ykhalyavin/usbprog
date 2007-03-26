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
// Created by: Alois Flammensboeck <flammensboeck@softsprings.org>
// Created on: Sat Mar 17 18:09:49 2007
//

#ifndef _USBPROG_H_
#define _USBPROG_H_

#include <wx/string.h>
#include <exception>


enum deviceType {
	AVRUPDATE = 0x00,
	BLINKDEMO = 0x01,
	USBPROG = 0x02,
	AVRISPMKII = 0x03
};
	
enum usbprogMode {
	update,
	online
};

extern "C"
{
  #include "../lib/avrupdate.h"
}


const int UPDATE_PRODUCT_ID = 0x0c62;
const int UPDATE_VENDOR_ID = 0x1781;

const wxString ONLINE_VERSIONS_FILE = _T("http://www.ixbat.de/usbprog/versions.conf");


class UsbProgNotOpenException: std::exception
{
};
	
class UsbProgWrongModeException: std::exception
{
};

class usbprog
{
	bool opened;
	usb_dev_handle* usb_handle;
	
	public:
		usbprog();
		 ~usbprog();
	
		void open(usbprogMode mode);
		void close();
	
		usbprogMode getCurrentMode();
	
		bool getOpened();
	
		//only possible in update mode
		//device must be open
		void flashFile(wxString filename);
		
		//possible in update mode and in online mode
		//device need not to be open
		int getUSBDeviceID();
		wxString getUSBDeviceName();
		wxString getUSBDeviceName(int deviceID);
		int getVendorID();
		int getVendorID(int deviceID);
		int getProductID();
		int getProductID(int deviceID);
		
		//only possible in online mode
		//device must be open
		void startApplication();
	
	protected:
		// usbprog variables
	
		// TODO: add member variables...
	
	private:
		void checkIfOpened();
		void checkIfMode(usbprogMode mode);
};


#endif	//_USBPROG_H_
