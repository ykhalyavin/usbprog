//---------------------------------------------------------------------------
//
// Name:        usbprogFrm.h
// Author:      Robert Schilling
// Created:     02.08.2007 15:24:17
// Description: usbprogFrm class declaration
//
//---------------------------------------------------------------------------

#ifndef __USBPROGFRM_h__
#define __USBPROGFRM_h__

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif

//Do not add custom headers between 
//Header Include Start and Header Include End.
//wxDev-C++ designer will remove them. Add custom headers after the block.
////Header Include Start
#include <wx/filedlg.h>
#include <wx/statbmp.h>
#include <wx/gauge.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/statbox.h>
////Header Include End

////Dialog Style Start
#undef usbprogFrm_STYLE
#define usbprogFrm_STYLE wxCAPTION | wxSYSTEM_MENU | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX
////Dialog Style End

class usbprogFrm : public wxFrame
{
	private:
		DECLARE_EVENT_TABLE();
		
	public:
		usbprogFrm(wxWindow *parent, wxWindowID id = 1, const wxString &title = wxT("Usbprog-Update"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = usbprogFrm_STYLE);
		virtual ~usbprogFrm();
		void usbprogFrmActivate(wxActivateEvent& event);
		void WxButton3Click(wxCommandEvent& event);
		void WxButton1Click(wxCommandEvent& event);
		void WxRadioBox1Click(wxCommandEvent& event);
		void WxEdit1Updated(wxCommandEvent& event);
		void WxButton2Click(wxCommandEvent& event);
		void WxRadioButton1Click(wxCommandEvent& event);
		void WxRadioButton2Click(wxCommandEvent& event);
		void WxButton5Click(wxCommandEvent& event);
		void WxButton4Click(wxCommandEvent& event);
		void WxEdit2Updated(wxCommandEvent& event);
		
		// No description
		void printWxEdit2(char * text);
		void WxButton6Click(wxCommandEvent& event);
		void getUsbDevices(void);

		
	private:
		//Do not add custom control declarations between
		//GUI Control Declaration Start and GUI Control Declaration End.
		//wxDev-C++ will remove them. Add custom code after the block.
		////GUI Control Declaration Start
		wxFileDialog *WxOpenFileDialog1;
		wxButton *WxButton6;
		wxStaticText *WxStaticText5;
		wxStaticText *WxStaticText4;
		wxStaticBitmap *WxStaticBitmap1;
		wxButton *WxButton5;
		wxButton *WxButton4;
		wxStaticText *WxStaticText3;
		wxButton *WxButton2;
		wxGauge *WxGauge1;
		wxButton *WxButton3;
		wxButton *WxButton1;
		wxStaticText *WxStaticText2;
		wxTextCtrl *WxEdit2;
		wxTextCtrl *WxEdit1;
		wxRadioButton *WxRadioButton2;
		wxRadioButton *WxRadioButton1;
		wxStaticText *WxStaticText1;
		wxComboBox *WxComboBox2;
		wxComboBox *WxComboBox1;
		wxStaticBox *WxStaticBox1;
		////GUI Control Declaration End
		
	private:
		//Note: if you receive any error with these enum IDs, then you need to
		//change your old form code that are based on the #define control IDs.
		//#defines may replace a numeric value for the enum names.
		//Try copy and pasting the below block in your old form header files.
		enum
		{
			////GUI Enum Control ID Start
			ID_WXBUTTON6 = 1055,
			ID_WXSTATICTEXT5 = 1054,
			ID_WXSTATICTEXT4 = 1053,
			ID_WXSTATICBITMAP1 = 1052,
			ID_WXBUTTON5 = 1051,
			ID_WXBUTTON4 = 1050,
			ID_WXSTATICTEXT3 = 1047,
			ID_WXBUTTON2 = 1046,
			ID_WXGAUGE1 = 1042,
			ID_WXBUTTON3 = 1041,
			ID_WXBUTTON1 = 1039,
			ID_WXSTATICTEXT2 = 1038,
			ID_WXEDIT2 = 1037,
			ID_WXEDIT1 = 1032,
			ID_WXRADIOBUTTON2 = 1031,
			ID_WXRADIOBUTTON1 = 1030,
			ID_WXSTATICTEXT1 = 1028,
			ID_WXCOMBOBOX2 = 1027,
			ID_WXCOMBOBOX1 = 1026,
			ID_WXSTATICBOX1 = 1025,
			////GUI Enum Control ID End
			ID_DUMMY_VALUE_ //don't remove this value unless you have other enum values
		};
		
	private:
		void OnClose(wxCloseEvent& event);
		void CreateGUIControls();
};



#endif
