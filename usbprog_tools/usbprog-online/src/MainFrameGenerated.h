///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb  2 2007)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __MainFrameGenerated__
#define __MainFrameGenerated__

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
#include <wx/listctrl.h>

///////////////////////////////////////////////////////////////////////////

#define wxIDBtnFindAdapter 1000
#define wxIDBtnFlash 1001
#define wxIDBtnRefreshOnlineVersions 1002
#define wxIDTextCtrlAppLog 1003

///////////////////////////////////////////////////////////////////////////////
/// Class MainFrameGenerated
///////////////////////////////////////////////////////////////////////////////
class MainFrameGenerated : public wxFrame 
{
	DECLARE_EVENT_TABLE()
	private:
		
		// Private event handlers
		void _wxFB_OnBtnFindAdapterClick( wxCommandEvent& event ){ OnBtnFindAdapterClick( event ); }
		void _wxFB_OnBtnFlashClick( wxCommandEvent& event ){ OnBtnFlashClick( event ); }
		void _wxFB_OnBtnRefreshOnlineVersionsClick( wxCommandEvent& event ){ OnBtnRefreshOnlineVersionsClick( event ); }
		
	
	protected:
		wxStaticText* m_staticText2;
		wxStaticText* m_staticText3;
		wxStaticText* m_staticText4;
		wxStaticText* m_staticText5;
		wxStaticBitmap* m_bitmap1;
		wxStaticLine* m_staticline2;
		wxStaticText* m_staticText62;
		wxStaticText* lblCurrentFirmware;
		wxButton* btnFindAdapter;
		wxStaticLine* m_staticline1;
		wxStaticText* m_staticText6;
		wxListCtrl* listCtrlOnlineVersions;
		wxButton* btnFlash;
		wxButton* btnRefreshOnlineVersions;
		wxStaticText* m_staticText61;
		wxTextCtrl* textCtrlAppLog;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnBtnFindAdapterClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnBtnFlashClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnBtnRefreshOnlineVersionsClick( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		MainFrameGenerated( wxWindow* parent, int id = wxID_ANY, wxString title = wxT("USBProg - Online"), wxPoint pos = wxDefaultPosition, wxSize size = wxSize( 553,710 ), int style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
	
};

#endif //__MainFrameGenerated__
