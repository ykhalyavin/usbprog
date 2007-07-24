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

#include "AboutDialogGenerated.h"

///////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE( AboutDialogGenerated, wxDialog )
	EVT_BUTTON( wxIDBtnLinkEmbeddedProjects, AboutDialogGenerated::_wxFB_OnBtnLnkEmbeddedProjectsClick )
	EVT_BUTTON( wxIDBtnLnkSoftsprings, AboutDialogGenerated::_wxFB_OnBtnLnkSoftspringsClick )
END_EVENT_TABLE()

AboutDialogGenerated::AboutDialogGenerated( wxWindow* parent, int id, wxString title, wxPoint pos, wxSize size, int style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxVERTICAL );
	
	m_panel1 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel1->SetBackgroundColour( wxColour( 255, 255, 255 ) );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText1 = new wxStaticText( m_panel1, wxID_ANY, wxT("usbprog-online"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->SetFont( wxFont( 14, 74, 90, 92, false, wxT("Sans") ) );
	
	bSizer2->Add( m_staticText1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_bitmap1 = new wxStaticBitmap( m_panel1, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_bitmap1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_staticText3 = new wxStaticText( m_panel1, wxID_ANY, wxT("A tool to download new firmware and flash it to usbprog"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	bSizer2->Add( m_staticText3, 1, wxALIGN_CENTER_HORIZONTAL, 0 );
	
	m_staticline1 = new wxStaticLine( m_panel1, wxID_ANY, wxDefaultPosition, wxSize( 50,-1 ), wxLI_HORIZONTAL );
	bSizer2->Add( m_staticline1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_staticText2 = new wxStaticText( m_panel1, wxID_ANY, wxT("Homepage"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->SetFont( wxFont( 10, 74, 90, 92, false, wxT("Sans") ) );
	
	bSizer2->Add( m_staticText2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	btnLinkEmbeddedProjects = new wxButton( m_panel1, wxIDBtnLinkEmbeddedProjects, wxT("http://www.embedded-projects.net"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
	bSizer2->Add( btnLinkEmbeddedProjects, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_staticline11 = new wxStaticLine( m_panel1, wxID_ANY, wxDefaultPosition, wxSize( 50,-1 ), wxLI_HORIZONTAL );
	bSizer2->Add( m_staticline11, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_staticText4 = new wxStaticText( m_panel1, wxID_ANY, wxT("Developed by"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->SetFont( wxFont( 10, 74, 90, 92, false, wxT("Sans") ) );
	
	bSizer2->Add( m_staticText4, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_bitmap2 = new wxStaticBitmap( m_panel1, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_bitmap2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_staticText5 = new wxStaticText( m_panel1, wxID_ANY, wxT("Alois Flammensboeck"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_staticText5, 0, wxALIGN_CENTER|wxALL, 5 );
	
	btnLnkSoftsprings = new wxButton( m_panel1, wxIDBtnLnkSoftsprings, wxT("http://www.softsprings.de"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
	bSizer2->Add( btnLnkSoftsprings, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_panel1->SetSizer( bSizer2 );
	m_panel1->Layout();
	bSizer2->Fit( m_panel1 );
	bSizer13->Add( m_panel1, 1, wxEXPAND | wxALL, 5 );
	
	btnClose = new wxButton( this, wxID_OK, wxT("Ok"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer13->Add( btnClose, 0, wxALIGN_CENTER|wxALL, 5 );
	
	this->SetSizer( bSizer13 );
	this->Layout();
}
