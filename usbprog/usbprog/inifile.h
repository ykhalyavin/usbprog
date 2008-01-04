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
#ifndef USBPROG_INIFILE_H
#define USBPROG_INIFILE_H

#include <string>
#include <map>

#include <usbprog/usbprog.h>

class IniFile {
    public:
        IniFile(const std::string &fileName);
        virtual ~IniFile() {}

        void readFile() throw (IOError);

        std::string getValue(const std::string &key) const;
        int getIntValue(const std::string &key) const;
        bool isKeyAvailable(const std::string &key) const;

    private:
        std::string m_fileName;
        std::map<std::string, std::string> m_map;
};


#endif /* USBPROG_INIFILE_H */

// vim: set sw=4 ts=4 et:
