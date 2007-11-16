/*
 * (c) 2007, Bernhard Walle <bernhard.walle@gmx.de>
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
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <vector>

#include <unistd.h>
#include <usb.h>
#include <usbprog/devices.h>

using std::vector;
using std::string;
using std::ostream;
using std::ios;
using std::setw;
using std::endl;
using std::stringstream;
using std::setfill;
using std::min;
using std::right;
using std::left;
using std::setw;
using std::copy;

#define VENDOR_ID_USBPROG       0x1781
#define PRODUCT_ID_USBPROG      0x0c62
#define BCDDEVICE_UPDATE        0x0000

/* Device {{{1 */

/* -------------------------------------------------------------------------- */
Device::Device(struct usb_device *handle)
    : m_handle(handle), m_updateMode(false)
{}

/* -------------------------------------------------------------------------- */
uint16_t Device::getVendor() const
{
    return m_handle->descriptor.idVendor;
}

/* -------------------------------------------------------------------------- */
uint16_t Device::getProduct() const
{
    return m_handle->descriptor.idProduct;
}

/* -------------------------------------------------------------------------- */
std::string Device::getDevice() const
{
    return string(m_handle->filename);
}

/* -------------------------------------------------------------------------- */
std::string Device::getBus() const
{
    return string(m_handle->bus->dirname);
}

/* -------------------------------------------------------------------------- */
bool Device::isUpdateMode() const
{
    return m_updateMode;
}

/* -------------------------------------------------------------------------- */
void Device::setUpdateMode(bool updateMode)
{
    m_updateMode = updateMode;
}

/* -------------------------------------------------------------------------- */
struct usb_device *Device::getHandle() const
{
    return m_handle;
}

/* -------------------------------------------------------------------------- */
void Device::setName(const string &name)
{
    m_name = name;
}

/* -------------------------------------------------------------------------- */
string Device::getName() const
{
    return m_name;
}

/* -------------------------------------------------------------------------- */
string Device::toString() const
{
    stringstream ss;

    ss << "Bus " << getBus() << " ";
    ss << "Device " << getDevice() << ": ";
    ss << setw(4) << std::hex << setfill('0') << getVendor();
    ss << setw(1) << ":";
    ss << setw(4) << std::hex << setfill('0') << getProduct();

    if (m_name.size() > 0)
        ss << " - " + m_name;

    return ss.str();
}


/* DeviceManager {{{1 */

/* -------------------------------------------------------------------------- */
DeviceManager::DeviceManager()
    : m_currentUpdateDevice(-1)
{
    usb_init();
}

/* -------------------------------------------------------------------------- */
void DeviceManager::setUsbDebugging(int debuglevel)
{
    usb_set_debug(debuglevel);
}

/* -------------------------------------------------------------------------- */
void DeviceManager::discoverUpdateDevices(Firmwarepool *firmwarepool)
{
    usb_find_busses();
    usb_find_devices();

    m_updateDevices.clear();

    vector<Firmware *> firmwares;
    if (firmwarepool)
        firmwares = firmwarepool->getFirmwareList();

    for (struct usb_bus *bus = usb_get_busses(); bus; bus = bus->next)
        for (struct usb_device *dev = bus->devices; dev; dev = dev->next) {
            uint16_t vendorid = dev->descriptor.idVendor;
            uint16_t productid = dev->descriptor.idProduct;
            uint16_t bcddevice = dev->descriptor.bcdDevice;
            if (vendorid == VENDOR_ID_USBPROG &&
                    productid == PRODUCT_ID_USBPROG &&
                    bcddevice == BCDDEVICE_UPDATE) {
                Device *d = new Device(dev);
                d->setUpdateMode(true);
                d->setName("USBprog in update mode");
                m_updateDevices.push_back(d);
            } else if (firmwarepool)
                for (vector<Firmware *>::const_iterator it = firmwares.begin();
                        it != firmwares.end(); ++it)
                    if (vendorid != 0 && productid != 0 &&
                            (*it)->getVendorId() == vendorid &&
                            (*it)->getProductId() == productid &&
                            (*it)->getBcdDevice() == bcddevice) {
                        Device *d = new Device(dev);
                        d->setName("USBprog with \"" + (*it)->getLabel() + 
                                "\" firmware");
                        m_updateDevices.push_back(d);
                    }
        }
}

/* -------------------------------------------------------------------------- */
void DeviceManager::printDevices(ostream &os) const
{
    int i = 0;
    Device *up = getUpdateDevice();
    for (DeviceVector::const_iterator it = m_updateDevices.begin();
            it != m_updateDevices.end(); ++it) {
        os << " [" << setw(2) << right << i++ << "] " << left;
        if (up != NULL && *up == **it)
            os << " *  ";
        else
            os << "    ";
        os << (*it)->toString() << endl;
    }
}

/* -------------------------------------------------------------------------- */
void DeviceManager::switchUpdateMode()
    throw (IOError)
{
    Device *dev = getUpdateDevice();
    if (dev->isUpdateMode())
        return;

    struct usb_dev_handle *usb_handle = usb_open(dev->getHandle());
    if (!usb_handle)
        throw IOError("Could not open USB device: " + string(usb_strerror()));

    usb_set_configuration(usb_handle, dev->getHandle()->config[0].bConfigurationValue);
    int usb_interface = dev->getHandle()->config[0].interface[0].
        altsetting[0].bInterfaceNumber;
    int ret = usb_claim_interface(usb_handle, usb_interface);
    if (ret < 0) {
        usb_close(usb_handle);
        throw IOError(usb_strerror());
    }

    /* needed ?*/
    usb_set_altinterface(usb_handle, 0);
    if (ret < 0) {
        usb_release_interface(usb_handle, usb_interface);
        usb_close(usb_handle);
        throw IOError("Error when setting altinterface to 0: " + string(usb_strerror()));
    }

    int timeout = 6;

    while (usb_control_msg(usb_handle, 0xC0, 0x01, 0, 0, NULL, 8, 1000) < 0){
        if (--timeout == 0)
            break;
        sleep(1);
    }

    usb_release_interface(usb_handle, usb_interface);
    usb_close(usb_handle);

    // set again the update device
    discoverUpdateDevices();

    // TODO: Verify that the m_currentUpdateDevice is still valid!
}

/* -------------------------------------------------------------------------- */
size_t DeviceManager::getNumberUpdateDevices() const
{
    return m_updateDevices.size();
}

/* -------------------------------------------------------------------------- */
Device *DeviceManager::getUpdateDevice() const
{
    Device *dev = NULL;

    if (m_currentUpdateDevice < -1 ||
            m_currentUpdateDevice >= (ssize_t)m_updateDevices.size())
        return NULL;

    if (m_currentUpdateDevice == (ssize_t)-1) {
        for (DeviceVector::const_iterator it = m_updateDevices.begin();
                it != m_updateDevices.end(); ++it)
            if ((*it)->isUpdateMode())
                return *it;

        return NULL;
    } else
        return m_updateDevices[m_currentUpdateDevice];
}

/* -------------------------------------------------------------------------- */
void DeviceManager::setCurrentUpdateDevice(ssize_t number)
{
    m_currentUpdateDevice = number;
}

/* -------------------------------------------------------------------------- */
bool operator==(const Device &a, const Device &b)
{
    return a.getBus() == b.getBus() &&
        a.getDevice() == b.getDevice() &&
        a.getProduct() == b.getProduct() &&
        a.getVendor() == b.getVendor();
}

/* -------------------------------------------------------------------------- */
bool operator!=(const Device &a, const Device &b)
{
    return a.getBus() != b.getBus() ||
        a.getDevice() != b.getDevice() ||
        a.getProduct() != b.getProduct() ||
        a.getVendor() != b.getVendor();
}

/* UsbprogUpdater {{{1 */

#define USB_PAGESIZE 64
#define WRITEPAGE      0x02
#define STARTAPP       0x01

/* -------------------------------------------------------------------------- */
UsbprogUpdater::UsbprogUpdater(Device *dev)
    : m_dev(dev), m_progressNotifier(NULL), m_devHandle(NULL)
{}

/* -------------------------------------------------------------------------- */
UsbprogUpdater::~UsbprogUpdater()
{
    if (m_devHandle)
        updateClose();
}

/* -------------------------------------------------------------------------- */
void UsbprogUpdater::setProgress(ProgressNotifier *progress)
{
    m_progressNotifier = progress;
}

/* -------------------------------------------------------------------------- */
void UsbprogUpdater::writeFirmware(const ByteVector &bv)
    throw (IOError)
{
    char buf[USB_PAGESIZE];
    char cmd[USB_PAGESIZE];
    int ret;

    if (!m_devHandle)
        throw IOError("Device not opened");

    int page = 0;
    memset(cmd, 0, USB_PAGESIZE);

    for (int i = 0; i < bv.size(); i += 64) {
        size_t sz = min(USB_PAGESIZE, int(bv.size()-i-1));
        memset(buf, 0, USB_PAGESIZE);
        copy(bv.begin() + i, bv.begin() + i + sz, buf);

        cmd[0] = WRITEPAGE;
        cmd[1] = (char)page++;

        ret = usb_bulk_write(m_devHandle,2,cmd,USB_PAGESIZE, 100);
        if (ret < 0) {
            updateClose();
            if (m_progressNotifier)
                m_progressNotifier->finished();
            throw IOError("Error while writing to USB device: "+
                    string(usb_strerror()));
        }

        // data message
        ret = usb_bulk_write(m_devHandle,2,buf,USB_PAGESIZE, 100);
        if (ret < 0) {
            updateClose();
            if (m_progressNotifier)
                m_progressNotifier->finished();
            throw IOError("Error while writing to USB device: "+
                    string(usb_strerror()));
        }

        if (m_progressNotifier)
            m_progressNotifier->progressed(bv.size(), i);
    }

    if (m_progressNotifier)
        m_progressNotifier->finished();
}

/* -------------------------------------------------------------------------- */
void UsbprogUpdater::updateOpen()
    throw (IOError)
{
    struct usb_device *dev = m_dev->getHandle();

    int ret;

    if (m_devHandle)
        throw IOError("Device still opened. Close first.");

    m_devHandle = usb_open(dev);
    if (!m_devHandle)
        throw IOError("usb_open failed " + string(usb_strerror()));

    usb_set_configuration(m_devHandle, dev->config[0].bConfigurationValue);

    int usb_interface = dev->config[0].interface[0].altsetting[0].bInterfaceNumber;
    ret = usb_claim_interface(m_devHandle, usb_interface);
    if (ret < 0) {
        updateClose();
        throw IOError("Claiming interface failed: " + string(usb_strerror()));
    }
}

/* -------------------------------------------------------------------------- */
void UsbprogUpdater::updateClose()
    throw (IOError)
{
    if (!m_devHandle)
        throw IOError("Device already closed");

    int usb_interface = m_dev->getHandle()->config[0].interface[0]
        .altsetting[0].bInterfaceNumber;
    usb_release_interface(m_devHandle, usb_interface);
    usb_close(m_devHandle);
    m_devHandle = NULL;
}

/* -------------------------------------------------------------------------- */
void UsbprogUpdater::startDevice()
    throw (IOError)
{
    char buf[USB_PAGESIZE];
    memset(buf, 0, USB_PAGESIZE);

    buf[0] = STARTAPP;
    int ret = usb_bulk_write(m_devHandle, 2, buf, USB_PAGESIZE, 100);
    if (ret < 0)
        throw IOError("Error in bulk write: " + string(usb_strerror()));
}

/* }}} */

// vim: set sw=4 ts=4 fdm=marker et:
