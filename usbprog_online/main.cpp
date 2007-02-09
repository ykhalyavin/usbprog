/*
 *  * Version: $Id: svnrev.c 3116 2005-03-18 16:26:29Z thiadmer $
 *   */

#include "wx/wxprec.h"
#include "wx/wx.h" 

#include "window.h"



class vScope: public wxApp
{
    virtual bool OnInit();
		};

		IMPLEMENT_APP(vScope)

bool vScope::OnInit()
{
wxString title;
#ifndef SVN_REV
//title.Printf("vScope Studio 2006");
#else
//title.Printf("vScope Studio Rev. %s", SVN_REV);
#endif

MyFrame *frame = new MyFrame(0, 10, title, wxPoint(50,50), wxSize(100,100),0);
frame->Show(TRUE);
SetTopWindow(frame);
return TRUE;
} 

