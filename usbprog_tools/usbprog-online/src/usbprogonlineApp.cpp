/***************************************************************
 * Name:      usbprogonlineApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Alois Flammensboeck (flammenboeck@softsprings.org)
 * Created:   2007-03-09
 * Copyright: Alois Flammensboeck (http://www.softsprings.org)
 * License:
 **************************************************************/

#ifdef WX_PRECOMP //
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "usbprogonlineApp.h"
#include "usbprogonlineMain.h"

IMPLEMENT_APP(usbprogonlineApp);

bool usbprogonlineApp::OnInit()
{
	usbprogonlineFrame* frame = new usbprogonlineFrame(0L);

	frame->Show();

	return true;
}
