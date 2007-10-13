//---------------------------------------------------------------------------
//
// Name:        usbprogApp.h
// Author:      Robert Schilling
// Created:     02.08.2007 15:24:17
// Description: 
//
//---------------------------------------------------------------------------

#ifndef __USBPROGFRMApp_h__
#define __USBPROGFRMApp_h__

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#else
	#include <wx/wxprec.h>
#endif

class usbprogFrmApp : public wxApp
{
	public:
		bool OnInit();
		int OnExit();
};

#endif
