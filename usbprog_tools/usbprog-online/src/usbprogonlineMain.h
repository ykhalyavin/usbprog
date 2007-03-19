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

#include "GUIFrame.h"
#include "usbprog.h"
#include "OnlineVersions.h"

class usbprogonlineFrame: public GUIFrame
{
	usbprog usbProg;
    OnlineVersions onlineVersions;
	public:
		usbprogonlineFrame(wxFrame *frame);
		~usbprogonlineFrame();
	private:
		virtual void OnClose(wxCloseEvent& event);
		virtual void OnQuit(wxCommandEvent& event);
		virtual void OnAbout(wxCommandEvent& event);
    protected:
		virtual void OnBtnFindAdapterClick( wxCommandEvent& event );
		virtual void OnBtnDownloadClick( wxCommandEvent& event );
		virtual void OnBtnCancelClick( wxCommandEvent& event );
		virtual void OnBtnQuitClick( wxCommandEvent& event );
		virtual void OnBtnFlashClick( wxCommandEvent& event );
};

#endif // usbprogonlineMAIN_H
