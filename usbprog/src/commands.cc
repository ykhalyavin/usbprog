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
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <vector>
#include <fstream>

#include <usbprog/firmwarepool.h>
#include <usbprog/util.h>
#include <usbprog/stringutil.h>

#include "commands.h"
#include "configuration.h"
#include "usbprog.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

using std::vector;
using std::ostream;
using std::stringstream;
using std::string;
using std::cout;
using std::endl;
using std::left;
using std::right;
using std::setw;
using std::setfill;
using std::max;
using std::find;
using std::hex;
using std::ifstream;
using std::ios;

#define BUFFERSIZE       2048

/* ListCommand {{{1 */

/* -------------------------------------------------------------------------- */
ListCommand::ListCommand(Firmwarepool *firmwarepool)
    : AbstractCommand("list"), m_firmwarepool(firmwarepool)
{}

/* -------------------------------------------------------------------------- */
bool ListCommand::execute(CommandArgVector args, ostream &os)
    throw (ApplicationError)
{
    StringList firmwarelist = m_firmwarepool->getFirmwareNameList();

    size_t maxSize = 0;
    for (StringList::const_iterator it = firmwarelist.begin();
            it != firmwarelist.end(); ++it)
        maxSize = max(maxSize, it->size());

    maxSize += 2;

    for (StringList::const_iterator it = firmwarelist.begin();
            it != firmwarelist.end(); ++it) {
        Firmware *fw = m_firmwarepool->getFirmware(*it);
        os << left << setw(maxSize) << fw->getName();
        if (m_firmwarepool->isFirmwareOnDisk(fw->getName()))
            os << "[*] ";
        else
            os << "[ ] ";
        os << fw->getLabel() << endl;
    }

    if (!Configuration::config()->getBatchMode())
        os << endl << "*: Firmware file downloaded" << endl;

    return true;
}

/* -------------------------------------------------------------------------- */
StringVector ListCommand::aliases() const
{
    StringVector ret;
    ret.push_back("firmwares");
    return ret;
}

/* -------------------------------------------------------------------------- */
string ListCommand::help() const
{
    return "Lists all available firmwares.";
}

/* -------------------------------------------------------------------------- */
void ListCommand::printLongHelp(ostream &os) const
{
    os << "Name:            list\n"
       << "Aliases:         firmwares\n\n"
       << "Description:\n"
       << "Prints a list of all availalbe firmwares. The identifier has\n"
       << "to be used for other commands."
       << endl;
}

/* InfoCommand {{{1 */
InfoCommand::InfoCommand(Firmwarepool *firmwarepool)
    : AbstractCommand("info"), m_firmwarepool(firmwarepool)
{}

/* -------------------------------------------------------------------------- */
bool InfoCommand::execute(CommandArgVector args, ostream &os)
    throw (ApplicationError)
{
    string fwstr = args[0]->getString();
    Firmware *fw = m_firmwarepool->getFirmware(fwstr);
    if (!fw)
        throw ApplicationError(fwstr + ": Invalid firmware specified.");

    os << "Identifier   : " << fw->getName() << endl;
    os << "Name         : " << fw->getLabel() << endl;
    os << "URL          : " << fw->getUrl() << endl;
    os << "File name    : " << fw->getFilename() << endl;
    os << "Author       : " << fw->getAuthor() << endl;
    os << "Version      : " << fw->getVersion() << " ["
                            << fw->getDate().getDateTimeString(DTF_ISO_DATE)
                            << "]" << endl;
    if (fw->getMD5Sum().size() > 0)
        os << "MD5sum       : " << fw->getMD5Sum() << endl;

    // vendor ID and/or Product ID
    bool hasDeviceInfo = fw->getVendorId() != 0 || fw->getProductId() != 0 ||
            fw->getBcdDevice() != 0;
    if (hasDeviceInfo)
        os << "Device ID(s) : ";
    if (fw->getVendorId() != 0)
        os << "Vendor: 0x" << setw(4) << hex << setfill('0') << fw->getVendorId();
    if (fw->getVendorId() != 0 && fw->getProductId() != 0)
        os << ", ";
    if (fw->getProductId() != 0)
        os << "Product: 0x" << setw(4) << hex << fw->getProductId();
    if (fw->getBcdDevice() != 0 && (fw->getProductId() != 0 
                || fw->getVendorId() != 0))
        os << ", ";
    if (fw->getBcdDevice() != 0)
        os << "BCDDevice: 0x" << setw(4) << hex << fw->getBcdDevice();
    if (hasDeviceInfo)
        os << endl;

    os << endl;
    os << "Description" << endl;
    os << wordwrap(fw->getDescription(), DEFAULT_TERMINAL_WIDTH) << endl;

    if (!Configuration::config()->getBatchMode()) {
        os << endl;
        os << "For information about the Pin assignment, use the "
           << "\"pin " << fw->getName() << "\" command." << endl;
    }

    // reset fill character
    os << setfill(' ');

    return true;
}

/* -------------------------------------------------------------------------- */
size_t InfoCommand::getArgNumber() const
{
    return 1;
}

/* -------------------------------------------------------------------------- */
CommandArg::Type InfoCommand::getArgType(size_t pos) const
{
    switch (pos) {
        case 0:         return CommandArg::STRING;
        default:        return CommandArg::INVALID;
    }
}

/* -------------------------------------------------------------------------- */
string InfoCommand::getArgTitle(size_t pos) const
{
    switch (pos) {
        case 0:         return "firmware";
        default:        return "";
    }
}

/* -------------------------------------------------------------------------- */
StringVector InfoCommand::aliases() const
{
    StringVector ret;
    ret.push_back("firmware");
    return ret;
}

/* -------------------------------------------------------------------------- */
string InfoCommand::help() const
{
    return "Prints information about a specific firmware.";
}

/* -------------------------------------------------------------------------- */
void InfoCommand::printLongHelp(ostream &os) const
{
    os << "Name:            info\n"
       << "Aliases:         firmware\n\n"
       << "Description:\n"
       << "Displays information about a specific firmware. To obtain a list\n"
       << "of all available firmwares, use the \"list\" command. To get\n"
       << "information about pin assignment, use \"pin <firmware>\"."
       << endl;
}

/* PinCommand {{{1 */
PinCommand::PinCommand(Firmwarepool *firmwarepool)
    : AbstractCommand("pin"), m_firmwarepool(firmwarepool)
{}

/* -------------------------------------------------------------------------- */
bool PinCommand::execute(CommandArgVector args, ostream &os)
    throw (ApplicationError)
{
    string fwstr = args[0]->getString();
    Firmware *fw = m_firmwarepool->getFirmware(fwstr);
    if (!fw)
        throw ApplicationError(fwstr + ": Invalid firmware specified.");

    if (!Configuration::config()->getBatchMode()) {
        os << "            +----------------+" << endl;
        os << "            |  9  7  5  3  1 |" << endl;
        os << "            | 10  8  6  4  2 |" << endl;
        os << "            +----------------+" << endl;
        os << endl;
    }

    // calc max length
    size_t maxlen = 0;
    StringVector pins = fw->getPins();
    for (StringVector::const_iterator it = pins.begin();
            it != pins.end(); ++it)
        maxlen = max(maxlen, fw->getPin(*it).size());
    maxlen += 5;

    for (int i = 1; i <= 10; i += 2) {
        stringstream name1, name2;
        name1 << "P" << i;
        name2 << "P" << i+1;

        os << "[" << right << setw(5) << name1.str() << "] "
             << setw(maxlen) << left << fw->getPin(name1.str());
        os << "[" << right << setw(5) << name2.str() << "] "
             << left << fw->getPin(name2.str()) << endl;

        StringVector::iterator delit = find(pins.begin(), pins.end(), name1.str());
        if (delit != pins.end())
            pins.erase(delit);
        delit = find(pins.begin(), pins.end(), name2.str());
        if (delit != pins.end())
            pins.erase(delit);
    }

    for (int i = 0; i < pins.size(); i += 2) {
        os << "[" << right << setw(5) << pins[i] << "] "
             << setw(maxlen) << left << fw->getPin(pins[i]);

        if (i + 1 < pins.size())
            os << "[" << right << setw(5) << pins[i+1] << "] "
                << left << fw->getPin(pins[i+1]);
        os << endl;
    }

    return true;
}

