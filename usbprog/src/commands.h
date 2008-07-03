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
#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <usbprog/firmwarepool.h>
#include <usbprog/usbprog.h>

#include "shell.h"

/* ListCommand {{{ */

class ListCommand : public AbstractCommand {
    public:
        ListCommand(Firmwarepool *firmwarepool);

    public:
        /* return false => end */
        bool execute(CommandArgVector args, StringVector options,
                std::ostream &os) throw (ApplicationError);

        StringVector aliases() const;

        std::string help() const;
        void printLongHelp(std::ostream &os) const;

    private:
        Firmwarepool *m_firmwarepool;
};

/* }}} */
/* InfoCommand {{{ */

class InfoCommand : public AbstractCommand {
    public:
        InfoCommand(Firmwarepool *firmwarepool);

    public:
        /* return false => end */
        bool execute(CommandArgVector args, StringVector options,
                std::ostream &os) throw (ApplicationError);

        size_t getArgNumber() const;
        CommandArg::Type getArgType(size_t pos) const;
        std::string getArgTitle(size_t pos) const;

        StringVector aliases() const;
        std::vector<std::string> getCompletions(
            const std::string &start, size_t pos, bool option,
            bool *filecompletion) const;

        std::string help() const;
        void printLongHelp(std::ostream &os) const;

    private:
        Firmwarepool *m_firmwarepool;
};

/* }}} */
/* PinCommand {{{ */

class PinCommand : public AbstractCommand {
    public:
        PinCommand(Firmwarepool *firmwarepool);

    public:
        /* return false => end */
        bool execute(CommandArgVector args, StringVector options,
                std::ostream &os) throw (ApplicationError);

        size_t getArgNumber() const;
        CommandArg::Type getArgType(size_t pos) const;
        std::string getArgTitle(size_t pos) const;

        StringVector aliases() const;

        std::string help() const;
        void printLongHelp(std::ostream &os) const;

        std::vector<std::string> getCompletions(
            const std::string &start, size_t pos, bool option,
            bool *filecompletion) const;

    private:
        Firmwarepool *m_firmwarepool;
};

/* }}} */
/* DownloadCommand {{{ */

class DownloadCommand : public AbstractCommand {
    public:
        DownloadCommand(Firmwarepool *firmwarepool);

    public:
        /* return false => end */
        bool execute(CommandArgVector args, StringVector options,
                std::ostream &os) throw (ApplicationError);

        size_t getArgNumber() const;
        CommandArg::Type getArgType(size_t pos) const;
        std::string getArgTitle(size_t pos) const;

        StringVector aliases() const;

        std::string help() const;
        void printLongHelp(std::ostream &os) const;

        std::vector<std::string> getCompletions(
            const std::string &start, size_t pos, bool option,
            bool *filecompletion) const;

    protected:
        bool downloadAll(std::ostream &os)
            throw (IOError);

    private:
        Firmwarepool *m_firmwarepool;
};

/* }}} */
/* CacheCommand {{{ */

class CacheCommand : public AbstractCommand {
    public:
        CacheCommand(Firmwarepool *firmwarepool);

    public:
        /* return false => end */
        bool execute(CommandArgVector args, StringVector options,
                std::ostream &os) throw (ApplicationError);

        size_t getArgNumber() const;
        CommandArg::Type getArgType(size_t pos) const;
        std::string getArgTitle(size_t pos) const;

        std::string help() const;
        void printLongHelp(std::ostream &os) const;

        std::vector<std::string> getCompletions(
            const std::string &start, size_t pos, bool option,
            bool *filecompletion) const;

    private:
        Firmwarepool *m_firmwarepool;
};

/* }}} */
/* DevicesCommand {{{ */

class DevicesCommand : public AbstractCommand {
    public:
        DevicesCommand(DeviceManager *devicemanager,
                Firmwarepool *firmwarepool);

    public:
        /* return false => end */
        bool execute(CommandArgVector args, StringVector options,
                std::ostream &os) throw (ApplicationError);

        std::string help() const;
        void printLongHelp(std::ostream &os) const;

    private:
        DeviceManager *m_devicemanager;
        Firmwarepool  *m_firmwarepool;
};

/* }}} */
/* DeviceCommand {{{ */

class DeviceCommand : public AbstractCommand {
    public:
        DeviceCommand(DeviceManager *devicemanager,
                Firmwarepool *firmwarepool);

    public:
        bool execute(CommandArgVector args, StringVector options,
                std::ostream &os) throw (ApplicationError);

        size_t getArgNumber() const;
        CommandArg::Type getArgType(size_t pos) const;
        std::string getArgTitle(size_t pos) const;

        std::string help() const;
        void printLongHelp(std::ostream &os) const;

        std::vector<std::string> getCompletions(
            const std::string &start, size_t pos, bool option,
            bool *filecompletion) const;

    private:
        DeviceManager *m_devicemanager;
        Firmwarepool *m_firmwarepool;
};

/* }}} */
/* UploadCommand {{{ */

class UploadCommand : public AbstractCommand {
    public:
        UploadCommand(DeviceManager *devicemanager,
                      Firmwarepool  *firmwarepool);

    public:
        bool execute(CommandArgVector args, StringVector options,
                std::ostream &os) throw (ApplicationError);

        size_t getArgNumber() const;
        CommandArg::Type getArgType(size_t pos) const;
        std::string getArgTitle(size_t pos) const;

        StringVector getSupportedOptions() const;

        std::string help() const;
        void printLongHelp(std::ostream &os) const;

        std::vector<std::string> getCompletions(
            const std::string &start, size_t pos, bool option,
            bool *filecompletion) const;

    private:
        DeviceManager *m_devicemanager;
        Firmwarepool  *m_firmwarepool;
};

/* }}} */
/* StartCommand {{{ */

class StartCommand : public AbstractCommand {
    public:
        StartCommand(DeviceManager *devicemanager);

    public:
        bool execute(CommandArgVector args, StringVector options,
                std::ostream &os) throw (ApplicationError);

        std::string help() const;
        void printLongHelp(std::ostream &os) const;

    private:
        DeviceManager *m_devicemanager;
};

/* }}} */
/* CopyingCommand {{{ */

class CopyingCommand : public AbstractCommand {
    public:
        CopyingCommand();

    public:
        bool execute(CommandArgVector args, StringVector,
                std::ostream &os) throw (ApplicationError);

        StringVector aliases() const;
        std::string help() const;
        void printLongHelp(std::ostream &os) const;
};

/* }}} */

#endif /* COMMANDS_H */

// vim: set sw=4 ts=4 fdm=marker et: :collapseFolds=1:
