/***************************************************************
 * Name:      usbprogonlineMain.h
 * Purpose:   Defines Application Frame
 * Author:    Alois Flammensboeck (flammenboeck@softsprings.org)
 * Created:   2007-03-09
 * Copyright: Alois Flammensboeck (http://www.softsprings.org)
 * License:
 **************************************************************/

#ifndef USBPROGONLINEMAIN_H
#define USBPROGONLINEMAIN_H

#include "usbprogonlineApp.h"
#include <wx/log.h>

#include "MainFrameGenerated.h"
#include "OnlineVersions.h"
#include "USBDevices.h"
#include <exception>

extern "C"
{
  #include "../lib/avrupdate.h"
}

const wxString ONLINE_VERSIONS_FILE = _T("http://www.ixbat.de/usbprog/versions.conf");

class MainFrame: public MainFrameGenerated
{
	wxBitmap* hardware;
    OnlineVersions onlineVersions;
	USBDevices	usbDevices;
	USBDeviceInfo* deviceInfo;
	public:
		MainFrame(wxFrame *frame);
		~MainFrame();
	private:
		virtual void OnClose(wxCloseEvent& event);
		virtual void OnQuit(wxCommandEvent& event);
		void RefreshUSBDevices();
		void RefreshOnlineVersions();
		void SwitchToAppLog();
		void SwitchToAdapterSelection();
	    void StartSession();
	    void CloseSession();
		void UpdateFirmware(wxString filename);
    protected:
  		virtual void OnBtnRefreshListClick( wxCommandEvent& event );
  		virtual void OnBtnConnectClick( wxCommandEvent& event );
		virtual void OnBtnFlashClick( wxCommandEvent& event );
		virtual void OnBtnRefreshOnlineVersionsClick( wxCommandEvent& event );	
		virtual void OnBtnSelectFileClick( wxCommandEvent& event );
		virtual void OnBtnFlashLocalFile( wxCommandEvent& event );
		virtual void OnBtnClearLog( wxCommandEvent& event );
		virtual void OnListCtrlOnlineVersionsItemDeselected( wxListEvent& event );
		virtual void OnListCtrlOnlineVersionsItemSelected( wxListEvent& event );

};

#endif // usbprogonlineMAIN_H
