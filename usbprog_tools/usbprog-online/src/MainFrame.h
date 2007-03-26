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
#include "usbprog.h"
#include "OnlineVersions.h"

class MainFrame: public MainFrameGenerated
{
	wxBitmap* hardware;
	usbprog usbProg;
    OnlineVersions onlineVersions;
	public:
		MainFrame(wxFrame *frame);
		~MainFrame();
	private:
		virtual void OnClose(wxCloseEvent& event);
		virtual void OnQuit(wxCommandEvent& event);
		void FindAdapter();
		void RefreshOnlineVersions();
    protected:
		
		virtual void OnBtnFindAdapterClick( wxCommandEvent& event );
		virtual void OnBtnFlashClick( wxCommandEvent& event );
		virtual void OnBtnRefreshOnlineVersionsClick( wxCommandEvent& event );	

};

#endif // usbprogonlineMAIN_H
