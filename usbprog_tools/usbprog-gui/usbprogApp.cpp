//---------------------------------------------------------------------------
//
// Name:        usbprogApp.cpp
// Author:      Robert Schilling
// Created:     02.08.2007 15:24:17
// Description: 
//
//---------------------------------------------------------------------------

#include "usbprogApp.h"
#include "usbprogFrm.h"

IMPLEMENT_APP(usbprogFrmApp)

bool usbprogFrmApp::OnInit()
{
    usbprogFrm* frame = new usbprogFrm(NULL);
    SetTopWindow(frame);
    frame->Show();
    return true;
}
 
int usbprogFrmApp::OnExit()
{
	return 0;
}
