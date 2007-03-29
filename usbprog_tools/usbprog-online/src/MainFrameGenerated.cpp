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
	EVT_LIST_ITEM_DESELECTED( wxIDListCtrlOnlineVersions, MainFrameGenerated::_wxFB_OnListCtrlOnlineVersionsItemDeselected )
	EVT_LIST_ITEM_SELECTED( wxIDListCtrlOnlineVersions, MainFrameGenerated::_wxFB_OnListCtrlOnlineVersionsItemSelected )
	EVT_BUTTON( wxIDBtnDownloadAndFlash, MainFrameGenerated::_wxFB_OnBtnFlashClick )
	EVT_BUTTON( wxIDBtnRefreshOnlineVersions, MainFrameGenerated::_wxFB_OnBtnRefreshOnlineVersionsClick )
	EVT_BUTTON( wxIDBtnSelectFile, MainFrameGenerated::_wxFB_OnBtnSelectFileClick )
	EVT_BUTTON( wxIDBtnFlashLocalFile, MainFrameGenerated::_wxFB_OnBtnFlashLocalFile )
	EVT_BUTTON( wxIDBtnClearLog, MainFrameGenerated::_wxFB_OnBtnClearLog )
END_EVENT_TABLE()

MainFrameGenerated::MainFrameGenerated( wxWindow* parent, int id, wxString title, wxPoint pos, wxSize size, int style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );
	
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
	
	bSizer11->Add( bSizer1, 0, wxEXPAND, 5 );
	
	notebookSource = new wxNotebook( this, wxIDNotebookSource, wxDefaultPosition, wxSize( -1,-1 ), wxNB_TOP );
	panelStatus = new wxPanel( notebookSource, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText621 = new wxStaticText( panelStatus, wxID_ANY, wxT("Connection status:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText621->SetFont( wxFont( 10, 74, 90, 92, false, wxT("Sans") ) );
	
	bSizer9->Add( m_staticText621, 0, wxALL, 5 );
	
	lblConnectionStatus = new wxStaticText( panelStatus, wxID_ANY, wxT("Not connected to usbprog-adapter"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	bSizer9->Add( lblConnectionStatus, 0, wxALL|wxEXPAND, 5 );
	
	bSizer9->Add( 0, 10, 0, wxEXPAND, 0 );
	
	m_staticText62 = new wxStaticText( panelStatus, wxID_ANY, wxT("Current firmware:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText62->SetFont( wxFont( 10, 74, 90, 92, false, wxT("Sans") ) );
	
	bSizer9->Add( m_staticText62, 0, wxALL, 5 );
	
	lblCurrentFirmware = new wxStaticText( panelStatus, wxID_ANY, wxT("No usbprog-adapter found"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	bSizer9->Add( lblCurrentFirmware, 0, wxALL|wxEXPAND, 5 );
	
	bSizer9->Add( 0, 10, 1, wxEXPAND, 0 );
	
	btnFindAdapter = new wxButton( panelStatus, wxIDBtnFindAdapter, wxT("ReConnect usbprog-adapter"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer9->Add( btnFindAdapter, 0, wxALL, 5 );
	
	panelStatus->SetSizer( bSizer9 );
	panelStatus->Layout();
	bSizer9->Fit( panelStatus );
	notebookSource->AddPage( panelStatus, wxT("Adapter status"), true );
	panelOnline = new wxPanel( notebookSource, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	panelOnline->Enable( false );
	
	wxBoxSizer* bSizer51;
	bSizer51 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText6 = new wxStaticText( panelOnline, wxID_ANY, wxT("Please select a firmware you want to flash:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->SetFont( wxFont( 10, 74, 90, 92, false, wxT("Sans") ) );
	
	bSizer51->Add( m_staticText6, 0, wxALL, 5 );
	
	listCtrlOnlineVersions = new wxListCtrl( panelOnline, wxIDListCtrlOnlineVersions, wxDefaultPosition, wxSize( -1,-1 ), wxLC_REPORT );
	bSizer51->Add( listCtrlOnlineVersions, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );
	
	btnDownloadAndFlash = new wxButton( panelOnline, wxIDBtnDownloadAndFlash, wxT("Download and flash selected firmware"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer10->Add( btnDownloadAndFlash, 10, wxALL|wxLEFT, 5 );
	
	btnRefreshOnlineVersions = new wxButton( panelOnline, wxIDBtnRefreshOnlineVersions, wxT("Refresh firmware list"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( btnRefreshOnlineVersions, 0, wxALL, 5 );
	
	bSizer51->Add( bSizer10, 0, wxEXPAND, 5 );
	
	panelOnline->SetSizer( bSizer51 );
	panelOnline->Layout();
	bSizer51->Fit( panelOnline );
	notebookSource->AddPage( panelOnline, wxT("Online"), false );
	panelLocal = new wxPanel( notebookSource, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	panelLocal->Enable( false );
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText9 = new wxStaticText( panelLocal, wxID_ANY, wxT("Please select a file you want to flash:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->SetFont( wxFont( 10, 74, 90, 92, false, wxT("sans") ) );
	
	bSizer6->Add( m_staticText9, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxHORIZONTAL );
	
	textCtrlFileName = new wxTextCtrl( panelLocal, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer7->Add( textCtrlFileName, 2, wxALL, 5 );
	
	btnSelectFile = new wxButton( panelLocal, wxIDBtnSelectFile, wxT("..."), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer7->Add( btnSelectFile, 0, wxALL, 5 );
	
	bSizer6->Add( bSizer7, 1, wxEXPAND, 5 );
	
	btnFlashLocalFile = new wxButton( panelLocal, wxIDBtnFlashLocalFile, wxT("Flash file"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer6->Add( btnFlashLocalFile, 0, wxALL, 5 );
	
	panelLocal->SetSizer( bSizer6 );
	panelLocal->Layout();
	bSizer6->Fit( panelLocal );
	notebookSource->AddPage( panelLocal, wxT("Local"), false );
	panelLog = new wxPanel( notebookSource, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText61 = new wxStaticText( panelLog, wxID_ANY, wxT("Application log:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText61->SetFont( wxFont( 10, 74, 90, 92, false, wxT("Sans") ) );
	
	bSizer8->Add( m_staticText61, 0, wxALL, 5 );
	
	textCtrlAppLog = new wxTextCtrl( panelLog, wxIDTextCtrlAppLog, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxHSCROLL|wxTE_MULTILINE );
	bSizer8->Add( textCtrlAppLog, 1, wxALL|wxEXPAND, 5 );
	
	btnClearLog = new wxButton( panelLog, wxIDBtnClearLog, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer8->Add( btnClearLog, 0, wxALL, 5 );
	
	panelLog->SetSizer( bSizer8 );
	panelLog->Layout();
	bSizer8->Fit( panelLog );
	notebookSource->AddPage( panelLog, wxT("Application log"), false );
	
	bSizer11->Add( notebookSource, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( bSizer11 );
	this->Layout();
}
