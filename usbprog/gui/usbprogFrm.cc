/*
 * (c) 2007-2008, Robert Schilling
 *                Bernhard Walle <bernhard.walle@gmx.de>
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
#include <stdexcept>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cstdio>

#include <wx/arrstr.h>
#include <wx/gbsizer.h>
#include <wx/sizer.h>

#include <usbprog/util.h>
#include "usbprogFrm.h"
#include "usbprogApp.h"
#include "infodialog.h"
#include "pindialog.h"
#include "usbprogApp.h"

#include "usbprog.xpm"
#include "usbprog_icon.xpm"

/* -------------------------------------------------------------------------- */
using std::exception;
using std::string;
using std::vector;
using std::stringstream;
using std::ifstream;
using std::ios;
using std::fopen;
using std::fclose;
using std::setvbuf;

// Global Variables:
bool internetConnection = true;

/* Event table {{{ */

/* -------------------------------------------------------------------------- */
BEGIN_EVENT_TABLE(usbprogFrm,wxFrame)
	EVT_CLOSE(usbprogFrm::OnClose)
    EVT_MENU(ID_EXIT_MENU,              usbprogFrm::exitMenuHandler)
    EVT_MENU(ID_ABOUT_MENU,             usbprogFrm::aboutMenuHandler)
    EVT_MENU(ID_CACHE_DELETE,           usbprogFrm::deleteCache)
    EVT_MENU(ID_CACHE_CLEAN,            usbprogFrm::cleanCache)
    EVT_MENU(ID_CACHE_DOWNLOAD_ALL,     usbprogFrm::downloadAll)
    EVT_MENU(ID_DEBUG_ENABLE_DISABLE,   usbprogFrm::enableDisableDebug)
    EVT_MENU(ID_FIRMWARE_INFO,          usbprogFrm::showFirmwareInfo)
    EVT_MENU(ID_PIN_INFO,               usbprogFrm::showPinInfo)
    EVT_COMBOBOX(ID_DEVICECOMBO,        usbprogFrm::deviceComboHandler)
    EVT_BUTTON(ID_REFRESH_DEV_BUTTON,   usbprogFrm::deviceRefreshHandler)
    EVT_BUTTON(ID_REFRESH_POOL_BUTTON,  usbprogFrm::firmwareRefreshHandler)
    EVT_BUTTON(ID_BROWSE_BUTTON,        usbprogFrm::browseButtonHandler)
    EVT_RADIOBUTTON(ID_ONLINE_RB,       usbprogFrm::onlineRadioHandler)
    EVT_RADIOBUTTON(ID_LOCAL_RB,        usbprogFrm::poolRadioHandler)
    EVT_BUTTON(ID_UPLOAD_BUTTON,        usbprogFrm::uploadHandler)
END_EVENT_TABLE()

/* }}} */
/* GUIProgressNotifier {{{ */

class GUIProgressNotifier : public ProgressNotifier {
    public:
        GUIProgressNotifier(wxGauge *gauge);

    public:
        int progressed(double total, double now);
        void finished();

    private:
        wxGauge *m_gauge;
};

/* -------------------------------------------------------------------------- */
GUIProgressNotifier::GUIProgressNotifier(wxGauge *gauge)
    : m_gauge(gauge)
{}

/* -------------------------------------------------------------------------- */
int GUIProgressNotifier::progressed(double total, double now)
{
    m_gauge->SetValue(static_cast<int>(now/total*100));
    wxGetApp().Yield();
    return 1;
}

/* -------------------------------------------------------------------------- */
void GUIProgressNotifier::finished()
{
    m_gauge->SetValue(0);
}

/* }}} */
/* usbprogFrm {{{ */

/* -------------------------------------------------------------------------- */
usbprogFrm::usbprogFrm(wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint &position, const wxSize& size)
    : wxFrame(parent, id, title, position, size,
            wxDEFAULT_FRAME_STYLE & ~wxRESIZE_BORDER), m_deviceManager(NULL),
      m_firmwarepool(NULL)

{
    CreateGUIControls();

    m_deviceManager = new DeviceManager();
    m_firmwarepool = new Firmwarepool(Fileutil::configDir("usbprog"));

    // dummy event
    wxCommandEvent evt;
    firmwareRefreshHandler(evt);
    deviceRefreshHandler(evt);

    GetSizer()->SetSizeHints(this);
    m_topBox->SetSizeHints(m_panel);
}

/* -------------------------------------------------------------------------- */
usbprogFrm::~usbprogFrm()
{
    delete m_deviceManager;
    delete m_firmwarepool;
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::CreateGUIControls()
{
	SetIcon(usbprog_icon_xpm);
    CreateStatusBar();

    // Program
    wxMenu *programMenu = new wxMenu;
    programMenu->AppendCheckItem(ID_DEBUG_ENABLE_DISABLE, wxT("&Logging"),
            wxT("Enable/disable debugging output to file"));
    programMenu->AppendSeparator();
    programMenu->Append(ID_EXIT_MENU, wxT("E&xit\tCtrl-q"),
            wxT("Quit this program"));

    // Firmware
    wxMenu *firmwareMenu = new wxMenu;
    firmwareMenu->Append(ID_FIRMWARE_INFO, wxT("&Info\tF2"),
            wxT("Shows information about currently selected firmware"));
    firmwareMenu->Append(ID_PIN_INFO, wxT("&Pins\tF3"),
            wxT("Shows pin assignment of currently selected firmware"));

    // Cache
    wxMenu *cacheMenu = new wxMenu;
    cacheMenu->Append(ID_CACHE_DELETE, wxT("&Delete cache"),
            wxT("Deletes the whole cache from disk"));
    cacheMenu->Append(ID_CACHE_CLEAN, wxT("C&lean"),
            wxT("Deletes old firmware versions from the cache"));
    cacheMenu->AppendSeparator();
    cacheMenu->Append(ID_CACHE_DOWNLOAD_ALL, wxT("Download &all"),
            wxT("Deletes old firmware versions from the cache"));

    // Help
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(ID_ABOUT_MENU, wxT("&About\tF1"),
            wxT("Show version information"));

    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(programMenu, wxT("&Program"));
    menuBar->Append(firmwareMenu, wxT("&Firmware"));
    menuBar->Append(cacheMenu, wxT("&Cache"));
    menuBar->Append(helpMenu, wxT("&Help"));

    SetMenuBar(menuBar);

    // panel for the main GUI
    m_panel = new wxPanel(this, wxID_ANY);

    m_topBox = new wxBoxSizer(wxHORIZONTAL);
    wxGridBagSizer *topSizer = new wxGridBagSizer;
    wxBoxSizer *frameSizer = new wxBoxSizer(wxHORIZONTAL);

    // logo on the left
	m_logoBitmap = new wxStaticBitmap(m_panel, ID_LOGOBITMAP,
            wxBitmap(usbprog_xpm));
	m_logoBitmap->Enable(true);
    topSizer->Add(m_logoBitmap, wxGBPosition(0, 0), wxGBSpan(5, 1),
            wxRIGHT, 10);

    // device label
    m_deviceLabel = new wxStaticText(m_panel, ID_DEVICELABEL, wxT("Device"));
    topSizer->Add(m_deviceLabel, wxGBPosition(0, 1), wxGBSpan(),
            wxALIGN_CENTER_VERTICAL);

    // device combo box
    m_deviceCombo = new wxComboBox(m_panel, ID_DEVICECOMBO, wxT(""),
            wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
    wxSize sz = m_deviceCombo->GetMinSize();
    sz.SetWidth(200);
    m_deviceCombo->SetMinSize(sz);
    topSizer->Add(m_deviceCombo, wxGBPosition(0, 2), wxGBSpan(),
            wxLEFT|wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

    // refresh devices button
    m_refreshDevButton = new wxButton(m_panel, ID_REFRESH_DEV_BUTTON,
            wxT("&Refresh"));
    topSizer->Add(m_refreshDevButton, wxGBPosition(0, 3), wxGBSpan(),
            wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);

    // device info text
    m_devInfo = new wxStaticText(m_panel, ID_DEV_INFO, wxT("No device selected"));
    topSizer->Add(m_devInfo, wxGBPosition(1, 2), wxGBSpan(1, 2),
            wxLEFT|wxRIGHT | wxALIGN_TOP, 5);

    // online radio button
    m_onlineRB = new wxRadioButton(m_panel, ID_ONLINE_RB, wxT("&Online Pool"));
    m_onlineRB->SetValue(true);
    topSizer->Add(m_onlineRB, wxGBPosition(2, 1), wxGBSpan(),
            wxALIGN_CENTER_VERTICAL);

    // pool combo box
    m_poolCombo = new wxComboBox(m_panel, ID_POOL_COMBO, wxT(""),
            wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
    sz = m_poolCombo->GetMinSize();
    sz.SetWidth(200);
    m_poolCombo->SetMinSize(sz);
    topSizer->Add(m_poolCombo, wxGBPosition(2, 2), wxGBSpan(),
            wxTOP|wxBOTTOM|wxLEFT|wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

    // pool refresh
    m_refreshPoolButton = new wxButton(m_panel, ID_REFRESH_POOL_BUTTON,
            wxT("R&efresh"));
    topSizer->Add(m_refreshPoolButton, wxGBPosition(2, 3), wxGBSpan(),
            wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);

    // local disk radio button
    m_localRB = new wxRadioButton(m_panel, ID_LOCAL_RB, wxT("&Local disk"));
    topSizer->Add(m_localRB, wxGBPosition(3, 1), wxGBSpan(),
            wxALIGN_CENTER_VERTICAL);

    // path text
    m_pathText = new wxTextCtrl(m_panel, ID_PATH_TEXT, wxT(""));
    m_pathText->Enable(false);
    sz = m_pathText->GetMinSize();
    sz.SetWidth(200);
    m_pathText->SetMinSize(sz);
    topSizer->Add(m_pathText, wxGBPosition(3, 2), wxGBSpan(),
            wxTOP|wxBOTTOM|wxLEFT|wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

    // pool refresh
    m_browseButton = new wxButton(m_panel, ID_BROWSE_BUTTON,
            wxT("&Browse"));
    m_browseButton->Enable(false);
    topSizer->Add(m_browseButton, wxGBPosition(3, 3), wxGBSpan(),
            wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);

    // upload button
    m_uploadButton = new wxButton(m_panel, ID_UPLOAD_BUTTON, wxT("U&pload"));
    topSizer->Add(m_uploadButton, wxGBPosition(4, 3), wxGBSpan(),
            wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);

    // progress bar
    m_progressGauge = new wxGauge(m_panel, ID_PROCESS_GAUGE, 100, wxDefaultPosition,
            wxSize(5, 5));
    topSizer->Add(m_progressGauge, wxGBPosition(5, 1), wxGBSpan(1, 3),
            wxTOP|wxBOTTOM|wxLEFT| wxALIGN_CENTER_VERTICAL |
            wxEXPAND, 5);

    m_urlInfo = new wxStaticText(m_panel, ID_URL_INFO,
            wxT("Read more: http://www.embedded-projects.net/usbprog/"));
    topSizer->Add(m_urlInfo, wxGBPosition(6, 1), wxGBSpan(1, 3),
            wxTOP|wxBOTTOM|wxLEFT| wxALIGN_CENTER_VERTICAL |
            wxEXPAND, 5);

    // file dialog
	m_fileDialog = new wxFileDialog(m_panel, wxT("Choose a file"),
            wxT(""), wxT(""), wxT("*.bin"), wxOPEN);

    m_topBox->Add(topSizer, wxEXPAND, wxALL, 10);

    frameSizer->Add(m_topBox, wxEXPAND);
    SetSizer(frameSizer);
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::OnClose(wxCloseEvent& event)
{
	Destroy();
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::onlineRadioHandler(wxCommandEvent& event)
{
    if (m_onlineRB->GetValue()) {
        m_poolCombo->Enable(true);
        m_refreshPoolButton->Enable(true);
        m_pathText->Enable(false);
        m_browseButton->Enable(false);
    }
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::poolRadioHandler(wxCommandEvent& evt)
{
    if (m_localRB->GetValue()) {
        m_pathText->Enable(true);
        m_browseButton->Enable(true);
        m_poolCombo->Enable(false);
        m_refreshPoolButton->Enable(false);
    }
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::exitMenuHandler(wxCommandEvent& event)
{
	Destroy();
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::aboutMenuHandler(wxCommandEvent &evt)
{
    wxMessageDialog dlg(this,
            wxT("USBprog " USBPROG_VERSION_STRING "\n"
                "(c) 2007, Bernhard Walle <bernhard.walle@gmx.de>\n"
                "http://www.embedded-projects.net/usbprog"),
            wxT("About ..."),
            wxOK | wxICON_INFORMATION);
    dlg.ShowModal();
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::cleanCache(wxCommandEvent &evt)
{
    m_firmwarepool->cleanCache();
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::deleteCache(wxCommandEvent &evt)
{
    m_firmwarepool->deleteCache();
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::downloadAll(wxCommandEvent &evt)
{
    vector<Firmware *> firmwares = m_firmwarepool->getFirmwareList();

    int totalNo = firmwares.size() - 1;
    int currentNo = 0;
    wxBusyCursor cursor;

    for (vector<Firmware *>::const_iterator it = firmwares.begin();
            it != firmwares.end(); ++it) {
        try {
            m_progressGauge->SetValue(int((double(currentNo++)/totalNo*100)));
            if (m_firmwarepool->isFirmwareOnDisk((*it)->getName()))
                status("Firmware " + (*it)->getLabel() + " is already there.");
            else {
                status("Downloading " + (*it)->getLabel() + " ...");
                m_firmwarepool->downloadFirmware((*it)->getName());
            }
        } catch (const std::exception &ex) {
            status("Error while downloading firmware " + (*it)->getName() +
                ": " + ex.what());
        }
    }

    m_progressGauge->SetValue(100);
    wxGetApp().Yield();

    status("Downloading of firmware files has been successful!");

    m_progressGauge->SetValue(0);
    wxGetApp().Yield();
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::browseButtonHandler(wxCommandEvent& event)
{
	if (m_fileDialog->ShowModal() == wxID_OK) {
        m_pathText->SetValue(m_fileDialog->GetPath());
    }
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::deviceComboHandler(wxCommandEvent &evt)
{
    if (!m_deviceCombo->GetValue()) {
        m_devInfo->SetLabel(wxT("No device selected"));
        return;
    }

    Device *dev = getSelectedDevice();
    if (!dev) {
        status("Invalid device selected.");
        return;
    }

    stringstream ss;
    ss << "Bus: " << dev->getBus() << ", ";
    ss << "Device: " << dev->getDevice();
    m_devInfo->SetLabel(WXSTRING(ss.str()));
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::deviceRefreshHandler(wxCommandEvent &evt)
{
    try {
        m_deviceManager->discoverUpdateDevices(m_firmwarepool);
    } catch (const exception &e) {
        status(e.what());
    }

    m_deviceCombo->Clear();
    m_deviceCombo->SetValue(wxT(""));

    for (int i = 0; i < int(m_deviceManager->getNumberUpdateDevices()); i++) {
        Device *dev = m_deviceManager->getDevice(i);

        stringstream ss;
        ss << i << ": ";
        if (dev->getName().size() == 0)
            ss << dev->toShortString();
        else
            ss << dev->getName();

        m_deviceCombo->Append(WXSTRING(ss.str()));
    }

    wxCommandEvent dummy;
    deviceComboHandler(dummy);
}

/* -------------------------------------------------------------------------- */
Device *usbprogFrm::getSelectedDevice()
{
    int number;
    stringstream ss;
    ss << m_deviceCombo->GetValue();
    ss >> number;

    if (number < 0 || number >= int(m_deviceManager->getNumberUpdateDevices()))
        return NULL;

    m_deviceManager->setCurrentUpdateDevice(number);
    return m_deviceManager->getDevice(number);
}

/* -------------------------------------------------------------------------- */
Firmware *usbprogFrm::getSelectedFirmware()
{
    string value = (const char *)m_poolCombo->GetValue().mb_str();

    Firmware *found = NULL;
    vector<Firmware *> fws = m_firmwarepool->getFirmwareList();
    for (vector<Firmware *>::const_iterator it = fws.begin();
            it != fws.end(); ++it) {
        Firmware *fw = *it;
        if (fw->getLabel() == value) {
            found = fw;
            break;
        }
    }

    return found;
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::firmwareRefreshHandler(wxCommandEvent &evt)
{
    try {
        m_firmwarepool->downloadIndex(DEFAULT_INDEX_URL);
    } catch (const DownloadError &e) {
        internetConnection = false;
        status((string("Error downloading index file: ") + e.what()));
    } catch (const exception &e) {
        status(e.what());
    }
    try {
        m_firmwarepool->readIndex();
    } catch (const ParseError &e) {
        status(string("Unable to parse the firmware file: ") + e.what());
    } catch (const IOError &e) {
        status(string("I/O error when reading firmware file: ") + e.what());
    }

    m_poolCombo->Clear();
    m_poolCombo->SetValue(wxT(""));

    vector<Firmware *> fws = m_firmwarepool->getFirmwareList();
    for (vector<Firmware *>::const_iterator it = fws.begin();
            it != fws.end(); ++it) {
        Firmware *fw = *it;
        if (internetConnection ||
                m_firmwarepool->isFirmwareOnDisk(fw->getName()))
            m_poolCombo->Append(WXSTRING(fw->getLabel()));
    }
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::status(const char *text)
{
     status(string(text));
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::status(const string &text)
{
    SetStatusText(WXSTRING(text));
    wxGetApp().Yield();
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::uploadHandler(wxCommandEvent &evt)
{
    ByteVector data;
    wxBusyCursor cursor;

    if (m_onlineRB->GetValue()) {
        Firmware *fw = getSelectedFirmware();
        if (!fw) {
            status("No firmware specified.");
            return;
        }

        try {
            if (!m_firmwarepool->isFirmwareOnDisk(fw->getName())) {
                status("Downloading firmware ...");
                m_firmwarepool->downloadFirmware(fw->getName());
            }
            m_firmwarepool->fillFirmware(fw->getName());
        } catch (const std::exception &e) {
            status(string("Downloading firmware failed: ") + e.what());
            return;
        }

        data = fw->getData();

    } else {
        if (m_pathText->GetValue().size() == 0) {
            status("No firmware specified");
            return;
        }

        try {
            Firmwarepool::readFromFile(string(m_pathText->GetValue().mb_str()), data);
        } catch (const IOError &ioe) {
            status(string("Error while reading firmware file: ") + ioe.what());
            return;
        }
    }

    Device *dev = getSelectedDevice();
    if (!dev) {
        status("No device selected");
        return;
    }

    // switch in update mode
    if (!dev->isUpdateMode()) {
        try {
            status("Switching to update mode ...");
            m_deviceManager->switchUpdateMode();
        } catch (const IOError &err) {
            status(string("I/O Error: ") + err.what());
            return;
        }
    }

    dev = getSelectedDevice();
    if (!dev) {
        status("Device not found after switching to update mode");
        return;
    }

    UsbprogUpdater upd(dev);
    GUIProgressNotifier notifier(m_progressGauge);
    upd.setProgress(&notifier);
    try {
        status("Opening device ...");
        upd.updateOpen();
        status("Performing update ...");
        upd.writeFirmware(data);
        status("Starting device ...");
        upd.startDevice();
        upd.updateClose();
    } catch (const IOError &err) {
        status(string("Error while uploading firmware: ") + err.what());
        return;
    }

    status("Upload successful!");
    wxGetApp().Yield();
    usbprog_sleep(2);
    wxGetApp().Yield();
    wxCommandEvent dummy;
    deviceRefreshHandler(evt);
    status("");
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::enableDisableDebug(wxCommandEvent &evt)
{
    Debug *dbg = Debug::debug();

    if (dbg->isDebugEnabled()) {
        dbg->setLevel(Debug::DL_NONE);
        fclose(dbg->getFileHandle());
        dbg->setFileHandle(NULL);
    } else {
        wxFileDialog fdg(m_panel, wxT("Choose a file"),
            wxT(""), wxT(""), wxT("*.log"), wxSAVE);
        if (fdg.ShowModal() != wxID_OK)
            dbg->setFileHandle(stderr);
        else {
            FILE *fp = fopen(fdg.GetPath().mb_str(), "a");
            if (fp) {
                setvbuf(fp, NULL, _IONBF, 0);
                dbg->setFileHandle(fp);
            } else {
                status("Could not create specified file, using standard error");
                dbg->setFileHandle(stderr);
            }
        }
        dbg->setLevel(Debug::DL_TRACE);
    }
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::showFirmwareInfo(wxCommandEvent &evt)
{
    Firmware *fw = getSelectedFirmware();
    if (!fw) {
        status("No firmware selected");
        return;
    }

    InfoDialog dialog(fw, this);
    dialog.ShowModal();
}

/* -------------------------------------------------------------------------- */
void usbprogFrm::showPinInfo(wxCommandEvent &evt)
{
    Firmware *fw = getSelectedFirmware();
    if (!fw) {
        status("No firmware selected");
        return;
    }

    PinDialog dialog(fw, this);
    dialog.ShowModal();
}

/* }}} */

// vim: set sw=4 ts=4 fdm=marker et: :collapseFolds=1:
