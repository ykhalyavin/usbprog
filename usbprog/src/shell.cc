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
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

#include <usbprog/usbprog.h>

#include "shell.h"
#include "io.h"
#include "configuration.h"

using std::ostream;
using std::string;
using std::runtime_error;
using std::stringstream;
using std::exception;
using std::cout;
using std::cerr;
using std::endl;
using std::setw;
using std::vector;

/* AbstractCommand {{{1 */

/* -------------------------------------------------------------------------- */
AbstractCommand::AbstractCommand(const string &name)
    : m_name(name)
{}

/* -------------------------------------------------------------------------- */
size_t AbstractCommand::getArgNumber() const
{
    return 0;
}

/* -------------------------------------------------------------------------- */
CommandArg::Type AbstractCommand::getArgType(size_t pos) const
{
    return CommandArg::INVALID;
}

/* -------------------------------------------------------------------------- */
string AbstractCommand::getArgTitle(size_t pos) const
{
    return "";
}

/* -------------------------------------------------------------------------- */
string AbstractCommand::name() const
{
    return m_name;
}

/* -------------------------------------------------------------------------- */
StringVector AbstractCommand::aliases() const
{
    return StringVector();
}

/* CommandArg ------------------------------------------------------ {{{1 --- */

/* -------------------------------------------------------------------------- */
CommandArg::Type CommandArg::getType() const
{
    return m_type;
}

/* -------------------------------------------------------------------------- */
string CommandArg::getString() const
    throw (runtime_error)
{
    if (m_type != STRING)
        throw runtime_error("Not a string");

    return m_string;
}

/* -------------------------------------------------------------------------- */
long long CommandArg::getInteger() const
    throw (runtime_error)
{
    if (m_type != INTEGER)
        throw runtime_error("Not an integer");

    return m_int.ll;
}

/* -------------------------------------------------------------------------- */
unsigned long long CommandArg::getUInteger() const
    throw (runtime_error)
{
    if (m_type != UINTEGER)
        throw runtime_error("Not an unsigned integer");

    return m_int.ull;
}

/* -------------------------------------------------------------------------- */
double CommandArg::getFloat() const
    throw (runtime_error)
{
    if (m_type != FLOAT)
        throw runtime_error("Not a float");

    return m_int.d;
}

/* -------------------------------------------------------------------------- */
void CommandArg::setString(const string &string)
{
    m_type = STRING;
    m_string = string;
}

/* -------------------------------------------------------------------------- */
void CommandArg::setInteger(long long integer)
{
    m_type = INTEGER;
    m_int.ll = integer;
}

/* -------------------------------------------------------------------------- */
void CommandArg::setUInteger(unsigned long long integer)
{
    m_type = UINTEGER;
    m_int.ull = integer;
}

/* -------------------------------------------------------------------------- */
void CommandArg::setFloat(double value)
{
    m_type = FLOAT;
    m_int.d = value;
}

/* -------------------------------------------------------------------------- */
CommandArg *CommandArg::fromString(const std::string &str, CommandArg::Type type)
{
    stringstream string;
    CommandArg *ret = new CommandArg;

    string << str;

    switch (type) {
        case STRING:
            ret->setString(string.str());
            break;

        case INTEGER: {
                long long val;
                string >> val;
                ret->setInteger(val);
                break;
          }

        case UINTEGER: {
                unsigned long long val;
                string >> val;
                ret->setUInteger(val);
                break;
            }

        case FLOAT: {
                double val;
                string >> val;
                ret->setFloat(val);
                break;
            }
    }

    return ret;
}

/* Shell ----------------------------------------------------------- {{{1 --- */

/* -------------------------------------------------------------------------- */
Shell::Shell(const string &prompt)
{
    m_lineReader = LineReader::defaultLineReader(prompt);
    try {
        m_lineReader->readHistory(Configuration::config()->getHistoryFile());
    } catch (const IOError &ioe)
    {}
    addCommand(new ExitCommand);
    addCommand(new HelpCommand(this));
    addCommand(new HelpCmdCommand(this));
}

/* -------------------------------------------------------------------------- */
Shell::~Shell()
{
    vector<Command *> deleteList;

    for (StringCommandMap::const_iterator it = m_commands.begin();
            it != m_commands.end(); ++it)
        if (it->second->name() == it->first)
            deleteList.push_back(it->second);

    for (vector<Command *>::const_iterator it = deleteList.begin();
            it != deleteList.end(); ++it)
        delete *it;

    try {
        m_lineReader->writeHistory(Configuration::config()->getHistoryFile());
    } catch (const IOError &ioe) {
        cerr << "Error when saving history: " << ioe.what() << endl;
    }
    delete m_lineReader;
}

/* -------------------------------------------------------------------------- */
void Shell::addCommand(Command *cmd)
{
    // if it's already in the map, remove it first and free the memory
    StringCommandMap::iterator it = m_commands.find(cmd->name());
    if (it != m_commands.end()) {
        delete it->second;
        m_commands.erase(it);
    }

    m_commands[cmd->name()] = cmd;

    StringVector aliases = cmd->aliases();
    for (StringVector::const_iterator it = aliases.begin();
            it != aliases.end(); ++it)
        m_commands[*it] = cmd;
}

/* -------------------------------------------------------------------------- */
void Shell::run()
{
    bool result = true;

    while (!m_lineReader->eof() && result) {
        stringstream strstr;
        strstr << m_lineReader->readLine();
        StringVector vec;
        string s;
        while (strstr >> s)
            vec.push_back(s);

        if (vec.size() == 0)
            continue;

        try {
            result = run(vec, false);
        } catch (const ApplicationError &err) {
            cout << err.what() << endl;
        }
    }

    // output a newline if the readline() library has encountered
    // an EOF
    if (m_lineReader->eof())
        cout << endl;
}

/* -------------------------------------------------------------------------- */
bool Shell::run(StringVector input, bool multiple)
    throw (ApplicationError)
{
    bool result = true;
    int loop = 0;

    if (input.size() == 0)
        throw ApplicationError("Input size == 0");

    do {
        string cmdstr = input[0];
        string execstr = cmdstr;
        input.erase(input.begin());
        StringCommandMap::const_iterator it = m_commands.find(cmdstr);
        if (it == m_commands.end())
            throw ApplicationError("Invalid command");
        Command *cmd = it->second;

        if (multiple && cmd->getArgNumber() > input.size())
            throw ApplicationError(cmdstr + ": Not enough arguments provided");
        if (!multiple && cmd->getArgNumber() < input.size())
            throw ApplicationError(cmdstr + ": Too much arguments provided.");

        CommandArgVector vec;
        for (int argNo = 0; argNo < cmd->getArgNumber(); argNo++) {
            string argstr;
            if (input.size() > 0) {
                argstr = input[0];
                input.erase(input.begin());
            } else {
                string prompt = cmd->getArgTitle(argNo) + "> ";
                argstr = m_lineReader->readLine(prompt.c_str());
            }

            execstr += " " + argstr;
            CommandArg *arg = CommandArg::fromString(argstr,
                    cmd->getArgType(argNo));
            vec.push_back(arg);
        }

        try {
            if (multiple && (input.size() > 0 || loop != 0))
                cout << "===> " << execstr << endl;
            loop++;
            result = cmd->execute(vec, cout);
            if (multiple && result && input.size() > 0)
                cout << endl;

        } catch (const ApplicationError &ex) {
            cout << ex.what() << endl;
        }

        // free memory
        for (CommandArgVector::const_iterator it = vec.begin();
                it != vec.end(); ++it)
            delete *it;

    } while (result && input.size() > 0 && multiple);

    return result;
}

/* ExitCommand {{{1 */

/* -------------------------------------------------------------------------- */
ExitCommand::ExitCommand()
    : AbstractCommand("exit")
{}

/* -------------------------------------------------------------------------- */
bool ExitCommand::execute(CommandArgVector args, ostream &os)
    throw (ApplicationError)
{
    return false;
}

/* -------------------------------------------------------------------------- */
StringVector ExitCommand::aliases() const
{
    StringVector sv;
    sv.push_back("quit");
    sv.push_back("q");
    sv.push_back("x");
    return sv;
}

/* -------------------------------------------------------------------------- */
string ExitCommand::help() const
{
    return "Exits the program";
}

/* -------------------------------------------------------------------------- */
void ExitCommand::printLongHelp(ostream &os) const
{
    os << "Name:            exit" << endl;
    os << "Aliases:         quit, q, x" << endl;
    os << endl;
    os << "Description:" << endl;
    os << "Exits the program." << endl;
}

/* HelpCommand {{{1 */

/* -------------------------------------------------------------------------- */
HelpCommand::HelpCommand(Shell *sh)
    : AbstractCommand("help")
{
    m_sh = sh;
}

/* -------------------------------------------------------------------------- */
bool HelpCommand::execute(CommandArgVector args, ostream &os)
    throw (ApplicationError)
{
    for (StringCommandMap::const_iterator it = m_sh->m_commands.begin();
            it != m_sh->m_commands.end(); ++it) {
        if (it->second->name() == it->first)
            os << setw(20) << std::left << it->second->name()
                 << it->second->help() << endl;
    }

    os << endl;
    os << "To get more information about a specific command, use "
       << "\"helpcmd command\"." << endl;

    return true;
}

/* -------------------------------------------------------------------------- */
string HelpCommand::help() const
{
    return "Prints an overview about all commands.";
}

/* -------------------------------------------------------------------------- */
void HelpCommand::printLongHelp(ostream &os) const
{
    os << "Name:            help" << endl;
    os << "Arguments:       command" << endl;
    os << endl;
    os << "Description:" << endl;
    os << "Prints an overview about all commands. To get help for a specific" << endl;
    os << "command, use \"helpcmd command\"." << endl;
}

/* HelpCmdCommand  {{{1 */

/* -------------------------------------------------------------------------- */
HelpCmdCommand::HelpCmdCommand(Shell *sh)
{
    m_sh = sh;
}

/* -------------------------------------------------------------------------- */
bool HelpCmdCommand::execute(CommandArgVector args, ostream &os)
    throw (ApplicationError)
{
    string cmd = args[0]->getString();

    if (m_sh->m_commands.find(cmd) == m_sh->m_commands.end())
        os << "Invalid command: " + cmd << endl;
    else {
        Command *c = m_sh->m_commands.find(cmd)->second;
        c->printLongHelp(os);
    }

    return true;
}

/* -------------------------------------------------------------------------- */
size_t HelpCmdCommand::getArgNumber() const
{
    return 1;
}

/* -------------------------------------------------------------------------- */
CommandArg::Type HelpCmdCommand::getArgType(size_t pos) const
{
    switch (pos) {
        case 0:     return CommandArg::STRING;
        default:    return CommandArg::INVALID;
    }
}

/* -------------------------------------------------------------------------- */
string HelpCmdCommand::getArgTitle(size_t pos) const
{
    switch (pos) {
        case 0:     return "command";
        default:    return "";
    }
}

/* -------------------------------------------------------------------------- */
string HelpCmdCommand::name() const
{
    return "helpcmd";
}

/* -------------------------------------------------------------------------- */
StringVector HelpCmdCommand::aliases() const
{
    StringVector sv;
    sv.push_back("?");
    return sv;
}

/* -------------------------------------------------------------------------- */
string HelpCmdCommand::help() const
{
    return "Prints help for a command";
}

/* -------------------------------------------------------------------------- */
void HelpCmdCommand::printLongHelp(ostream &os) const
{
    os << "Name:            helpcmd" << endl;
    os << "Aliases:         ?" << endl;
    os << "Arguments:       1) the command" << endl;
    os << endl;
    os << "Description:" << endl;
    os << "Prints the help for a given command. To get an overview about" << endl;
    os << "all commands, use \"help\"." << endl;
}


// vim: set sw=4 ts=4 fdm=marker et:
