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

#include <usbprog/stringutil.h>

#include "pindialog.h"
#include "usbprogApp.h"
#include "pins.xpm"

using std::stringstream;

/* Event table {{{1 */

BEGIN_EVENT_TABLE(PinDialog, wxDialog )
    EVT_BUTTON(ID_OK, PinDialog::OnOkClick)
END_EVENT_TABLE()

IMPLEMENT_CLASS(PinDialog, wxDialog)

/* }}} */


/* -------------------------------------------------------------------------- */
PinDialog::PinDialog(Firmware *firmware,
		wxWindow* parent, wxWindowID id, const wxString& caption,
		const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, caption, pos, size, style),
      m_firmware(firmware)
{
    createControls();
}

/* -------------------------------------------------------------------------- */
void PinDialog::createControls()
{
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topSizer);

    // A second box sizer to give more space around the controls
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);

    // sizer for the device description
    wxGridBagSizer *descSizer = new wxGridBagSizer;

    // P1
    m_p1Label = new wxStaticText(this, ID_P1_LABEL, wxT("P1:"));
    wxFont font = m_p1Label->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    m_p1Label->SetFont(font);
    descSizer->Add(m_p1Label, wxGBPosition(0, 0), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_p1Desc = new wxStaticText(this, ID_P1_DESC,
            WXSTRING(m_firmware->getPin("P1")));
    descSizer->Add(m_p1Desc, wxGBPosition(0, 1), wxGBSpan(),
            wxRIGHT, 20);

    // P2
    m_p2Label = new wxStaticText(this, ID_P2_LABEL, wxT("P2:"));
    m_p2Label->SetFont(font);
    descSizer->Add(m_p2Label, wxGBPosition(0, 2), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_p2Desc = new wxStaticText(this, ID_P2_DESC,
            WXSTRING(m_firmware->getPin("P2")));
    descSizer->Add(m_p2Desc, wxGBPosition(0, 3), wxGBSpan(), 0, 0);

    // P3
    m_p3Label = new wxStaticText(this, ID_P3_LABEL, wxT("P3:"));
    m_p3Label->SetFont(font);
    descSizer->Add(m_p3Label, wxGBPosition(1, 0), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_p3Desc = new wxStaticText(this, ID_P3_DESC,
            WXSTRING(m_firmware->getPin("P3")));
    descSizer->Add(m_p3Desc, wxGBPosition(1, 1), wxGBSpan(),
            wxRIGHT, 20);

    // P4
    m_p4Label = new wxStaticText(this, ID_P4_LABEL, wxT("P4:"));
    m_p4Label->SetFont(font);
    descSizer->Add(m_p4Label, wxGBPosition(1, 2), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_p4Desc = new wxStaticText(this, ID_P4_DESC,
            WXSTRING(m_firmware->getPin("P4")));
    descSizer->Add(m_p4Desc, wxGBPosition(1, 3), wxGBSpan(), 0, 0);

    // P5
    m_p5Label = new wxStaticText(this, ID_P5_LABEL, wxT("P5:"));
    m_p5Label->SetFont(font);
    descSizer->Add(m_p5Label, wxGBPosition(2, 0), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_p5Desc = new wxStaticText(this, ID_P5_DESC,
            WXSTRING(m_firmware->getPin("P5")));
    descSizer->Add(m_p5Desc, wxGBPosition(2, 1), wxGBSpan(),
            wxRIGHT, 20);

    // P6
    m_p6Label = new wxStaticText(this, ID_P6_LABEL, wxT("P6:"));
    m_p6Label->SetFont(font);
    descSizer->Add(m_p6Label, wxGBPosition(2, 2), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_p6Desc = new wxStaticText(this, ID_P6_DESC,
            WXSTRING(m_firmware->getPin("P6")));
    descSizer->Add(m_p6Desc, wxGBPosition(2, 3), wxGBSpan(), 0, 0);

    // P7
    m_p7Label = new wxStaticText(this, ID_P7_LABEL, wxT("P7:"));
    m_p7Label->SetFont(font);
    descSizer->Add(m_p7Label, wxGBPosition(3, 0), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_p7Desc = new wxStaticText(this, ID_P7_DESC,
            WXSTRING(m_firmware->getPin("P7")));
    descSizer->Add(m_p7Desc, wxGBPosition(3, 1), wxGBSpan(),
            wxRIGHT, 20);

    // P8
    m_p8Label = new wxStaticText(this, ID_P8_LABEL, wxT("P8:"));
    m_p8Label->SetFont(font);
    descSizer->Add(m_p8Label, wxGBPosition(3, 2), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_p8Desc = new wxStaticText(this, ID_P8_DESC,
            WXSTRING(m_firmware->getPin("P8")));
    descSizer->Add(m_p8Desc, wxGBPosition(3, 3), wxGBSpan(), 0, 0);

    // P9
    m_p9Label = new wxStaticText(this, ID_P9_LABEL, wxT("P9:"));
    m_p9Label->SetFont(font);
    descSizer->Add(m_p9Label, wxGBPosition(4, 0), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_p9Desc = new wxStaticText(this, ID_P9_DESC,
            WXSTRING(m_firmware->getPin("P9")));
    descSizer->Add(m_p9Desc, wxGBPosition(4, 1), wxGBSpan(),
            wxRIGHT, 20);

    // P10
    m_p10Label = new wxStaticText(this, ID_P10_LABEL, wxT("P10:"));
    m_p10Label->SetFont(font);
    descSizer->Add(m_p10Label, wxGBPosition(4, 2), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_p10Desc = new wxStaticText(this, ID_P10_DESC,
            WXSTRING(m_firmware->getPin("P10")));
    descSizer->Add(m_p10Desc, wxGBPosition(4, 3), wxGBSpan(), 0, 0);

    // LED
    m_ledLabel = new wxStaticText(this, ID_LED_LABEL, wxT("LED:"));
    m_ledLabel->SetFont(font);
    descSizer->Add(m_ledLabel, wxGBPosition(5, 0), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_ledDesc = new wxStaticText(this, ID_LED_DESC,
            WXSTRING(m_firmware->getPin("LED")));
    descSizer->Add(m_ledDesc, wxGBPosition(5, 1), wxGBSpan(),
            wxRIGHT, 20);

    // JP
    m_jpLabel = new wxStaticText(this, ID_JP_LABEL, wxT("JP:"));
    m_jpLabel->SetFont(font);
    descSizer->Add(m_jpLabel, wxGBPosition(5, 2), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_jpDesc = new wxStaticText(this, ID_JP_DESC,
            WXSTRING(m_firmware->getPin("JP")));
    descSizer->Add(m_jpDesc, wxGBPosition(5, 3), wxGBSpan(), 0, 0);

    // TX
    m_txLabel = new wxStaticText(this, ID_TX_LABEL, wxT("TX:"));
    m_txLabel->SetFont(font);
    descSizer->Add(m_txLabel, wxGBPosition(6, 0), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_txDesc = new wxStaticText(this, ID_TX_DESC,
            WXSTRING(m_firmware->getPin("TX")));
    descSizer->Add(m_txDesc, wxGBPosition(6, 1), wxGBSpan(),
            wxRIGHT, 20);

    // RX
    m_rxLabel = new wxStaticText(this, ID_RX_LABEL, wxT("RX:"));
    m_rxLabel->SetFont(font);
    descSizer->Add(m_rxLabel, wxGBPosition(6, 2), wxGBSpan(),
            wxALIGN_RIGHT|wxRIGHT, 10);

    m_rxDesc = new wxStaticText(this, ID_RX_DESC,
            WXSTRING(m_firmware->getPin("RX")));
    descSizer->Add(m_rxDesc, wxGBPosition(6, 3), wxGBSpan(), 0, 0);

    // logo on the right
	m_pinBitmap = new wxStaticBitmap(this, ID_PIN_BITMAP,
            wxBitmap(pins_xpm));
	m_pinBitmap->Enable(true);
    descSizer->Add(m_pinBitmap, wxGBPosition(0, 4), wxGBSpan(7, 1),
            wxLEFT|wxALIGN_TOP, 40);

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
void PinDialog::OnOkClick(wxCommandEvent &evt)
{
    EndModal(wxID_OK);
}

// vim: set sw=4 ts=4 fdm=marker et:
