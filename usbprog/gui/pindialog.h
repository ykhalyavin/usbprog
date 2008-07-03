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
#ifndef PINDIALOG_H
#define PINDIALOG_H

#include <usbprog/firmwarepool.h>

#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/statbmp.h>

class PinDialog : public wxDialog {
    DECLARE_CLASS(PinDialog)
    DECLARE_EVENT_TABLE()

    public:
        PinDialog(Firmware *firmware,
                wxWindow* parent = NULL,
                wxWindowID id = wxID_ANY,
                const wxString& caption = wxT("Pin Assignment Information"),
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX);
        virtual ~PinDialog() {}

    protected:
        void createControls();
        void OnOkClick(wxCommandEvent &evt);

    private:
        Firmware *m_firmware;
        wxStaticText *m_p1Label;
        wxStaticText *m_p1Desc;
        wxStaticText *m_p2Label;
        wxStaticText *m_p2Desc;
        wxStaticText *m_p3Label;
        wxStaticText *m_p3Desc;
        wxStaticText *m_p4Label;
        wxStaticText *m_p4Desc;
        wxStaticText *m_p5Label;
        wxStaticText *m_p5Desc;
        wxStaticText *m_p6Label;
        wxStaticText *m_p6Desc;
        wxStaticText *m_p7Label;
        wxStaticText *m_p7Desc;
        wxStaticText *m_p8Label;
        wxStaticText *m_p8Desc;
        wxStaticText *m_p9Label;
        wxStaticText *m_p9Desc;
        wxStaticText *m_p10Label;
        wxStaticText *m_p10Desc;
        wxStaticText *m_txLabel;
        wxStaticText *m_txDesc;
        wxStaticText *m_rxLabel;
        wxStaticText *m_rxDesc;
        wxStaticText *m_ledLabel;
        wxStaticText *m_ledDesc;
        wxStaticText *m_jpLabel;
        wxStaticText *m_jpDesc;
        wxStaticBitmap *m_pinBitmap;

        wxButton *m_okButton;

        enum {
            ID_OK = wxID_HIGHEST + 1,
            ID_P1_LABEL,
            ID_P1_DESC,
            ID_P2_LABEL,
            ID_P2_DESC,
            ID_P3_LABEL,
            ID_P3_DESC,
            ID_P4_LABEL,
            ID_P4_DESC,
            ID_P5_LABEL,
            ID_P5_DESC,
            ID_P6_LABEL,
            ID_P6_DESC,
            ID_P7_LABEL,
            ID_P7_DESC,
            ID_P8_LABEL,
            ID_P8_DESC,
            ID_P9_LABEL,
            ID_P9_DESC,
            ID_P10_LABEL,
            ID_P10_DESC,
            ID_TX_LABEL,
            ID_TX_DESC,
            ID_RX_LABEL,
            ID_RX_DESC,
            ID_LED_LABEL,
            ID_LED_DESC,
            ID_JP_LABEL,
            ID_JP_DESC,
            ID_PIN_BITMAP
        };
};


#endif /* PINDIALOG_H */

// vim: set sw=4 ts=4 fdm=marker et: :collapseFolds=1:
