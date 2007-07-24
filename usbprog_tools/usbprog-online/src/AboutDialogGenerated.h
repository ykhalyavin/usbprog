///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb  2 2007)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __AboutDialogGenerated__
#define __AboutDialogGenerated__

// Define WX_GCH in order to support precompiled headers with GCC compiler.
// You have to create the header "wx_pch.h" and include all files needed
// for compile your gui inside it.
// Then, compile it and place the file "wx_pch.h.gch" into the same
// directory that "wx_pch.h".
#ifdef WX_GCH
#include <wx_pch.h>
#else
#include <wx/wx.h>
#endif

#include <wx/statbmp.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/panel.h>

///////////////////////////////////////////////////////////////////////////

#define wxIDBtnLinkEmbeddedProjects 1000
#define wxIDBtnLnkSoftsprings 1001

///////////////////////////////////////////////////////////////////////////////
/// Class AboutDialogGenerated
///////////////////////////////////////////////////////////////////////////////
class AboutDialogGenerated : public wxDialog 
{
	DECLARE_EVENT_TABLE()
	private:
		
		// Private event handlers
		void _wxFB_OnBtnLnkEmbeddedProjectsClick( wxCommandEvent& event ){ OnBtnLnkEmbeddedProjectsClick( event ); }
		void _wxFB_OnBtnLnkSoftspringsClick( wxCommandEvent& event ){ OnBtnLnkSoftspringsClick( event ); }
		
	
	protected:
		wxPanel* m_panel1;
		wxStaticText* m_staticText1;
		wxStaticBitmap* m_bitmap1;
		wxStaticText* m_staticText3;
		wxStaticLine* m_staticline1;
		wxStaticText* m_staticText2;
		wxButton* btnLinkEmbeddedProjects;
		wxStaticLine* m_staticline11;
		wxStaticText* m_staticText4;
		wxStaticBitmap* m_bitmap2;
		wxStaticText* m_staticText5;
		wxButton* btnLnkSoftsprings;
		wxButton* btnClose;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnBtnLnkEmbeddedProjectsClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnBtnLnkSoftspringsClick( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		AboutDialogGenerated( wxWindow* parent, int id = wxID_ANY, wxString title = wxEmptyString, wxPoint pos = wxDefaultPosition, wxSize size = wxSize( 400,500 ), int style = wxDEFAULT_DIALOG_STYLE );
	
};

#endif //__AboutDialogGenerated__
