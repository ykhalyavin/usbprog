///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb  2 2007)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __GUIFrame__
#define __GUIFrame__

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

#include <wx/statline.h>
#include <wx/button.h>
#include <wx/listctrl.h>

///////////////////////////////////////////////////////////////////////////

#define wxIDBtnFindAdapter 1000
#define wxIDBtnDownload 1001
#define wxIDBtnFlash 1002

///////////////////////////////////////////////////////////////////////////////
/// Class GUIFrame
///////////////////////////////////////////////////////////////////////////////
class GUIFrame : public wxFrame 
{
	DECLARE_EVENT_TABLE()
	private:
		
		// Private event handlers
		void _wxFB_OnBtnFindAdapterClick( wxCommandEvent& event ){ OnBtnFindAdapterClick( event ); }
		void _wxFB_OnBtnDownloadClick( wxCommandEvent& event ){ OnBtnDownloadClick( event ); }
		void _wxFB_OnBtnFlashClick( wxCommandEvent& event ){ OnBtnFlashClick( event ); }
		
	
	protected:
		wxStaticText* label_1;
		wxStaticText* label_2;
		wxTextCtrl* teUSBDeviceVID;
		wxStaticText* label_2_copy;
		wxTextCtrl* teUSBDevicePID;
		wxStaticText* label_1_copy;
		wxStaticText* label_2_copy_1;
		wxTextCtrl* teUSBUpdateVID;
		wxStaticText* label_2_copy_copy;
		wxTextCtrl* teUSBUpdatePID;
		wxStaticLine* static_line_1;
		wxStaticText* label_1_copy_copy;
		wxTextCtrl* teVersionsFileURL;
		wxStaticLine* static_line_1_copy;
		wxStaticText* label_1_copy_copy_copy;
		wxStaticText* lblCurrentFirmware;
		wxButton* btnFindAdapter;
		wxStaticLine* static_line_1_copy_copy;
		wxStaticText* label_4;
		wxStaticText* label_1_copy_1;
		wxListCtrl* listCtrlOnlineVersions;
		wxStaticText* label_1_copy_1_copy;
		wxTextCtrl* text_ctrl_3;
		wxStaticText* label_1_copy_1_copy_copy;
		wxButton* btnDownload;
		wxButton* btnFlash;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnBtnFindAdapterClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnBtnDownloadClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnBtnFlashClick( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		GUIFrame( wxWindow* parent, int id = wxID_ANY, wxString title = wxT("usbprog - Online Update"), wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize, int style = wxDEFAULT_FRAME_STYLE );
	
};

#endif //__GUIFrame__
