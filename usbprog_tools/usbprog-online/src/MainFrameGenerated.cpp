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

#include "MainFrameGenerated.h"

///////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE( MainFrameGenerated, wxFrame )
	EVT_BUTTON( wxIDBtnFindAdapter, MainFrameGenerated::_wxFB_OnBtnFindAdapterClick )
	EVT_BUTTON( wxIDBtnFlash, MainFrameGenerated::_wxFB_OnBtnFlashClick )
	EVT_BUTTON( wxIDBtnRefreshOnlineVersions, MainFrameGenerated::_wxFB_OnBtnRefreshOnlineVersionsClick )
END_EVENT_TABLE()

MainFrameGenerated::MainFrameGenerated( wxWindow* parent, int id, wxString title, wxPoint pos, wxSize size, int style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 1, 0, 0 );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText2 = new wxStaticText( this, wxID_ANY, wxT("USBProg - Online"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->SetFont( wxFont( 12, 74, 90, 92, false, wxT("Sans") ) );
	
	bSizer2->Add( m_staticText2, 0, wxALL, 5 );
	
	m_staticText3 = new wxStaticText( this, wxID_ANY, wxT("A tool to download and flash new \nfirmware to your usbprog-adapter"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->SetFont( wxFont( 10, 74, 90, 92, false, wxT("Sans") ) );
	
	bSizer2->Add( m_staticText3, 1, wxALL, 5 );
	
	m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("For more Information please visit:"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_staticText4, 0, wxALL, 5 );
	
	m_staticText5 = new wxStaticText( this, wxID_ANY, wxT("http://www.embedded-projects.net"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_staticText5, 0, wxALL, 5 );
	
	bSizer1->Add( bSizer2, 1, wxEXPAND, 5 );
	
	m_bitmap1 = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_bitmap1, 0, wxALL, 5 );
	
	fgSizer1->Add( bSizer1, 1, wxEXPAND, 5 );
	
	m_staticline2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgSizer1->Add( m_staticline2, 0, wxALL|wxEXPAND, 5 );
	
	fgSizer1->Add( 0, 0, 1, wxEXPAND, 0 );
	
	m_staticText62 = new wxStaticText( this, wxID_ANY, wxT("Current firmware:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText62->SetFont( wxFont( 10, 74, 90, 92, false, wxT("Sans") ) );
	
	fgSizer1->Add( m_staticText62, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxHORIZONTAL );
	
	lblCurrentFirmware = new wxStaticText( this, wxID_ANY, wxT("No usbprog-adapter found"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	bSizer5->Add( lblCurrentFirmware, 10, wxALIGN_CENTER|wxALIGN_LEFT|wxALL, 5 );
	
	btnFindAdapter = new wxButton( this, wxIDBtnFindAdapter, wxT("ReConnect usbprog-adapter"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( btnFindAdapter, 0, wxALL, 5 );
	
	fgSizer1->Add( bSizer5, 1, wxEXPAND, 5 );
	
	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgSizer1->Add( m_staticline1, 0, wxALL|wxEXPAND, 5 );
	
	fgSizer1->Add( 0, 0, 1, wxEXPAND, 0 );
	
	m_staticText6 = new wxStaticText( this, wxID_ANY, wxT("Please select a firmware you want to flash:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->SetFont( wxFont( 10, 74, 90, 92, false, wxT("Sans") ) );
	
	fgSizer1->Add( m_staticText6, 0, wxALL, 5 );
	
	listCtrlOnlineVersions = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxSize( -1,200 ), wxLC_REPORT );
	listCtrlOnlineVersions->SetMinSize( wxSize( -1,200 ) );
	
	fgSizer1->Add( listCtrlOnlineVersions, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );
	
	btnFlash = new wxButton( this, wxIDBtnFlash, wxT("Download and flash selected firmware"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer3->Add( btnFlash, 10, wxALL|wxLEFT, 5 );
	
	btnRefreshOnlineVersions = new wxButton( this, wxIDBtnRefreshOnlineVersions, wxT("Refresh firmware list"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer3->Add( btnRefreshOnlineVersions, 0, wxALL, 5 );
	
	fgSizer1->Add( bSizer3, 1, wxEXPAND, 5 );
	
	fgSizer1->Add( 0, 10, 1, wxEXPAND, 0 );
	
	m_staticText61 = new wxStaticText( this, wxID_ANY, wxT("Application log:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText61->SetFont( wxFont( 10, 74, 90, 92, false, wxT("Sans") ) );
	
	fgSizer1->Add( m_staticText61, 0, wxALL, 5 );
	
	textCtrlAppLog = new wxTextCtrl( this, wxIDTextCtrlAppLog, wxEmptyString, wxDefaultPosition, wxSize( -1,150 ), wxHSCROLL|wxTE_MULTILINE );
	fgSizer1->Add( textCtrlAppLog, 0, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( fgSizer1 );
	this->Layout();
}
