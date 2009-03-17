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
#ifndef USBPROG_USBPROG_H
#define USBPROG_USBPROG_H

#include <vector>
#include <map>
#include <stdexcept>
#include <cstdarg>

/* Preprocessor definitions {{{ */

/* also update configure.in and win32/installer.nsi */
#define USBPROG_VERSION_STRING "0.1.9"

#define DEFAULT_INDEX_URL       "http://www.ixbat.de/usbprog/versions.xml"
#define AUTO_NOT_UPDATE_TIME    10

/* }}} */
/* Typedefs {{{ */

struct Device;
typedef std::vector<unsigned char> ByteVector;
typedef std::map<std::string, std::string> StringStringMap;
typedef std::vector<Device *> DeviceVector;
typedef std::vector<std::string> StringVector;

/* }}} */
/* Exceptions {{{1 */

class IOError : public std::runtime_error {
    public:
        IOError(const std::string& string)
            : std::runtime_error(string) {}
};

class ParseError : public std::runtime_error {
    public:
        ParseError(const std::string& string)
            : std::runtime_error(string) {}
};

class GeneralError : public std::runtime_error {
    public:
        GeneralError(const std::string& string)
            : std::runtime_error(string) {}
};

/* }}} */
/* ProgressNotifier {{{ */

class ProgressNotifier {
    public:
        virtual ~ProgressNotifier() {}

    public:
        virtual int progressed(double total, double now) = 0;
        virtual void finished() = 0;
};

/* }}} */
/* interface for handling messages {{{ */

enum MessageType {
    MT_STATUS
};

class OutputHandler {
    public:
        virtual ~OutputHandler() {}

    public:
        virtual void message(MessageType type, const std::string &message) = 0;
};

/* }}} */
/* Debugging {{{ */

class Debug {
    public:
        enum Level {
            DL_TRACE    = 0,
            DL_DEBUG    = 10,
            DL_INFO     = 20,
            DL_NONE     = 100
        };

    public:
        static Debug *debug();

        void dbg(const char *msg, ...);
        void info(const char *msg, ...);
        void trace(const char *msg, ...);
        void msg(Debug::Level level, const char *msg, ...);
        void vmsg(Debug::Level level, const char *msg, std::va_list args);

        void setLevel(Debug::Level level);
        Debug::Level getLevel() const;
        bool isDebugEnabled() const;
        void setFileHandle(FILE *handle);
        FILE *getFileHandle() const;

    protected:
        Debug();

    private:
        static Debug *m_instance;

    private:
        Level m_debuglevel;
        FILE *m_handle;
};

/* }}} */

#endif /* USBPROG_USBPROG_H */

// vim: set sw=4 ts=4 fdm=marker et: :collapseFolds=1:
