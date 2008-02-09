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
#ifndef DEVICES_H
#define DEVICES_H

#include <stdint.h>
#include <string>
#include <vector>
#include <iostream>

#include <usbprog/usbprog.h>
#include <usbprog/firmwarepool.h>

/* Forward declarations {{{1 */

struct usb_dev_handle;
struct usb_device;


/* Device {{{1 */

class Device {
    public:
        Device(struct usb_device *handle);
        virtual ~Device() {}

    public:
        uint16_t getVendor() const;
        uint16_t getProduct() const;
        std::string getDevice() const;
        std::string getBus() const;
        std::string toString() const;
        std::string toShortString() const;

        bool isUpdateMode() const;
        void setUpdateMode(bool updateMode);

        void setName(const std::string &name);
        std::string getName() const;

        void setShortName(const std::string &shortName);
        std::string getShortName() const;

        struct usb_device *getHandle() const;

    private:
        struct usb_device *m_handle;
        bool m_updateMode;
        std::string m_name;
        std::string m_shortName;
};

bool operator==(const Device &a, const Device &b);
bool operator!=(const Device &a, const Device &b);
bool operator==(const DeviceVector &a, const DeviceVector &b);

/* DeviceManager {{{1 */

class DeviceManager {
    public:
        DeviceManager();
        DeviceManager(int debuglevel);
        virtual ~DeviceManager();

    public:
        void init(int debuglevel = 0);
        void setUsbDebugging(int debuglevel);
        void discoverUpdateDevices(Firmwarepool *firmwarepool = NULL);
        void printDevices(std::ostream &os) const;
        void switchUpdateMode()
            throw (IOError);

        size_t getNumberUpdateDevices() const;
        Device *getDevice(size_t number) const;
        Device *getUpdateDevice() const;
        void setCurrentUpdateDevice(ssize_t number);


    private:
        DeviceVector m_updateDevices;
        ssize_t m_currentUpdateDevice;
};

/* UsbprogUpdater {{{1 */

class UsbprogUpdater {
    public:
        UsbprogUpdater(Device *dev);
        virtual ~UsbprogUpdater();

    public:
        void setProgress(ProgressNotifier *notifier);
        void updateOpen()
            throw (IOError);
        void writeFirmware(const ByteVector &bv)
            throw (IOError);
        void startDevice()
            throw (IOError);
        void updateClose()
            throw (IOError);

    private:
        Device           *m_dev;
        ProgressNotifier *m_progressNotifier;
        usb_dev_handle   *m_devHandle;
};


#endif /* DEVICES_H */

// vim: set sw=4 ts=4 fdm=marker et:
