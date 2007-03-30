/***************************************************************
 * Name:      usbprogonlineMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Alois Flammensboeck (flammensboeck@softsprings.org)
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

#include "MainFrame.h"
#include "wx/protocol/http.h"
#include <iostream>
#include <wx/filename.h>
#include "../images/hardware.xpm"


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


MainFrame::MainFrame(wxFrame *frame)
	: MainFrameGenerated(frame)
{

		wxLog *logger=new wxLogTextCtrl(textCtrlAppLog);
  		wxLog::SetActiveTarget(logger);
  		wxLog::SetVerbose(true);
  		
		listCtrlOnlineVersions->InsertColumn(0,_T("Nr."),wxLIST_FORMAT_LEFT, 30);
		listCtrlOnlineVersions->InsertColumn(1,_T("Application"),wxLIST_FORMAT_LEFT, 120);
		listCtrlOnlineVersions->InsertColumn(2,_T("Vers."),wxLIST_FORMAT_LEFT, 40);
		listCtrlOnlineVersions->InsertColumn(3,_T("Description"),wxLIST_FORMAT_LEFT,330);
	
		btnDownloadAndFlash->Enable(false);
		
		//teUSBUpdateVID->SetValue(wxString::Format(_T("0x%4X"),UPDATE_VENDOR_ID));
		//teUSBUpdatePID->SetValue(wxString::Format(_T("0x%4X"),UPDATE_PRODUCT_ID));
		lblCurrentFirmware->SetLabel(_T(""));
		hardware = new wxBitmap((const char **) hardware_xpm);
		m_bitmap1->SetBitmap(*hardware);
		FindAdapter();
		RefreshOnlineVersions();
}

MainFrame::~MainFrame()
{
		delete hardware;
}

void MainFrame::OnClose(wxCloseEvent &event)
{
    Destroy();
}

void MainFrame::OnQuit(wxCommandEvent &event)
{
    Destroy();
}

void MainFrame::FindAdapter()
{
	try{
		int deviceID = usbProg.getUSBDeviceID();
		if (deviceID >= 0){
			lblConnectionStatus->SetLabel(_T("Connected to usbprog-adapter"));
			lblCurrentFirmware->SetLabel(usbProg.getUSBDeviceName(deviceID));
			panelOnline->Enable(true);
			panelLocal->Enable(true);
		}else{
			lblConnectionStatus->SetLabel(_T("NOT Connected to usbprog-adapter"));
			lblCurrentFirmware->SetLabel(wxEmptyString);
			panelOnline->Enable(false);
			panelLocal->Enable(false);
		}
		
		//teUSBDeviceVID->SetValue(wxString::Format(_T("0x%4X"),usbProg.getVendorID(deviceID)));
		//teUSBDevicePID->SetValue(wxString::Format(_T("0x%4X"),usbProg.getProductID(deviceID)));
	}catch(std::exception &e){
		wxLogError(_T("%s"), e.what());
		SwitchToAppLog();
	}
	
}

void MainFrame::SwitchToAppLog()
{
	notebookSource->SetSelection(3);
}

void MainFrame::OnBtnFindAdapterClick( wxCommandEvent& event )
{
	FindAdapter();	
}



void MainFrame::OnBtnSelectFileClick( wxCommandEvent& event )
{
	wxFileDialog fileDialog(this);
	fileDialog.ShowModal();
	textCtrlFileName->SetValue(fileDialog.GetPath());
}


void MainFrame::RefreshOnlineVersions()
{   
	// wxString url = teVersionsFileURL->GetLineText(0);
  	try{
	    onlineVersions.Update(ONLINE_VERSIONS_FILE);
	
    	//clear the listCtrlOnlineVersions
		listCtrlOnlineVersions->DeleteAllItems();
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
	

void MainFrame::OnBtnRefreshOnlineVersionsClick( wxCommandEvent& event )
{
	RefreshOnlineVersions();
}

 
void MainFrame::OnBtnFlashClick( wxCommandEvent& event )
{
	
  	try{
		if (listCtrlOnlineVersions->GetSelectedItemCount() == 1){
			SwitchToAppLog();
			wxString tmpFileName = wxFileName::CreateTempFileName(_T("firmware"));
			long firstSelectedItem =-1;
			firstSelectedItem = listCtrlOnlineVersions->GetNextItem(firstSelectedItem,
										 wxLIST_NEXT_ALL,
										 wxLIST_STATE_SELECTED);
			onlineVersions.DownloadOnlineVersion(firstSelectedItem,tmpFileName);
			wxLogInfo(_T("Saved online version to %s"),tmpFileName.c_str());
			
			
			if (usbProg.getOpened()){
				usbProg.close();
			}
			usbProg.open(update);
			usbProg.flashFile(tmpFileName);
			usbProg.startApplication();
			usbProg.close();
			
			OnBtnFindAdapterClick(event)  ;     
			wxLogInfo(_T("Deleting temporary file %s"),	tmpFileName.c_str());
			wxRemoveFile(tmpFileName);	
		} else{
			wxLogError(_T("One File has to be selected to download"));
		}
	}catch(std::exception &e){
		wxLogError(_T("%s"), e.what());
   	}	
}


void MainFrame::OnBtnFlashLocalFile( wxCommandEvent& event )
{
	try{
		wxString fileName = textCtrlFileName->GetLineText(0);
		if (wxFile::Exists(fileName)){
			SwitchToAppLog();
			if (usbProg.getOpened()){
				usbProg.close();
			}
			usbProg.open(update);
			usbProg.flashFile(fileName);
			usbProg.startApplication();
			usbProg.close();
			
			OnBtnFindAdapterClick(event)  ;
		}			
	}catch(std::exception &e){
		wxLogError(_T("%s"), e.what());
   	}	
}
void MainFrame::OnBtnClearLog( wxCommandEvent& event )
{
	textCtrlAppLog->Clear();
}

void MainFrame::OnListCtrlOnlineVersionsItemDeselected( wxListEvent& event )
{
	if (listCtrlOnlineVersions->GetSelectedItemCount()>0){
		btnDownloadAndFlash->Enable(true);
	}else{
		btnDownloadAndFlash->Enable(false);
	}
}
void MainFrame::OnListCtrlOnlineVersionsItemSelected( wxListEvent& event )
{
	if (listCtrlOnlineVersions->GetSelectedItemCount()>0){
		btnDownloadAndFlash->Enable(true);
	}else{
		btnDownloadAndFlash->Enable(false);
	}
}
