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
#ifndef USBPROG_DATE_H
#define USBPROG_DATE_H

#include <ctime>
#include <stdexcept>
#include <string>

#include <usbprog/usbprog.h>

enum DateTimeFormat {
    DTF_ISO_DATETIME,           /* YYYY-MM-DD HH:MM */
    DTF_ISO_DATE,               /* YYYY-MM-DD */
    DTF_ISO_SHORT_DATETIME,     /* YY-MM-DD HH:MM */
    DTF_ISO_SHORT_DATE          /* YY-MM-DD */
};

class DateTime {

    public:
        DateTime();
        DateTime(time_t time);
        DateTime(struct tm *time);
        DateTime(const std::string &string, DateTimeFormat format)
            throw (ParseError);

    public:
        void setDateTime(time_t time);
        void setDateTime(struct tm *time);
        void setDateTime(const std::string &string, DateTimeFormat format)
            throw (ParseError);

        time_t getDateTimeSeconds() const;
        struct tm getDateTimeTm() const;
        std::string getDateTimeString(DateTimeFormat format) const;

    private:
        time_t      m_dateTime;
};

bool operator==(const DateTime &a, const DateTime &b);
bool operator!=(const DateTime &a, const DateTime &b);
bool operator<=(const DateTime &a, const DateTime &b);
bool operator<(const DateTime &a, const DateTime &b);
bool operator>(const DateTime &a, const DateTime &b);
bool operator>=(const DateTime &a, const DateTime &b);
long long operator-(const DateTime &a, const DateTime &b);
long long operator+(const DateTime &a, const DateTime &b);


#endif /* USBPROG_DATE_H */

// vim: set sw=4 ts=4 et:
