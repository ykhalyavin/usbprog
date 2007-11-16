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
#ifndef IO_H
#define IO_H

#include <usbprog/usbprog.h>

class LineReader {
    public:
        virtual ~LineReader() {}

    public:
        static LineReader *defaultLineReader(const std::string &prompt); /* factory */

    public:
        virtual std::string readLine(const char *prompt = NULL) = 0;
        virtual std::string getPrompt() const = 0;
        virtual bool eof() const = 0;
        virtual void readHistory(const std::string &file)
            throw (IOError) = 0;
        virtual void writeHistory(const std::string &file)
            throw (IOError) = 0;
        virtual bool haveHistory() const = 0;
};

class AbstractLineReader : public LineReader {
    public:
        AbstractLineReader(const std::string &prompt);

    public:
        std::string getPrompt() const;
        bool eof() const;
        void readHistory(const std::string &file)
            throw (IOError);
        void writeHistory(const std::string &file)
            throw (IOError);
        virtual bool haveHistory() const;

    protected:
        void setEof(bool elf);

    private:
        std::string m_prompt;
        bool m_eof;
};

#endif /* IO_H */

// vim: set sw=4 ts=4 et:
