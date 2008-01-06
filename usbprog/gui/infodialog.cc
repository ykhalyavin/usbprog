/*
 * (c) 2007-2008, Bernhard Walle <bernhard.walle@gmx.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <string>
#include <sstream>

#include <wx/gbsizer.h>
#include <wx/sizer.h>

#include "infodialog.h"
#include "usbprogApp.h"
#include <usbprog/stringutil.h>

using std::stringstream;

/* Event table {{{1 */

BEGIN_EVENT_TABLE(InfoDialog, wxDialog )
    EVT_BUTTON(ID_OK, InfoDialog::OnOkClick)
END_EVENT_TABLE()

IMPLEMENT_CLASS(InfoDialog, wxDialog)

/* }}} */


/* -------------------------------------------------------------------------- */
InfoDialog::InfoDialog(Firmware *firmware,
		wxWindow* parent, wxWindowID id, const wxString& caption,
		const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, caption, pos, size, style),
      m_firmware(firmware)
{
    createControls();
}

/* -------------------------------------------------------------------------- */
void InfoDialog::createControls()
{
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topSizer);

    // A second box sizer to give more space around the controls
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);

    // sizer for the device description
    wxGridBagSizer *descSizer = new wxGridBagSizer;

    // Name
    m_nameLabel = new wxStaticText(this, ID_NAME_LABEL, wxT("Name:"));
    wxFont font = m_nameLabel->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    m_nameLabel->SetFont(font);
    descSizer->Add(m_nameLabel, wxGBPosition(0, 0), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_nameDesc = new wxStaticText(this, ID_NAME_DESC,
            WXSTRING(m_firmware->getLabel()));
    descSizer->Add(m_nameDesc, wxGBPosition(0, 1), wxGBSpan(), 0, 0);

    // URL
    m_urlLabel = new wxStaticText(this, ID_URL_LABEL, wxT("URL:"));
    m_urlLabel->SetFont(font);
    descSizer->Add(m_urlLabel, wxGBPosition(1, 0), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_urlDesc = new wxStaticText(this, ID_URL_DESC,
            WXSTRING(m_firmware->getUrl()));
    descSizer->Add(m_urlDesc, wxGBPosition(1, 1), wxGBSpan(), 0, 0);

    // Filename
    m_filenameLabel = new wxStaticText(this, ID_URL_LABEL, wxT("File name:"));
    m_filenameLabel->SetFont(font);
    descSizer->Add(m_filenameLabel, wxGBPosition(2, 0), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_filenameDesc = new wxStaticText(this, ID_URL_DESC,
            WXSTRING(m_firmware->getFilename()));
    descSizer->Add(m_filenameDesc, wxGBPosition(2, 1), wxGBSpan(), 0, 0);

    // Author
    m_authorLabel = new wxStaticText(this, ID_AUTHOR_LABEL, wxT("Author:"));
    m_authorLabel->SetFont(font);
    descSizer->Add(m_authorLabel, wxGBPosition(3, 0), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_authorDesc = new wxStaticText(this, ID_AUTHOR_LABEL,
            WXSTRING(m_firmware->getAuthor()));
    descSizer->Add(m_authorDesc, wxGBPosition(3, 1), wxGBSpan(), 0, 0);

    // version
    m_versionLabel = new wxStaticText(this, ID_VERSION_LABEL, wxT("Version:"));
    m_versionLabel->SetFont(font);
    descSizer->Add(m_versionLabel, wxGBPosition(4, 0), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_verionDesc = new wxStaticText(this, ID_VERSION_DESC,
            WXSTRING(m_firmware->formatDateVersion()));
    descSizer->Add(m_verionDesc, wxGBPosition(4, 1), wxGBSpan(), 0, 0);

    // expand field
    wxStaticText *filler = new wxStaticText(this, ID_VERSION_LABEL, wxEmptyString);
    descSizer->Add(filler, wxGBPosition(4, 2), wxGBSpan(), wxEXPAND, 0);

    // device id(s)
    if (m_firmware->hasDeviceId()) {

        m_deviceIdLabel = new wxStaticText(this, ID_DEVICE_ID_LABEL,
                wxT("Device ID(s):"));
        m_deviceIdLabel->SetFont(font);
        descSizer->Add(m_deviceIdLabel, wxGBPosition(5, 0), wxGBSpan(),
                wxALIGN_RIGHT|wxRIGHT, 10);

        m_deviceIdDesc = new wxStaticText(this, ID_DEVICE_ID_LABEL,
                WXSTRING(m_firmware->formatDeviceId()));
        descSizer->Add(m_deviceIdDesc, wxGBPosition(5, 1), wxGBSpan(), 0, 0);
    }

    // Description
    m_descriptionLabel = new wxStaticText(this, ID_DESC_LABEL,
            wxT("Description:"));
    m_descriptionLabel->SetFont(font);
    descSizer->Add(m_descriptionLabel, wxGBPosition(6, 0), wxGBSpan(1, 1),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_descriptionText = new wxTextCtrl(this, ID_DESC_TEXT,
            WXSTRING(wordwrap(m_firmware->getDescription(), -1)),
            wxDefaultPosition, wxSize(300, 100),
            wxTE_MULTILINE|wxTE_READONLY|wxTE_WORDWRAP);
    descSizer->Add(m_descriptionText, wxGBPosition(7, 0), wxGBSpan(1, 3),
            wxEXPAND, 0);

    // Ok button
    m_okButton = new wxButton(this, ID_OK, wxT("&Close"));


    boxSizer->Add(descSizer);
    boxSizer->AddSpacer(15);
    boxSizer->Add(m_okButton, wxALIGN_RIGHT);

    topSizer->Add(boxSizer, 0, wxEXPAND|wxALL, 15);

    topSizer->Fit(this);
    GetSizer()->SetSizeHints(this);
}

/* -------------------------------------------------------------------------- */
void InfoDialog::OnOkClick(wxCommandEvent &evt)
{
    EndModal(wxID_OK);
}

// vim: set sw=4 ts=4 fdm=marker et:
