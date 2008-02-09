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
#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <stdexcept>
#include <usbprog/usbprog.h>

#include "io.h"
#include "usbprog.h"

/* Typedefs {{{1 */

class Command;
class CommandArg;
typedef std::map<std::string, Command *> StringCommandMap;
typedef std::vector<CommandArg *> CommandArgVector;


/* CommandArg {{{1 */

class CommandArg {
    public:
        enum Type {
            INVALID,
            INTEGER,
            UINTEGER,
            FLOAT,
            STRING
        };

    public:
        Type getType() const;

        std::string getString() const
            throw (std::runtime_error);
        long long getInteger() const
            throw (std::runtime_error);
        unsigned long long getUInteger() const
            throw (std::runtime_error);
        double getFloat() const
            throw (std::runtime_error);

        void setString(const std::string &str);
        void setInteger(long long integer);
        void setUInteger(unsigned long long integer);
        void setFloat(double value);

    public:
        static CommandArg *fromString(const std::string &str, Type type);

    private:
        union {
            unsigned long long ull;
            long long ll;
            double d;
        } m_int;
        std::string m_string;
        Type m_type;
};

/* Interface for commands {{{1 */

class Command {
    public:
        virtual ~Command() {}

    public:
        /* return false => end */
        virtual bool execute(CommandArgVector args, StringVector options,
                std::ostream &os) throw (ApplicationError) = 0;

        virtual size_t getArgNumber() const = 0;
        virtual CommandArg::Type getArgType(size_t pos) const = 0;
        virtual std::string getArgTitle(size_t pos) const = 0;

        virtual StringVector getSupportedOptions() const = 0;

        virtual std::string name() const = 0;
        virtual StringVector aliases() const = 0;

        virtual std::string help() const = 0;
        virtual void printLongHelp(std::ostream &os) const = 0;
};

/* AbstractCommand {{{1 */

class AbstractCommand : public Command {
    public:
        AbstractCommand(const std::string &name);

    public:
        size_t getArgNumber() const;
        CommandArg::Type getArgType(size_t pos) const;
        std::string getArgTitle(size_t pos) const;
        std::string name() const;
        StringVector aliases() const;
        StringVector getSupportedOptions() const;

    private:
        std::string m_name;
};

/* The shell itself {{{1 */

class Shell {
    friend class HelpCommand;
    friend class HelpCmdCommand;

    public:
        Shell(const std::string &prompt);
        virtual ~Shell();

    public:
        void addCommand(Command *cmd);
        void run();
        bool run(StringVector input, bool multiple = true)
            throw (ApplicationError);

    private:
        StringCommandMap m_commands;
        LineReader *m_lineReader;
};

/* Exit command {{{1 */

class ExitCommand : public AbstractCommand {
    public:
        ExitCommand();

    public:
        bool execute(CommandArgVector args, StringVector vector,
                std::ostream &os) throw (ApplicationError);

        StringVector aliases() const;

        std::string help() const;
        void printLongHelp(std::ostream &os) const;
};

/* Help command {{{1 */

class HelpCommand : public AbstractCommand {
    public:
        HelpCommand(Shell *sh);

    public:
        bool execute(CommandArgVector args, StringVector sv,
                std::ostream &os) throw (ApplicationError);

        std::string help() const;
        void printLongHelp(std::ostream &os) const;

    private:
        Shell *m_sh;
};

/* HelpCmd command {{{1 */

class HelpCmdCommand : public AbstractCommand {
    public:
        HelpCmdCommand(Shell *sh);

    public:
        bool execute(CommandArgVector args, StringVector sv,
                std::ostream &os) throw (ApplicationError);
        size_t getArgNumber() const;
        CommandArg::Type getArgType(size_t pos) const;
        std::string getArgTitle(size_t pos) const;

        virtual StringVector aliases() const;

        std::string help() const;
        void printLongHelp(std::ostream &os) const;

    private:
        Shell *m_sh;
};

#endif /* SHELL_H */

// vim: set sw=4 ts=4 fdm=marker et:
