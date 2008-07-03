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
#include <iomanip>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <usbprog/stringutil.h>

using std::string;
using std::hex;
using std::stringstream;
using std::strncmp;
using std::malloc;

/* ShellStringTokenizer {{{ */

enum ShellStringTokenizerState {
    STATE_WORD,
    STATE_BACKSLASH,
    STATE_QUOTE,
    STATE_QUOTE_BACKSLASH,
    STATE_SPACE
};

/* -------------------------------------------------------------------------- */
ShellStringTokenizer::ShellStringTokenizer(const string &str)
    : m_string(str), m_pos(0)
{}

/* -------------------------------------------------------------------------- */
bool ShellStringTokenizer::hasMoreTokens() const
{
    return m_pos >= 0 && m_pos < m_string.size();
}

#define isquote(c) \
    (((c) == '"'))
#define isbackslash(c) \
    ((c) == '\\')

/* -------------------------------------------------------------------------- */
string ShellStringTokenizer::nextToken()
{
    ShellStringTokenizerState state = STATE_SPACE;
    string ret;

    while (m_pos < m_string.size()) {
        char c = m_string[m_pos++];

        switch (state) {
            case STATE_SPACE:
                if (isquote(c))
                    state = STATE_QUOTE;
                else if (isbackslash(c))
                    state = STATE_BACKSLASH;
                else {
                    state = STATE_WORD;
                    ret += c;
                }
                break;

            case STATE_WORD:
                if (isspace(c))
                    goto end;
                else if (isquote(c))
                    state = STATE_QUOTE;
                else if (isbackslash(c))
                    state = STATE_BACKSLASH;
                else
                    ret += c;
                break;

            case STATE_QUOTE:
                if (isspace(c))
                    ret += c;
                else if (isquote(c))
                    state = STATE_WORD;
                else if (isbackslash(c))
                    state = STATE_QUOTE_BACKSLASH;
                else
                    ret += c;
                break;

            case STATE_BACKSLASH:
                ret += c;
                state = STATE_WORD;
                break;

            case STATE_QUOTE_BACKSLASH:
                ret += c;
                state = STATE_QUOTE;
                break;
        }
    }

end:
    return ret;
}

/* -------------------------------------------------------------------------- */
StringVector ShellStringTokenizer::tokenize()
{
    StringVector ret;
    while (hasMoreTokens())
        ret.push_back(nextToken());

    return ret;
}

/* }}} */
/* global functions {{{ */

/* -------------------------------------------------------------------------- */
string wordwrap(const string &text, ssize_t margins)
{
    string ret, s;
    stringstream ss;
    ss << text;
    int curline = 0;

    while (ss >> s) {
        if ((margins < 0) || (curline + s.size() + 1 < margins)) {
            if (curline != 0) {
                ret += " ";
                curline++;
            }
            ret += s;
            curline += s.size();
        } else {
            ret += "\n" + s;
            curline = s.size();
        }
    }

    return ret;
}

/* -------------------------------------------------------------------------- */
string strip(string a)
{
    if (a.length() == 0)
        return a;

    a.erase(0, a.find_first_not_of("\n \t", 0));
    a.erase(a.find_last_not_of("\n \t")+1);

    char last = a[0];
    for (string::iterator it = a.begin()+1; it != a.end(); ++it) {
        if (*it == '\n')
            a.erase(it);
        if (*it == '\t')
            *it = ' ';
        else if ((*it == ' ' || *it == '\t') && (last == ' ' || last == '\t'))
            a.erase(it);
        else
            last = *it;
    }

    return a;
}

/* -------------------------------------------------------------------------- */
unsigned long parse_long(const char *string)
{
    stringstream ss;
    if (strncmp(string, "0x", 2) == 0)
        ss << hex << static_cast<const char *>(string + 2);
    else
        ss << string;
    unsigned long ret;
    ss >> ret;

    return ret;
}

/* -------------------------------------------------------------------------- */
char **stringvector_to_array(const StringVector &vec)
{
    if (vec.size() == 0)
        return NULL;

    /* use malloc to be able to free the result with C free() */
    char **ret = (char **)malloc(sizeof(char *) * (vec.size()+1) );

    char **cur = ret;
    for (StringVector::const_iterator it = vec.begin(); it != vec.end(); ++it)
        *cur++ = strdup((*it).c_str());
    *cur = NULL;

    return ret;
}

/* -------------------------------------------------------------------------- */
bool str_starts_with(const std::string &string, const std::string &start)
{
    return start.size() == 0 || string.find(start, 0) == 0;
}

/* -------------------------------------------------------------------------- */
std::vector<std::string> empty_element_sv()
{
    StringVector sv;
    sv.push_back("");
    return sv;
}

/* }}} */

// vim: set sw=4 ts=4 fdm=marker et: :collapseFolds=1:
