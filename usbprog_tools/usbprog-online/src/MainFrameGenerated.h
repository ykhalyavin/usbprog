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

#include <wx/bmpbuttn.h>
#include <wx/statbmp.h>
#include <wx/panel.h>
#include <wx/listctrl.h>
#include <wx/button.h>
#include <wx/notebook.h>

///////////////////////////////////////////////////////////////////////////

#define wxIDBtnAbout 1000
#define wxIDNotebookSource 1001
#define wxIDListCtrlUSBDevices 1002
#define wxIDBtnRefreshList 1003
#define wxIDBtnConnect 1004
#define wxIDListCtrlOnlineVersions 1005
#define wxIDBtnDownloadAndFlash 1006
#define wxIDBtnRefreshOnlineVersions 1007
#define wxIDBtnSelectFile 1008
#define wxIDBtnFlashLocalFile 1009
#define wxIDTextCtrlAppLog 1010
#define wxIDBtnClearLog 1011

///////////////////////////////////////////////////////////////////////////////
/// Class MainFrameGenerated
///////////////////////////////////////////////////////////////////////////////
class MainFrameGenerated : public wxFrame 
{
	DECLARE_EVENT_TABLE()
	private:
		
		// Private event handlers
		void _wxFB_OnBtnAboutClick( wxCommandEvent& event ){ OnBtnAboutClick( event ); }
		void _wxFB_OnBtnRefreshListClick( wxCommandEvent& event ){ OnBtnRefreshListClick( event ); }
		void _wxFB_OnBtnConnectClick( wxCommandEvent& event ){ OnBtnConnectClick( event ); }
		void _wxFB_OnListCtrlOnlineVersionsItemDeselected( wxListEvent& event ){ OnListCtrlOnlineVersionsItemDeselected( event ); }
		void _wxFB_OnListCtrlOnlineVersionsItemSelected( wxListEvent& event ){ OnListCtrlOnlineVersionsItemSelected( event ); }
		void _wxFB_OnBtnFlashClick( wxCommandEvent& event ){ OnBtnFlashClick( event ); }
		void _wxFB_OnBtnRefreshOnlineVersionsClick( wxCommandEvent& event ){ OnBtnRefreshOnlineVersionsClick( event ); }
		void _wxFB_OnBtnSelectFileClick( wxCommandEvent& event ){ OnBtnSelectFileClick( event ); }
		void _wxFB_OnBtnFlashLocalFile( wxCommandEvent& event ){ OnBtnFlashLocalFile( event ); }
		void _wxFB_OnBtnClearLog( wxCommandEvent& event ){ OnBtnClearLog( event ); }
		
	
	protected:
		wxStaticText* m_staticText2;
		wxBitmapButton* btn_about;
		wxStaticText* m_staticText3;
		wxStaticBitmap* m_bitmap1;
		wxNotebook* notebookSource;
		wxPanel* panelStatus;
		wxStaticText* m_staticText621;
		wxPanel* m_panel5;
		wxStaticText* lblConnectionStatus;
		wxStaticText* m_staticText6211;
		wxListCtrl* listCtrlUSBDevices;
		wxButton* btnRefreshList;
		wxButton* btnConnect;
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
		virtual void OnBtnAboutClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnBtnRefreshListClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnBtnConnectClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnListCtrlOnlineVersionsItemDeselected( wxListEvent& event ){ event.Skip(); }
		virtual void OnListCtrlOnlineVersionsItemSelected( wxListEvent& event ){ event.Skip(); }
		virtual void OnBtnFlashClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnBtnRefreshOnlineVersionsClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnBtnSelectFileClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnBtnFlashLocalFile( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnBtnClearLog( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		MainFrameGenerated( wxWindow* parent, int id = wxID_ANY, wxString title = wxT("usbprog-online"), wxPoint pos = wxDefaultPosition, wxSize size = wxSize( 553,553 ), int style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
	
};

#endif //__MainFrameGenerated__
