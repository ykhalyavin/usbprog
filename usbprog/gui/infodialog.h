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
#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <usbprog/firmwarepool.h>

#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/textctrl.h>

class InfoDialog : public wxDialog {
    DECLARE_CLASS(InfoDialog)
    DECLARE_EVENT_TABLE()

    public:
        InfoDialog(Firmware *firmware,
                wxWindow* parent = NULL,
                wxWindowID id = wxID_ANY,
                const wxString& caption = wxT("Firmware Information"),
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX);
        virtual ~InfoDialog() {}

    protected:
        void createControls();
        void OnOkClick(wxCommandEvent &evt);

    private:
        Firmware *m_firmware;
        wxStaticText *m_nameLabel;
        wxStaticText *m_nameDesc;
        wxStaticText *m_urlLabel;
        wxStaticText *m_urlDesc;
        wxStaticText *m_filenameLabel;
        wxStaticText *m_filenameDesc;
        wxStaticText *m_authorLabel;
        wxStaticText *m_authorDesc;
        wxStaticText *m_versionLabel;
        wxStaticText *m_verionDesc;
        wxStaticText *m_deviceIdLabel;
        wxStaticText *m_deviceIdDesc;
        wxStaticText *m_descriptionLabel;
        wxTextCtrl *m_descriptionText;
        wxButton *m_okButton;

        enum {
            ID_OK = wxID_HIGHEST + 1,
            ID_NAME_LABEL,
            ID_NAME_DESC,
            ID_URL_LABEL,
            ID_URL_DESC,
            ID_FILENAME_LABEL,
            ID_FILENAME_DESC,
            ID_AUTHOR_LABEL,
            ID_AUTHOR_DESC,
            ID_VERSION_LABEL,
            ID_VERSION_DESC,
            ID_DEVICE_ID_LABEL,
            ID_DEVICE_ID_DESC,
            ID_DESC_LABEL,
            ID_DESC_TEXT
        };
};


#endif /* INFODIALOG_H */

// vim: set sw=4 ts=4 fdm=marker et: :collapseFolds=1:
