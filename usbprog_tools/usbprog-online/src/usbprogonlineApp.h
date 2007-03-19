/***************************************************************
 * Name:      usbprogonlineApp.h
 * Purpose:   Defines Application Class
 * Author:    Alois Flammensboeck (flammenboeck@softsprings.org)
 * Created:   2007-03-09
 * Copyright: Alois Flammensboeck (http://www.softsprings.org)
 * License:   
 **************************************************************/
 
#ifndef USBPROGONLINEAPP_H
#define USBPROGONLINEAPP_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

class usbprogonlineApp : public wxApp
{
	public:
		virtual bool OnInit();
};

#endif // usbprogonlineAPP_H
