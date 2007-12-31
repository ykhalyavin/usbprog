/*
 * (c) 2007, Robert Schilling
 *           Bernhard Walle <bernhard.walle@gmx.de>
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
#ifndef USBPROG_FORM_H
#define USBPROG_FORM_H

#include <string>

#include <wx/wx.h>
#include <wx/frame.h>

#include <wx/filedlg.h>
#include <wx/statbmp.h>
#include <wx/gauge.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/statbox.h>

#include <usbprog/devices.h>
#include <usbprog/firmwarepool.h>

class usbprogFrm : public wxFrame {
    private:
        DECLARE_EVENT_TABLE();

    public:
        usbprogFrm(wxWindow *parent, wxWindowID id = 1,
                const wxString &title = wxT("USBprog"),
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize);
        virtual ~usbprogFrm();

    protected:
        Firmware *getSelectedFirmware();
        Device *getSelectedDevice();
        void enableDisableDebug(wxCommandEvent &evt);
        void cleanCache(wxCommandEvent &evt);
        void downloadAll(wxCommandEvent &evt);
        void deleteCache(wxCommandEvent &evt);
        void exitMenuHandler(wxCommandEvent &evt);
        void aboutMenuHandler(wxCommandEvent &evt);
        void deviceComboHandler(wxCommandEvent &evt);
        void deviceRefreshHandler(wxCommandEvent &evt);
        void firmwareRefreshHandler(wxCommandEvent &evt);
        void status(const char *text);
        void status(const std::string &text);
        void browseButtonHandler(wxCommandEvent &evt);
        void onlineRadioHandler(wxCommandEvent &evt);
        void poolRadioHandler(wxCommandEvent &evt);
        void uploadHandler(wxCommandEvent &evt);

    private:
        void OnClose(wxCloseEvent& event);
        void CreateGUIControls();

    private:
        wxStaticBitmap *m_logoBitmap;
        wxStaticText   *m_deviceLabel;
        wxComboBox     *m_deviceCombo;
        wxButton       *m_refreshDevButton;
        wxStaticText   *m_devInfo;
        wxRadioButton  *m_onlineRB;
        wxComboBox     *m_poolCombo;
        wxButton       *m_refreshPoolButton;
        wxRadioButton  *m_localRB;
        wxTextCtrl     *m_pathText;
        wxButton       *m_browseButton;
        wxButton       *m_uploadButton;
        wxGauge        *m_progressGauge;
        wxFileDialog   *m_fileDialog;
        wxBoxSizer     *m_topBox;
        wxPanel        *m_panel;

        DeviceManager  *m_deviceManager;
        Firmwarepool   *m_firmwarepool;

        enum {
            ID_LOGOBITMAP = wxID_HIGHEST + 1,

            ID_DEVICELABEL,
            ID_DEVICECOMBO,
            ID_REFRESH_DEV_BUTTON,
            ID_DEV_INFO,
            ID_ONLINE_RB,
            ID_POOL_COMBO,
            ID_REFRESH_POOL_BUTTON,
            ID_LOCAL_RB,
            ID_PATH_TEXT,
            ID_BROWSE_BUTTON,
            ID_UPLOAD_BUTTON,
            ID_PROCESS_GAUGE,
            ID_COPYRIGHT_LABEL,
            ID_EXIT_MENU,
            ID_ABOUT_MENU,
            ID_CACHE_DELETE,
            ID_CACHE_CLEAN,
            ID_CACHE_DOWNLOAD_ALL,
            ID_DEBUG_ENABLE_DISABLE
        };
};

#endif /* USBPROG_FORM_H */

// vim: set sw=4 ts=4 fdm=marker et:
