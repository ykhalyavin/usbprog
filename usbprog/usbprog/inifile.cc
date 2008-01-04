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
#include <iostream>
#include <fstream>

#include <usbprog/inifile.h>

using std::string;
using std::map;
using std::pair;
using std::istringstream;
using std::ifstream;

/* -------------------------------------------------------------------------- */
IniFile::IniFile(const std::string &fileName)
    : m_fileName(fileName)
{}

/* -------------------------------------------------------------------------- */
void IniFile::readFile() throw (IOError)
{
    ifstream file(m_fileName.c_str());

    if (!file)
        throw IOError("Cannot open the specified ini file " + m_fileName + ".");

    string line;

    while ( getline(file, line) ) {
        size_t equal = line.find("="); 
        if (line.find("#") == 0 || equal == string::npos)
            continue;

        m_map[line.substr(0, equal)] = line.substr(equal+1, line.length());
    }
}


/* -------------------------------------------------------------------------- */
string IniFile::getValue(const string &key) const
{
    map<string, string>::const_iterator result = m_map.find(key);

    if (result == m_map.end())
        return string();
    else
        return (*result).second;
}


/* -------------------------------------------------------------------------- */
int IniFile::getIntValue(const string &key) const
{
    map<string, string>::const_iterator result = m_map.find(key);

    if (result == m_map.end())
        return 0;
    else {
        int resultNumber;

        istringstream inStream((*result).second);
        inStream >> resultNumber;

        return resultNumber;
    }
}


/* -------------------------------------------------------------------------- */
bool IniFile::isKeyAvailable(const string &key) const
{
    return m_map.find(key) != m_map.end();
}


// vim: set sw=4 ts=4 et:
