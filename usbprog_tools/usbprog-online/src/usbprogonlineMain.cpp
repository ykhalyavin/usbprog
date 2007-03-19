/***************************************************************
 * Name:      usbprogonlineMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Alois Flammensboeck (flammenboeck@softsprings.org)
 * Created:   2007-03-09
 * Copyright: Alois Flammensboeck (http://www.softsprings.org)
 * License:
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "usbprogonlineMain.h"
#include "wx/protocol/http.h"
#include <iostream>
#include <wx/filename.h>

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}


usbprogonlineFrame::usbprogonlineFrame(wxFrame *frame)
	: GUIFrame(frame)
{
#if wxUSE_STATUSBAR
//	statusBar->SetStatusText(_("Hello Code::Blocks user!"), 0);
//	statusBar->SetStatusText(wxbuildinfo(short_f), 1);
#endif

		wxLog *logger=new wxLogTextCtrl(text_ctrl_3);
  		wxLog::SetActiveTarget(logger);
  		wxLog::SetVerbose(true);
  		
		listCtrlOnlineVersions->InsertColumn(0,_T("Nr."),wxLIST_FORMAT_LEFT, 30);
		listCtrlOnlineVersions->InsertColumn(1,_T("Application"),wxLIST_FORMAT_LEFT, 120);
		listCtrlOnlineVersions->InsertColumn(2,_T("Vers."),wxLIST_FORMAT_LEFT, 40);
		listCtrlOnlineVersions->InsertColumn(3,_T("Description"),wxLIST_FORMAT_LEFT,180);
		
		teUSBUpdateVID->SetValue(wxString::Format(_T("0x%4X"),UPDATE_VENDOR_ID));
		teUSBUpdatePID->SetValue(wxString::Format(_T("0x%4X"),UPDATE_PRODUCT_ID));
		lblCurrentFirmware->SetLabel(_T(""));
		
}

usbprogonlineFrame::~usbprogonlineFrame()
{
}

void usbprogonlineFrame::OnClose(wxCloseEvent &event)
{
    Destroy();
}

void usbprogonlineFrame::OnQuit(wxCommandEvent &event)
{
    Destroy();
}

void usbprogonlineFrame::OnAbout(wxCommandEvent &event)
{
    wxString msg = wxbuildinfo(long_f);
    wxMessageBox(msg, _("Welcome to..."));
}

void usbprogonlineFrame::OnBtnFindAdapterClick( wxCommandEvent& event )
{
	int deviceID = usbProg.getUSBDeviceID();
	lblCurrentFirmware->SetLabel(usbProg.getUSBDeviceName(deviceID));
	
	teUSBDeviceVID->SetValue(wxString::Format(_T("0x%4X"),usbProg.getVendorID(deviceID)));
	teUSBDevicePID->SetValue(wxString::Format(_T("0x%4X"),usbProg.getProductID(deviceID)));
	
}

void usbprogonlineFrame::OnBtnDownloadClick( wxCommandEvent& event ){
   // wxString url = teVersionsFileURL->GetLineText(0);
    wxString url = teVersionsFileURL->GetValue();
  	try{
	    onlineVersions.Update(url);
	
    
    	for (int i =0; i < onlineVersions.Count();i++){
   			wxString nr;
			nr << i;
			int itemIndex;
			itemIndex = listCtrlOnlineVersions->InsertItem(listCtrlOnlineVersions->GetItemCount(),nr);
			//Application Name
			listCtrlOnlineVersions->SetItem(itemIndex,1,onlineVersions[i]->Name);
		
			//Application Version
			listCtrlOnlineVersions->SetItem(itemIndex,2,onlineVersions[i]->Version);
		
			//Application Description
			listCtrlOnlineVersions->SetItem(itemIndex,3,onlineVersions[i]->Description);
    
    
   		}
   	}catch(std::exception &e){
		wxLogError(_T("%s"), e.what());
   	}
}


 void usbprogonlineFrame::OnBtnCancelClick( wxCommandEvent& event )
 {
 }
 
 void usbprogonlineFrame::OnBtnQuitClick( wxCommandEvent& event )
 {
 }
 
void usbprogonlineFrame::OnBtnFlashClick( wxCommandEvent& event )
{
	if (listCtrlOnlineVersions->GetSelectedItemCount() == 1){
		wxString tmpFileName = wxFileName::CreateTempFileName(_T("firmware"));
		long firstSelectedItem =-1;
		firstSelectedItem = listCtrlOnlineVersions->GetNextItem(firstSelectedItem,
                                     wxLIST_NEXT_ALL,
                                     wxLIST_STATE_SELECTED);
        onlineVersions.DownloadOnlineVersion(firstSelectedItem,tmpFileName);
		wxLogInfo(_T("Saving online version to %s"),tmpFileName.c_str());
        usbProg.open();
        usbProg.flashFile(tmpFileName);
        usbProg.close();
        OnBtnFindAdapterClick(event)  ;                    
	} else{
		wxLogError(_T("One File has to be selected to download"));
	}
	
}