/* -------------------------------------------------------------------------- */
size_t PinCommand::getArgNumber() const
{
    return 1;
}

/* -------------------------------------------------------------------------- */
CommandArg::Type PinCommand::getArgType(size_t pos) const
{
    switch (pos) {
        case 0:         return CommandArg::STRING;
        default:        return CommandArg::INVALID;
    }
}

/* -------------------------------------------------------------------------- */
string PinCommand::getArgTitle(size_t pos) const
{
    switch (pos) {
        case 0:         return "firmware";
        default:        return "";
    }
}

/* -------------------------------------------------------------------------- */
StringVector PinCommand::aliases() const
{
    StringVector ret;
    ret.push_back("pins");
    return ret;
}

/* -------------------------------------------------------------------------- */
string PinCommand::help() const
{
    return "Prints information about pin assignment.";
}

/* -------------------------------------------------------------------------- */
void PinCommand::printLongHelp(ostream &os) const
{
    os << "Name:            pin\n"
       << "Aliases:         pins\n"
       << "Argument:        firmware\n\n"
       << "Description:\n"
       << "Prints a list about pin usage. This might help you when connecting\n"
       << "something to your USBprog."
       << endl;
}

/* DownloadCommand {{{1 */

/* -------------------------------------------------------------------------- */
DownloadCommand::DownloadCommand(Firmwarepool *firmwarepool)
    : AbstractCommand("download"), m_firmwarepool(firmwarepool)
{}

/* -------------------------------------------------------------------------- */
bool DownloadCommand::downloadAll(ostream &os)
    throw (IOError)
{
    vector<Firmware *> firmwares = m_firmwarepool->getFirmwareList();

    for (vector<Firmware *>::const_iterator it = firmwares.begin();
            it != firmwares.end(); ++it) {
        try {
            if (m_firmwarepool->isFirmwareOnDisk((*it)->getName()))
                os << "Firmware " << (*it)->getLabel() << " is already there."
                   << endl;
            else {
                os << "Downloading " << (*it)->getLabel() << " ..." << endl;
                m_firmwarepool->downloadFirmware((*it)->getName());
            }
        } catch (const std::exception &ex) {
            os << "Error while downloading firmware " + (*it)->getName() +
                ": " + ex.what() << endl;
        }
    }

    return true;
}

/* -------------------------------------------------------------------------- */
bool DownloadCommand::execute(CommandArgVector args, ostream &os)
    throw (ApplicationError)
{
    string fwstr = args[0]->getString();
    if (Configuration::config()->isOffline()) {
        os << "Software is in offline mode. Downloading is not possbile."
           << endl;
        return true;
    }

    if (fwstr == "all")
        return downloadAll(os);

    Firmware *fw = m_firmwarepool->getFirmware(fwstr);
    if (!fw)
        throw ApplicationError(fwstr + ": Invalid firmware specified.");

    try {
        m_firmwarepool->downloadFirmware(fwstr);
        os << "Firmware " + fw->getName() + " has been downloaded successfully."
           << endl;
    } catch (const std::exception &ex) {
        os << "Error while downloading firmware: " << ex.what() << endl;
    }

    return true;
}

/* -------------------------------------------------------------------------- */
size_t DownloadCommand::getArgNumber() const
{
    return 1;
}

/* -------------------------------------------------------------------------- */
CommandArg::Type DownloadCommand::getArgType(size_t pos) const
{
    switch (pos) {
        case 0:         return CommandArg::STRING;
        default:        return CommandArg::INVALID;
    }
}

/* -------------------------------------------------------------------------- */
string DownloadCommand::getArgTitle(size_t pos) const
{
    switch (pos) {
        case 0:         return "firmware";
        default:        return "";
    }
}

/* -------------------------------------------------------------------------- */
StringVector DownloadCommand::aliases() const
{
    StringVector ret;
    ret.push_back("get");
    return ret;
}

/* -------------------------------------------------------------------------- */
string DownloadCommand::help() const
{
    return "Downloads a firmware file.";
}

