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
#ifndef USBPROG_UTIL_H
#define USBPROG_UTIL_H

#include <usbprog/date.h>

class Fileutil {
    public:
        static std::string homeDir();
        static std::string configDir(const std::string &program);
        static bool mkdir(const std::string &dir);
        static bool isDir(const std::string &dir);
        static bool isFile(const std::string &dir);
        static DateTime getMTime(const std::string &file)
            throw (IOError);
};

std::string pathconcat(const std::string &a, const std::string &b);
std::string strip(std::string a);
unsigned long parse_long(const char *string);
std::string wordwrap(const std::string &text, size_t margins);
unsigned void usbprog_sleep(unsigned int seconds);
unsigned void usbprog_msleep(unsigned int msec);

#endif /* USBPROG_UTIL_H */

// vim: set sw=4 ts=4 et:
