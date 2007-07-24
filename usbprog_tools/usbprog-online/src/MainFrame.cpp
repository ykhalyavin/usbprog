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
#include "AboutDialog.h"

#include <iostream>
#include <wx/filename.h>
#include "../images/hardware.xpm"
#include "../images/IconAbout24.xpm"
#include <stdexcept>


using namespace std;

using std::runtime_error;


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
	
		listCtrlUSBDevices->InsertColumn(2,_T("Manufacturer"),wxLIST_FORMAT_LEFT, 260);
		listCtrlUSBDevices->InsertColumn(3,_T("Product"),wxLIST_FORMAT_LEFT,260);
	
		btnDownloadAndFlash->Enable(false);
		
		hardware = new wxBitmap((const char **) hardware_xpm);
		m_bitmap1->SetBitmap(*hardware);
	
	
		aboutBtnBmp = new wxBitmap((const char **) IconAbout24_xpm);
		btn_about->SetBitmapLabel(*aboutBtnBmp);
	
		RefreshOnlineVersions();
		RefreshUSBDevices();
		CloseSession();
		SwitchToAdapterSelection();
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
void MainFrame::RefreshUSBDevices()
{
	
	try{
	    usbDevices.Update();
	
    	//clear the listCtrlUSBDevices
		listCtrlUSBDevices->DeleteAllItems();
    	for (int i =0; i < usbDevices.Count();i++){
			int itemIndex;
			itemIndex = listCtrlUSBDevices->InsertItem(listCtrlOnlineVersions->GetItemCount(),_T(""));
			//USB Device Manufacturer
			listCtrlUSBDevices->SetItem(itemIndex,0,usbDevices[i]->Manufacturer);
		
			//Application Product
			listCtrlUSBDevices->SetItem(itemIndex,1,usbDevices[i]->Product);
   		}
   	}catch(std::exception &e){
		wxLogError(_T("%s"), e.what());
   	}
}
void MainFrame::StartSession()
{
	try{
		
		if (listCtrlUSBDevices->GetSelectedItemCount() == 1){
			long firstSelectedItem =-1;
			firstSelectedItem = listCtrlUSBDevices->GetNextItem(firstSelectedItem,
											 wxLIST_NEXT_ALL,
											 wxLIST_STATE_SELECTED);
			lblConnectionStatus->SetLabel(wxString::Format(_T("Connected to %s"),usbDevices[firstSelectedItem]->Product.c_str()));

			panelOnline->Enable(true);
			panelLocal->Enable(true);
			deviceInfo = usbDevices[firstSelectedItem];
		} else{
			wxLogError(_T("One Device has to be selected to connect"));
		}
		//if (deviceID >= 0){
		/*}else{
			lblConnectionStatus->SetLabel(_T("NOT Connected to usbprog-adapter"));
			lblCurrentFirmware->SetLabel(wxEmptyString);
			panelOnline->Enable(false);
			panelLocal->Enable(false);
		}*/
		
		//teUSBDeviceVID->SetValue(wxString::Format(_T("0x%4X"),usbProg.getVendorID(deviceID)));
		//teUSBDevicePID->SetValue(wxString::Format(_T("0x%4X"),usbProg.getProductID(deviceID)));
	}catch(std::exception &e){
		wxLogError(_T("%s"), e.what());
		SwitchToAppLog();
	}
	
}

void MainFrame::CloseSession()
{
	try{
		deviceInfo = NULL;
		lblConnectionStatus->SetLabel(_T("NOT Connected to usbprog-adapter"));

		panelOnline->Enable(false);
		panelLocal->Enable(false);
	
		
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

void MainFrame::SwitchToAdapterSelection()
{
	notebookSource->SetSelection(0);
}

void MainFrame::OnBtnConnectClick( wxCommandEvent& event )
{
	StartSession();
}



void MainFrame::OnBtnSelectFileClick( wxCommandEvent& event )
{
	wxFileDialog fileDialog(this);
	fileDialog.ShowModal();
	textCtrlFileName->SetValue(fileDialog.GetPath());
}

void MainFrame::OnBtnRefreshListClick( wxCommandEvent& event )
{
	RefreshUSBDevices();
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

			UpdateFirmware(tmpFileName);
			
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
			
			UpdateFirmware(fileName);
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

void MainFrame::UpdateFirmware(wxString filename)
{
		
	try{
		if (deviceInfo == NULL){
				throw runtime_error("Not connected to device");
		}
		int vendorId = 0;
		int productId = 0;
		//Falls wir noch nicht im Update-Mode sind in diesen schalten
		if ((deviceInfo->VendorID != 0x1781) || (deviceInfo->ProductID != 0x0c62)|| (deviceInfo->BcdDevice != AVRUPDATE)){
			wxLogDebug(_T("Try to switch usbprog to update mode"));
			wxLogInfo(_T("usbprog found with: %s"),deviceInfo->Product.c_str());
			wxLogInfo(_T("start update mode"));
			avrupdate_start_with_vendor_request(deviceInfo->VendorID,deviceInfo->ProductID);
			wxLogInfo(_T("update mode started"));
			//now there should be a USBDevice AVRUPDATE
			USBDevices devices;
			
			wxLogInfo(_T("waiting some seconds"));
			
			#if _WIN32
			Sleep(7000);
			#else
			sleep(3);
			#endif
			devices.Update();
			bool updateDeviceFound = false;
			
			for (int i = 0 ; i< devices.Count();i++){
				if ((devices[i]->VendorID ==  0x1781) && (devices[i]->ProductID == 0x0c62)&& (devices[i]->BcdDevice == AVRUPDATE)){
					vendorId = devices[i]->VendorID;
					productId = devices[i]->ProductID;
					updateDeviceFound = true;
				}
			}
			
			if (!updateDeviceFound) {
					wxLogError(_T("Update device not found"));
					wxLogError(_T("Could not switch usbprog to update mode. Perhaps the device is not a usbprog adapter"));
					throw runtime_error("Could not switch usbprog to update mode. Perhaps the device is not a usbprog adapter");
			}
		}else{
			wxLogDebug(_T("usbprog is already in update mode"));
			vendorId = deviceInfo->VendorID;
			productId = deviceInfo->ProductID;
		}
		
		if ((vendorId == 0x1781) and (productId == 0x0c62)) {
			usb_dev_handle* usb_handle = avrupdate_open(vendorId,productId);	
			//at the moment we have to copy the filename string to provide
			//a non const char* as filename
			int wxchar_len = filename.Len()+1; 
			int char_len = wxchar_len*sizeof(wxChar)/sizeof(char);
		
			char * buf  = new char[char_len];
			try{
				memcpy( buf, filename.mb_str() , char_len*sizeof(char) ); 
			
				wxLogInfo(_T("Starting to flash %s"),filename.c_str());
				avrupdate_flash_bin(usb_handle,buf);
			}catch(...){
				delete(buf);
				throw;
			}
			delete(buf);
			
			sleep(3);
			wxLogInfo(_T("Completed flashing"),filename.c_str());
			avrupdate_startapp(usb_handle);
			avrupdate_close(usb_handle);
		}	
		CloseSession();
	}catch(std::exception &e){
		wxLogError(_T("%s"), e.what());
	}catch(...){
		wxLogError(_T("Fehler beim Update"));
	}
}
void MainFrame::OnBtnAboutClick( wxCommandEvent& event )
{
	AboutDialog* aboutDlg = new AboutDialog(this);
	aboutDlg->ShowModal();
	delete(aboutDlg);
}