/* -------------------------------------------------------------------------- */
void DownloadCommand::printLongHelp(ostream &os) const
{
    os << "Name:            download\n"
       << "Argument:        firmware\n\n"
       << "Description:\n"
       << "Downloads the specified firmware from the internet. Only available\n"
       << "when USBprog is not in offline mode. Use \"download all\" to download\n"
       << "all available firmware files."
       << endl;
}

/* CacheCommand {{{1 */

/* -------------------------------------------------------------------------- */
CacheCommand::CacheCommand(Firmwarepool *firmwarepool)
    : AbstractCommand("cache"), m_firmwarepool(firmwarepool)
{}

/* -------------------------------------------------------------------------- */
bool CacheCommand::execute(CommandArgVector args, ostream &os)
    throw (ApplicationError)
{
    string cmd = args[0]->getString();

    try {
        if (cmd == "clean")
            m_firmwarepool->cleanCache();
        else if (cmd == "delete")
            m_firmwarepool->deleteCache();
        else
            throw ApplicationError(cmd + ": Invalid command for \"cache\".");
    } catch (const IOError &ioe) {
        throw ApplicationError(string("I/O error: ") + ioe.what());
    }

    return true;
}

/* -------------------------------------------------------------------------- */
size_t CacheCommand::getArgNumber() const
{
    return 1;
}

/* -------------------------------------------------------------------------- */
CommandArg::Type CacheCommand::getArgType(size_t pos) const
{
    switch (pos) {
        case 0:         return CommandArg::STRING;
        default:        return CommandArg::INVALID;
    }
}

/* -------------------------------------------------------------------------- */
string CacheCommand::getArgTitle(size_t pos) const
{
    switch (pos) {
        case 0:         return "operation [clean/delete]";
        default:        return "";
    }
}

/* -------------------------------------------------------------------------- */
string CacheCommand::help() const
{
    return "Performs operation on the cache.";
}

/* -------------------------------------------------------------------------- */
void CacheCommand::printLongHelp(ostream &os) const
{
    os << "Name:            cache\n"
       << "Argument:        operation (clean/delete)\n\n"
       << "Description:\n"
       << "The \"delete\" operation deletes the whole cache. All firmware files\n"
       << "have to be downloaded again. The \"clean\" operation only deletes\n"
       << "obsolete firmware files, i.e. firmware data for which a newer version\n"
       << "is available."
       << endl;
}

/* DevicesCommand {{{1 */

/* -------------------------------------------------------------------------- */
DevicesCommand::DevicesCommand(DeviceManager *devicemanager,
        Firmwarepool *firmwarepool)
    : AbstractCommand("devices"), m_devicemanager(devicemanager),
      m_firmwarepool(firmwarepool)
{}

/* -------------------------------------------------------------------------- */
bool DevicesCommand::execute(CommandArgVector args, ostream &os)
    throw (ApplicationError)
{
    if (m_devicemanager->getNumberUpdateDevices() == 0)
        m_devicemanager->discoverUpdateDevices(m_firmwarepool);

    if (m_devicemanager->getNumberUpdateDevices() == 0)
        os << "No devices found." << endl;
    else
        m_devicemanager->printDevices(os);

    if (!Configuration::config()->getBatchMode() &&
            m_devicemanager->getNumberUpdateDevices() > 1)
        os << endl
           << "       * = Currently selected update device." << endl;

    return true;
}

/* -------------------------------------------------------------------------- */
string DevicesCommand::help() const
{
    return "Lists all update devices.";
}

/* -------------------------------------------------------------------------- */
void DevicesCommand::printLongHelp(ostream &os) const
{
    os << "Name:            devices\n\n"
       << "Description:\n"
       << "Lists all available update devices."
       << endl;
}


/* DeviceCommand {{{1 */

/* -------------------------------------------------------------------------- */
DeviceCommand::DeviceCommand(DeviceManager *devicemanager,
                             Firmwarepool *firmwarepool)
    : AbstractCommand("device"), m_devicemanager(devicemanager),
      m_firmwarepool(firmwarepool)
{}

/* -------------------------------------------------------------------------- */
bool DeviceCommand::execute(CommandArgVector args, ostream &os)
    throw (ApplicationError)
{
    string device = args[0]->getString();

    if (m_devicemanager->getNumberUpdateDevices() == 0)
        m_devicemanager->discoverUpdateDevices(m_firmwarepool);

    bool is_number = true;
    for (int i = 0; i < device.size(); i++) {
        if (!isdigit(device[i])) {
            is_number = false;
            break;
        }
    }

    int updatedevice = -1;

    if (is_number) {
        stringstream ss;
        ss << device;
        ss >> updatedevice;

        if (updatedevice < 0 || updatedevice >= m_devicemanager->getNumberUpdateDevices())
            throw ApplicationError("Invalid device number specified.");
    } else {

        for (int i = 0; i < m_devicemanager->getNumberUpdateDevices(); i++) {
            Device *dev = m_devicemanager->getDevice(i);

            if (dev->getShortName() == device) {
                updatedevice = i;
                break;
            }
        }

        if (updatedevice == -1)
            throw ApplicationError("Invalid update device name specified.");
    }

    m_devicemanager->setCurrentUpdateDevice(updatedevice);

    return true;
}

/* -------------------------------------------------------------------------- */
size_t DeviceCommand::getArgNumber() const
{
    return 1;
}

/* -------------------------------------------------------------------------- */
CommandArg::Type DeviceCommand::getArgType(size_t pos) const
{
    switch (pos) {
        case 0:         return CommandArg::STRING;
        default:        return CommandArg::INVALID;
    }
}

/* -------------------------------------------------------------------------- */
string DeviceCommand::getArgTitle(size_t pos) const
{
    switch (pos) {
        case 0:         return "device";
        default:        return "";
    }
}

/* -------------------------------------------------------------------------- */
string DeviceCommand::help() const
{
    return "Sets the update device.";
}

/* -------------------------------------------------------------------------- */
void DeviceCommand::printLongHelp(ostream &os) const
{
    os << "Name:            cache\n"
       << "Argument:        device number|device name\n\n"
       << "Description:\n"
       << "Sets the update device for the \"upload\" command. You have to use\n"
       << "an integer number which you can obtain with the \"devices\" command.\n"
       << "Alternatively, you can also use the short device name in the 2nd line\n"
       << "of the output of the \"devices\" command\n"
       << endl;
}

/* UploadCommand {{{1 */

/* -------------------------------------------------------------------------- */
UploadCommand::UploadCommand(DeviceManager *devicemanager,
                             Firmwarepool  *firmwarepool)
    : AbstractCommand("upload"), m_devicemanager(devicemanager),
      m_firmwarepool(firmwarepool)
{}

/* -------------------------------------------------------------------------- */
bool UploadCommand::execute(CommandArgVector args, ostream &os)
    throw (ApplicationError)
{
    string firmware = args[0]->getString();
    HashNotifier hn(DEFAULT_TERMINAL_WIDTH);

    if (m_devicemanager->getNumberUpdateDevices() == 0)
        m_devicemanager->discoverUpdateDevices();

    ByteVector data;

    if (Fileutil::isPathName(firmware)) {
        /* read from file */

        firmware = Fileutil::resolvePath(firmware);
        ifstream fin(firmware.c_str(), ios::binary);
        if (!fin)
            throw ApplicationError("Firmware file invalid");

        char buffer[BUFFERSIZE];
        while (!fin.eof()) {
            fin.read(buffer, BUFFERSIZE);
            if (fin.bad()) {
                fin.close();
                throw ApplicationError("Error while reading data from file.");
            }

            copy(buffer, buffer + fin.gcount(), back_inserter(data));
        }

        fin.close();
    } else {
        /* use pool */

        Firmware *fw = m_firmwarepool->getFirmware(firmware);
        if (!fw)
            throw ApplicationError(firmware+": Invalid firmware specified.");

        try {
            m_firmwarepool->fillFirmware(firmware);
        } catch (const IOError &err) {
            throw ApplicationError(string("I/O Error: ") + err.what());
        } catch (const GeneralError &err) {
            throw ApplicationError(string("General Error: ") + err.what());
        }

        data = fw->getData();
    }

    Device *dev = m_devicemanager->getUpdateDevice();
    if (!dev)
        throw ApplicationError("Unable to find update device.");

    // switch in update mode
    if (!dev->isUpdateMode()) {
        try {
            os << "Switching to update mode ..." << endl;
            m_devicemanager->switchUpdateMode();
        } catch (const IOError &err) {
            throw ApplicationError(string("I/O Error: ") + err.what());
        }
    }

    dev = m_devicemanager->getUpdateDevice();
    if (!dev)
        throw ApplicationError("Unable to find update device (2).");
    UsbprogUpdater updater(dev);

    if (!Configuration::config()->getBatchMode() &&
            !Configuration::config()->getDebug())
        updater.setProgress(&hn);

    try {
        os << "Opening device ..." << endl;
        updater.updateOpen();
        os << "Writing firmware ..." << endl;
        updater.writeFirmware(data);
        os << "Starting device ..." << endl;
        updater.startDevice();
        updater.updateClose();
    } catch (const IOError &err) {
        throw ApplicationError(string("I/O Error: ") + err.what());
    }

    os << "Detecting new USB devices ..." << endl;
    usbprog_sleep(2);
    m_devicemanager->discoverUpdateDevices();

    return true;
}

