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
#define _XOPEN_SOURCE
#include <cstdio>
#include <string>
#include <time.h>
#include <cstring>

#include <usbprog/date.h>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

using std::mktime;
using std::time;
using std::string;
using std::memset;

static const char *formatstrings[] = {
    "%Y-%m-%d %H:%M",           /* DTF_ISO_DATETIME */
    "%Y-%m-%d",                 /* DTF_ISO_DATE */
    "%y-%m-%d %H:%M",           /* DTF_ISO_SHORT_DATETIME */
    "%y-%m-%d"                  /* case DTF_ISO_SHORT_DATE */
};

/* DateTime {{{ */

/* -------------------------------------------------------------------------- */
DateTime::DateTime()
{
    setDateTime(time(NULL));
}

/* -------------------------------------------------------------------------- */
DateTime::DateTime(time_t time)
    : m_dateTime(time)
{}

/* -------------------------------------------------------------------------- */
DateTime::DateTime(struct tm *time)
{
    setDateTime(time);
}

/* -------------------------------------------------------------------------- */
DateTime::DateTime(const string &string, DateTimeFormat format)
    throw (ParseError)
{
    setDateTime(string, format);
}

/* -------------------------------------------------------------------------- */
void DateTime::setDateTime(time_t time)
{
    m_dateTime = time;
}

/* -------------------------------------------------------------------------- */
void DateTime::setDateTime(struct tm *time)
{
    m_dateTime = mktime(time);
}

/* -------------------------------------------------------------------------- */
#ifdef HAVE_STRPTIMEx
void DateTime::setDateTime(const std::string &string, DateTimeFormat format)
    throw (ParseError)
{
    struct tm time;
    char      *ret;

    memset(&time, 0, sizeof(struct tm));

    ret = strptime(string.c_str(), formatstrings[format], &time);
    if (ret == NULL || *ret != 0)
        throw ParseError("Invalid string for the specified format");

    setDateTime(&time);
}
#else
void DateTime::setDateTime(const std::string &string, DateTimeFormat format)
    throw (ParseError)
{
    int day, month, year, hour = 0, minute = 0;
    int ret;

    switch (format) {
        case DTF_ISO_DATETIME:
        case DTF_ISO_SHORT_DATETIME:
            ret = sscanf(string.c_str(), "%d-%d-%d %d:%d",
                    &year, &month, &day, &minute, &hour);
            if (ret != 5)
                throw ParseError("Invalid string for the specified format");

            break;

        case DTF_ISO_DATE:
        case DTF_ISO_SHORT_DATE:
            ret = sscanf(string.c_str(), "%d-%d-%d", &year, &month, &day);
            if (ret != 3)
                throw ParseError("Invalid string for the specified format");

            break;
    }

    if (format == DTF_ISO_SHORT_DATE || format == DTF_ISO_SHORT_DATETIME)
        year += year < 30 ? 2000 : 1900;

    struct tm my_tm;
    memset(&my_tm, 0, sizeof(struct tm));

    my_tm.tm_min = minute;
    my_tm.tm_hour = hour;
    my_tm.tm_mday = day;
    my_tm.tm_mon = month - 1;
    my_tm.tm_year = year - 1900;

    setDateTime(&my_tm);
}
#endif

/* -------------------------------------------------------------------------- */
time_t DateTime::getDateTimeSeconds() const
{
    return m_dateTime;
}

/* -------------------------------------------------------------------------- */
struct tm DateTime::getDateTimeTm() const
{
    return *(localtime(&m_dateTime));
}

/* -------------------------------------------------------------------------- */
string DateTime::getDateTimeString(DateTimeFormat format) const
{
    char        buffer[1024];
    struct tm   time;

    time = getDateTimeTm();

    strftime(buffer, 1024, formatstrings[format], &time);

    return string(buffer);
}

/* }}} */
/* Operators {{{ */

/* -------------------------------------------------------------------------- */
bool operator==(const DateTime &a, const DateTime &b)
{
    return a.getDateTimeSeconds() == b.getDateTimeSeconds();
}

/* -------------------------------------------------------------------------- */
bool operator!=(const DateTime &a, const DateTime &b)
{
    return a.getDateTimeSeconds() != b.getDateTimeSeconds();
}

/* -------------------------------------------------------------------------- */
bool operator<=(const DateTime &a, const DateTime &b)
{
    return a.getDateTimeSeconds() <= b.getDateTimeSeconds();
}

/* -------------------------------------------------------------------------- */
bool operator<(const DateTime &a, const DateTime &b)
{
    return a.getDateTimeSeconds() < b.getDateTimeSeconds();
}

/* -------------------------------------------------------------------------- */
bool operator>(const DateTime &a, const DateTime &b)
{
    return a.getDateTimeSeconds() > b.getDateTimeSeconds();
}

/* -------------------------------------------------------------------------- */
bool operator>=(const DateTime &a, const DateTime &b)
{
    return a.getDateTimeSeconds() >= b.getDateTimeSeconds();
}

/* -------------------------------------------------------------------------- */
long long operator-(const DateTime &a, const DateTime &b)
{
    return a.getDateTimeSeconds() - b.getDateTimeSeconds();
}

/* -------------------------------------------------------------------------- */
long long operator+(const DateTime &a, const DateTime &b)
{
    return a.getDateTimeSeconds() - b.getDateTimeSeconds();
}

/* }}} */

// vim: set sw=4 ts=4 fdm=marker et: :collapseFolds=1:
