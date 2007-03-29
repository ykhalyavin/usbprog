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
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>

///////////////////////////////////////////////////////////////////////////

#define wxIDNotebookSource 1000
#define wxIDBtnFindAdapter 1001
#define wxIDListCtrlOnlineVersions 1002
#define wxIDBtnDownloadAndFlash 1003
#define wxIDBtnRefreshOnlineVersions 1004
#define wxIDBtnSelectFile 1005
#define wxIDBtnFlashLocalFile 1006
#define wxIDTextCtrlAppLog 1007
#define wxIDBtnClearLog 1008

///////////////////////////////////////////////////////////////////////////////
/// Class MainFrameGenerated
///////////////////////////////////////////////////////////////////////////////
class MainFrameGenerated : public wxFrame 
{
	DECLARE_EVENT_TABLE()
	private:
		
		// Private event handlers
		void _wxFB_OnBtnFindAdapterClick( wxCommandEvent& event ){ OnBtnFindAdapterClick( event ); }
		void _wxFB_OnListCtrlOnlineVersionsItemDeselected( wxListEvent& event ){ OnListCtrlOnlineVersionsItemDeselected( event ); }
		void _wxFB_OnListCtrlOnlineVersionsItemSelected( wxListEvent& event ){ OnListCtrlOnlineVersionsItemSelected( event ); }
		void _wxFB_OnBtnFlashClick( wxCommandEvent& event ){ OnBtnFlashClick( event ); }
		void _wxFB_OnBtnRefreshOnlineVersionsClick( wxCommandEvent& event ){ OnBtnRefreshOnlineVersionsClick( event ); }
		void _wxFB_OnBtnSelectFileClick( wxCommandEvent& event ){ OnBtnSelectFileClick( event ); }
		void _wxFB_OnBtnFlashLocalFile( wxCommandEvent& event ){ OnBtnFlashLocalFile( event ); }
		void _wxFB_OnBtnClearLog( wxCommandEvent& event ){ OnBtnClearLog( event ); }
		
	
	protected:
		wxStaticText* m_staticText2;
		wxStaticText* m_staticText3;
		wxStaticText* m_staticText4;
		wxStaticText* m_staticText5;
		wxStaticBitmap* m_bitmap1;
		wxNotebook* notebookSource;
		wxPanel* panelStatus;
		wxStaticText* m_staticText621;
		wxStaticText* lblConnectionStatus;
		wxStaticText* m_staticText62;
		wxStaticText* lblCurrentFirmware;
		wxButton* btnFindAdapter;
		wxPanel* panelOnline;
		wxStaticText* m_staticText6;
		wxListCtrl* listCtrlOnlineVersions;
		wxButton* btnDownloadAndFlash;
		wxButton* btnRefreshOnlineVersions;
		wxPanel* panelLocal;
		wxStaticText* m_staticText9;
		wxTextCtrl* textCtrlFileName;
		wxButton* btnSelectFile;
		wxButton* btnFlashLocalFile;
		wxPanel* panelLog;
		wxStaticText* m_staticText61;
		wxTextCtrl* textCtrlAppLog;
		wxButton* btnClearLog;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnBtnFindAdapterClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnListCtrlOnlineVersionsItemDeselected( wxListEvent& event ){ event.Skip(); }
		virtual void OnListCtrlOnlineVersionsItemSelected( wxListEvent& event ){ event.Skip(); }
		virtual void OnBtnFlashClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnBtnRefreshOnlineVersionsClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnBtnSelectFileClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnBtnFlashLocalFile( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnBtnClearLog( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		MainFrameGenerated( wxWindow* parent, int id = wxID_ANY, wxString title = wxT("USBProg - Online"), wxPoint pos = wxDefaultPosition, wxSize size = wxSize( 553,553 ), int style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
	
};

#endif //__MainFrameGenerated__