/* -------------------------------------------------------------------------- */
size_t UploadCommand::getArgNumber() const
{
    return 1;
}

/* -------------------------------------------------------------------------- */
CommandArg::Type UploadCommand::getArgType(size_t pos) const
{
    switch (pos) {
        case 0:         return CommandArg::STRING;
        default:        return CommandArg::INVALID;
    }
}

/* -------------------------------------------------------------------------- */
string UploadCommand::getArgTitle(size_t pos) const
{
    switch (pos) {
        case 0:         return "firmware";
        default:        return "";
    }
}

/* -------------------------------------------------------------------------- */
string UploadCommand::help() const
{
    return "Uploads a new firmware.";
}

/* -------------------------------------------------------------------------- */
void UploadCommand::printLongHelp(ostream &os) const
{
    os << "Name:            upload\n"
       << "Argument:        firmware|filename\n\n"
       << "Description:\n"
       << "Uploads a new firmware. The firmware identifier can be found with\n"
       << "the \"list\" command. Alternatively, you can just specify a filename.\n"
       << "If you have more than one USBprog device connected, use the \"devices\"\n"
       << "command to obtain a list of available update devices and select one\n"
       << "with the \"device\" command."
       << endl;
}

/* CopyingCommand {{{1 */

/* -------------------------------------------------------------------------- */
CopyingCommand::CopyingCommand()
    : AbstractCommand("copying")
{}

/* -------------------------------------------------------------------------- */
bool CopyingCommand::execute(CommandArgVector args, ostream &os)
    throw (ApplicationError)
{
    os << "USBprog " << USBPROG_VERSION_STRING << endl;
    os << "Copyright (c) 2007 Bernhard Walle <bernhard.walle@gmx.de>\n\n";
    os << "This program is free software: you can redistribute it and/or modify\n"
       << "it under the terms of the GNU General Public License as published by\n"
       << "the Free Software Foundation, either version 2 of the License, or\n"
       << "(at your option) any later version.\n\n"
       << "This program is distributed in the hope that it will be useful,\n"
       << "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
       << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
       << "GNU General Public License for more details.\n\n"
       << "You should have received a copy of the GNU General Public License\n"
       << "along with this program. If not, see <http://www.gnu.org/licenses/>.\n";
}

/* -------------------------------------------------------------------------- */
string CopyingCommand::help() const
{
    return "Displays the copyright";
}

/* -------------------------------------------------------------------------- */
StringVector CopyingCommand::aliases() const
{
    StringVector ret;
    ret.push_back("license");
    return ret;
}

/* -------------------------------------------------------------------------- */
void CopyingCommand::printLongHelp(ostream &os) const
{
    os << "Name:            copying\n"
       << "Aliases:         license\n\n"
       << "Description:\n"
       << "Shows the license of the program."
       << endl;
}

// vim: set sw=4 ts=4 fdm=marker et:
