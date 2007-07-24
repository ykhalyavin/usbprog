/***************************************************************************
 *            AboutDialog.cpp
 *
 *  Tue Jul 24 11:47:59 2007
 *  Copyright  2007  User
 *  Email
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 #include "AboutDialog.h"
 #include "../images/hardware.xpm"
 #include "../images/SoftspringsLogo.xpm"
 
AboutDialog::AboutDialog( wxWindow* parent, int id, wxString title, wxPoint pos, wxSize size, int style ) : AboutDialogGenerated( parent, id, title, pos, size, style )
{
		hardware = new wxBitmap((const char **) hardware_xpm);
		m_bitmap1->SetBitmap(*hardware);
		softspringsLogo = new wxBitmap((const char **) SoftspringsLogo_xpm);
		m_bitmap2->SetBitmap(*softspringsLogo
	);
}


void AboutDialog::OnBtnLnkEmbeddedProjectsClick( wxCommandEvent& event )
{
		ExecuteURL(_T("http://www.embedded-projects.net"));
}

void AboutDialog::OnBtnLnkSoftspringsClick( wxCommandEvent& event )
{
		ExecuteURL(_T("http://www.softsprings.de"));
}

void AboutDialog::ExecuteURL(const wxString &link)
{

#if _WIN32
	wxLaunchWithDefaultBrowser(link);
#else
	// variable declarations
	 wxArrayString browsers;
	 wxPathList path_list;
	 bool BrowserWasFound = false;
	 unsigned int i = 0;
	 wxString path;
	
	// Add directories to wxPathList's search path from PATH environment variable
	path_list.AddEnvList(wxT("PATH"));
	
	// Add browsers filenames. First item = most priority
	browsers.Add(wxT("firefox"));
	browsers.Add(wxT("firefox-bin"));
	browsers.Add(wxT("mozilla"));
	browsers.Add(wxT("mozilla-bin"));
	browsers.Add(wxT("opera"));
	browsers.Add(wxT("konqueror"));
	browsers.Add(wxT("epiphany"));
	
	for (i = 0; i < browsers.GetCount(); i++) {
	 path = path_list.FindAbsoluteValidPath(browsers[i]);
	 if (path.IsEmpty()) {
	  continue;
	 } else {
	  BrowserWasFound = true;
	  break;
	 }
	}
	
	browsers.Clear();
	
	if (BrowserWasFound) {
	 path += wxT(" ");
	 path += link;
	 ::wxExecute(path);
	} else {
	 //wxMessageBox(wxT("No browser has been found."),MessageBoxHeader);
	}
	
#endif
}
