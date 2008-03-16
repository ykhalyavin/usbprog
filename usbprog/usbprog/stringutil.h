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
#ifndef USBPROG_STRINGUTIL_H
#define USBPROG_STRINGUTIL_H

#include <usbprog/usbprog.h>
#include <string>

/* StringTokenizer interface {{{1 */

class StringTokenizer {
    public:
        virtual ~StringTokenizer() {}

    public:
        virtual bool hasMoreTokens() const = 0;
        virtual std::string nextToken() = 0;

        virtual StringVector tokenize() = 0;
};

/* ShellStringTokenizer {{{1 */

class ShellStringTokenizer : public StringTokenizer {
    public:
        ShellStringTokenizer(const std::string &string);

    public:
        virtual bool hasMoreTokens() const;
        virtual std::string nextToken();

        virtual StringVector tokenize();

    private:
        std::string m_string;
        size_t m_pos;
};

/* global functions {{{1 */

std::string strip(std::string a);
unsigned long parse_long(const char *string);
std::string wordwrap(const std::string &text, ssize_t margins);
char **stringvector_to_array(const StringVector &vec);
bool str_starts_with(const std::string &string, const std::string &start);
std::vector<std::string> empty_element_sv();

#endif /* USBPROG_STRINGUTIL_H */

// vim: set sw=4 ts=4 fdm=marker et:
