///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb  2 2007)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif //WX_PRECOMP

#include "GUIFrame.h"

///////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE( GUIFrame, wxFrame )
	EVT_BUTTON( wxIDBtnFindAdapter, GUIFrame::_wxFB_OnBtnFindAdapterClick )
	EVT_BUTTON( wxIDBtnDownload, GUIFrame::_wxFB_OnBtnDownloadClick )
	EVT_BUTTON( wxIDBtnFlash, GUIFrame::_wxFB_OnBtnFlashClick )
END_EVENT_TABLE()

GUIFrame::GUIFrame( wxWindow* parent, int id, wxString title, wxPoint pos, wxSize size, int style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	bSizer1->Add( 20, 5, 0, wxADJUST_MINSIZE, 0 );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	bSizer2->Add( 5, 20, 0, wxADJUST_MINSIZE, 0 );
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( this, -1, wxT("usbprog") ), wxHORIZONTAL );
	
	sbSizer1->Add( 5, 0, 0, wxADJUST_MINSIZE, 0 );
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );
	
	bSizer4->Add( 20, 5, 0, wxADJUST_MINSIZE, 0 );
	
	label_1 = new wxStaticText( this, wxID_ANY, wxT("USB Device:"), wxDefaultPosition, wxDefaultSize, 0 );
	label_1->SetFont( wxFont( 10, 74, 90, 92, false, wxT("sans") ) );
	
	bSizer4->Add( label_1, 0, 0, 0 );
	
	bSizer4->Add( 20, 3, 0, wxADJUST_MINSIZE, 0 );
	
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxHORIZONTAL );
	
	label_2 = new wxStaticText( this, wxID_ANY, wxT("Vendor ID:"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( label_2, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	bSizer5->Add( 5, 20, 0, wxADJUST_MINSIZE, 0 );
	
	teUSBDeviceVID = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( teUSBDeviceVID, 0, 0, 0 );
	
	bSizer5->Add( 20, 1, 0, wxADJUST_MINSIZE, 0 );
	
	label_2_copy = new wxStaticText( this, wxID_ANY, wxT("Product ID:"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( label_2_copy, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	bSizer5->Add( 5, 20, 0, wxADJUST_MINSIZE, 0 );
	
	teUSBDevicePID = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( teUSBDevicePID, 0, 0, 0 );
	
	bSizer4->Add( bSizer5, 0, wxEXPAND, 0 );
	
	bSizer4->Add( 20, 5, 0, wxADJUST_MINSIZE, 0 );
	
	label_1_copy = new wxStaticText( this, wxID_ANY, wxT("USB avrupdate:"), wxDefaultPosition, wxDefaultSize, 0 );
	label_1_copy->SetFont( wxFont( 10, 74, 90, 92, false, wxT("sans") ) );
	
	bSizer4->Add( label_1_copy, 0, 0, 0 );
	
	bSizer4->Add( 20, 3, 0, wxADJUST_MINSIZE, 0 );
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxHORIZONTAL );
	
	label_2_copy_1 = new wxStaticText( this, wxID_ANY, wxT("Vendor ID:"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer6->Add( label_2_copy_1, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	bSizer6->Add( 5, 20, 0, wxADJUST_MINSIZE, 0 );
	
	teUSBUpdateVID = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer6->Add( teUSBUpdateVID, 0, 0, 0 );
	
	bSizer6->Add( 20, 1, 0, wxADJUST_MINSIZE, 0 );
	
	label_2_copy_copy = new wxStaticText( this, wxID_ANY, wxT("Product ID:"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer6->Add( label_2_copy_copy, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	bSizer6->Add( 5, 20, 0, wxADJUST_MINSIZE, 0 );
	
	teUSBUpdatePID = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer6->Add( teUSBUpdatePID, 0, 0, 0 );
	
	bSizer4->Add( bSizer6, 0, wxEXPAND, 0 );
	
	static_line_1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer4->Add( static_line_1, 1, wxEXPAND, 0 );
	
	label_1_copy_copy = new wxStaticText( this, wxID_ANY, wxT("Version File:"), wxDefaultPosition, wxSize( 200, 20 ), 0 );
	label_1_copy_copy->SetFont( wxFont( 10, 74, 90, 92, false, wxT("sans") ) );
	
	bSizer4->Add( label_1_copy_copy, 0, wxADJUST_MINSIZE, 0 );
	
	teVersionsFileURL = new wxTextCtrl( this, wxID_ANY, wxT("http://www.ixbat.de/usbprog/versions.conf"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer4->Add( teVersionsFileURL, 0, wxEXPAND, 0 );
	
	static_line_1_copy = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxSize( 336, 20 ), wxLI_HORIZONTAL );
	bSizer4->Add( static_line_1_copy, 1, wxEXPAND, 0 );
	
	label_1_copy_copy_copy = new wxStaticText( this, wxID_ANY, wxT("Current Firmware:"), wxDefaultPosition, wxSize( 200, 20 ), 0 );
	label_1_copy_copy_copy->SetFont( wxFont( 10, 74, 90, 92, false, wxT("sans") ) );
	
	bSizer4->Add( label_1_copy_copy_copy, 0, wxADJUST_MINSIZE, 0 );
	
	lblCurrentFirmware = new wxStaticText( this, wxID_ANY, wxT("Blink Demo"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE|wxST_NO_AUTORESIZE );
	bSizer4->Add( lblCurrentFirmware, 0, wxEXPAND, 0 );
	
	bSizer4->Add( 0, 5, 0, wxADJUST_MINSIZE, 0 );
	
	btnFindAdapter = new wxButton( this, wxIDBtnFindAdapter, wxT("Find usbprog Adapter"), wxDefaultPosition, wxSize( 146, 30 ), 0 );
	bSizer4->Add( btnFindAdapter, 0, wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, 0 );
	
	static_line_1_copy_copy = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer4->Add( static_line_1_copy_copy, 1, wxEXPAND, 0 );
	
	label_4 = new wxStaticText( this, wxID_ANY, wxT("http://www.embedded-projects.net"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer4->Add( label_4, 0, wxADJUST_MINSIZE, 0 );
	
	bSizer3->Add( bSizer4, 1, wxEXPAND, 0 );
	
	sbSizer1->Add( bSizer3, 1, wxEXPAND, 0 );
	
	sbSizer1->Add( 5, 0, 0, wxADJUST_MINSIZE, 0 );
	
	bSizer2->Add( sbSizer1, 1, wxEXPAND, 0 );
	
	bSizer2->Add( 10, 20, 0, wxADJUST_MINSIZE, 0 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( this, -1, wxT("Online Versions") ), wxHORIZONTAL );
	
	sbSizer2->Add( 5, 20, 0, wxADJUST_MINSIZE, 0 );
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxVERTICAL );
	
	bSizer7->Add( 20, 5, 0, wxADJUST_MINSIZE, 0 );
	
	label_1_copy_1 = new wxStaticText( this, wxID_ANY, wxT("Online:"), wxDefaultPosition, wxDefaultSize, 0 );
	label_1_copy_1->SetFont( wxFont( 10, 74, 90, 92, false, wxT("sans") ) );
	
	bSizer7->Add( label_1_copy_1, 0, wxADJUST_MINSIZE, 0 );
	
	bSizer7->Add( 20, 3, 0, wxADJUST_MINSIZE, 0 );
	
	listCtrlOnlineVersions = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxSUNKEN_BORDER );
	bSizer7->Add( listCtrlOnlineVersions, 5, wxEXPAND, 0 );
	
	label_1_copy_1_copy = new wxStaticText( this, wxID_ANY, wxT("Log:"), wxDefaultPosition, wxSize( 32, 20 ), 0 );
	label_1_copy_1_copy->SetFont( wxFont( 10, 74, 90, 92, false, wxT("sans") ) );
	
	bSizer7->Add( label_1_copy_1_copy, 0, wxADJUST_MINSIZE, 0 );
	
	text_ctrl_3 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	bSizer7->Add( text_ctrl_3, 0, wxEXPAND, 0 );
	
	label_1_copy_1_copy_copy = new wxStaticText( this, wxID_ANY, wxT("Process:"), wxDefaultPosition, wxSize( 63, 20 ), 0 );
	label_1_copy_1_copy_copy->SetFont( wxFont( 10, 74, 90, 92, false, wxT("sans") ) );
	
	bSizer7->Add( label_1_copy_1_copy_copy, 0, wxADJUST_MINSIZE, 0 );
	
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxHORIZONTAL );
	
	btnDownload = new wxButton( this, wxIDBtnDownload, wxT("Query available online versions"), wxDefaultPosition, wxSize( 210,30 ), 0 );
	bSizer8->Add( btnDownload, 0, 0, 0 );
	
	bSizer8->Add( 5, 20, 0, wxEXPAND|wxADJUST_MINSIZE, 0 );
	
	bSizer8->Add( 5, 20, 0, wxEXPAND|wxADJUST_MINSIZE, 0 );
	
	btnFlash = new wxButton( this, wxIDBtnFlash, wxT("Flash"), wxDefaultPosition, wxSize( 80, 30 ), 0 );
	bSizer8->Add( btnFlash, 0, 0, 0 );
	
	bSizer7->Add( bSizer8, 1, 0, 0 );
	
	sbSizer2->Add( bSizer7, 1, wxEXPAND, 0 );
	
	sbSizer2->Add( 5, 0, 0, wxADJUST_MINSIZE, 0 );
	
	bSizer2->Add( sbSizer2, 1, wxEXPAND, 0 );
	
	bSizer2->Add( 5, 20, 0, wxADJUST_MINSIZE, 0 );
	
	bSizer1->Add( bSizer2, 1, wxEXPAND, 0 );
	
	bSizer1->Add( 20, 5, 0, wxADJUST_MINSIZE, 0 );
	
	this->SetSizer( bSizer1 );
	this->Layout();
	bSizer1->Fit( this );
}
