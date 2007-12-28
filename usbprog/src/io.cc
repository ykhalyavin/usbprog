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
#include <cerrno>
#include <cstring>
#include <cstdlib>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_LIBREADLINE
# include <readline/readline.h>
# include <readline/history.h>
#endif

#include <usbprog/usbprog.h>
#include "io.h"

using std::string;
using std::getline;
using std::cin;
using std::cout;
using std::endl;
using std::strerror;
using std::free;

/* class definitions {{{1 */

#ifdef HAVE_LIBREADLINE
class ReadlineLineReader : public AbstractLineReader {
    public:
        ReadlineLineReader(const string &prompt);

    public:
        string readLine(const char *prompt = NULL);
        void readHistory(const std::string &file)
            throw (IOError);
        void writeHistory(const std::string &file)
            throw (IOError);
        bool haveHistory() const;
};
#endif

class SimpleLineReader : public AbstractLineReader {
    public:
        SimpleLineReader(const string &prompt);

    public:
        string readLine(const char *prompt = NULL);
};

/* LineReader {{{1 */

/* -------------------------------------------------------------------------- */
LineReader *LineReader::defaultLineReader(const string &prompt)
{
#ifdef HAVE_LIBREADLINE
    return new ReadlineLineReader(prompt);
#else
    return new SimpleLineReader(prompt);
#endif
}

/* AbstractLineReader {{{1 */

/* -------------------------------------------------------------------------- */
AbstractLineReader::AbstractLineReader(const string &prompt)
    : m_prompt(prompt), m_eof(false)
{}

/* -------------------------------------------------------------------------- */
string AbstractLineReader::getPrompt() const
{
    return m_prompt;
}

/* -------------------------------------------------------------------------- */
void AbstractLineReader::setEof(bool eof)
{
    m_eof = eof;
}

/* -------------------------------------------------------------------------- */
bool AbstractLineReader::eof() const
{
    return m_eof;
}

/* -------------------------------------------------------------------------- */
void AbstractLineReader::readHistory(const std::string &file)
    throw (IOError)
{}

/* -------------------------------------------------------------------------- */
void AbstractLineReader::writeHistory(const std::string &file)
    throw (IOError)
{}

/* -------------------------------------------------------------------------- */
bool AbstractLineReader::haveHistory() const
{
    return false;
}

/* SimpleLineReader {{{1 */

/* -------------------------------------------------------------------------- */
SimpleLineReader::SimpleLineReader(const string &prompt)
    : AbstractLineReader(prompt)
{}

/* -------------------------------------------------------------------------- */
string SimpleLineReader::readLine(const char *prompt)
{
    string ret;
    if (!prompt)
        cout << getPrompt();
    else
        cout << prompt;
    getline(cin, ret, '\n');
    if (cout.eof())
        setEof(true);
    return ret;
}

/* ReadlineLineReader {{{1 */

#ifdef HAVE_LIBREADLINE
/* -------------------------------------------------------------------------- */
ReadlineLineReader::ReadlineLineReader(const string &prompt)
    : AbstractLineReader(prompt)
{}

/* -------------------------------------------------------------------------- */
string ReadlineLineReader::readLine(const char *prompt)
{
    char *line_read;
    string ret;

    line_read = readline(prompt ? prompt : getPrompt().c_str());
    if (!line_read)
        setEof(true);
    else if (*line_read) {
        if (!prompt)
            add_history(line_read);
        ret = string(line_read);
        free(line_read);
    }

    return ret;
}

/* -------------------------------------------------------------------------- */
void ReadlineLineReader::readHistory(const std::string &file)
    throw (IOError)
{
    int ret = read_history(file.c_str());
    if (ret < 0)
        throw IOError(string("Reading readline history failed: ")
                + strerror(errno));
}

/* -------------------------------------------------------------------------- */
void ReadlineLineReader::writeHistory(const std::string &file)
    throw (IOError)
{
    int ret = write_history(file.c_str());
    if (ret < 0)
        throw IOError(string("Writing readline history failed: ")
                + strerror(errno));
}

/* -------------------------------------------------------------------------- */
bool ReadlineLineReader::haveHistory() const
{
    return true;
}

#endif


// vim: set sw=4 ts=4 fdm=marker et:
