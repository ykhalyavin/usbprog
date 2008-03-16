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
#ifndef IO_H
#define IO_H

#include <usbprog/usbprog.h>

/* Interface for completors {{{1 */
class Completor {
    public:
        virtual ~Completor() {}

    public:
        virtual std::vector<std::string> complete(const std::string &text,
                const std::string &full_text, size_t start_idx, ssize_t end_idx) = 0;
};

/* Interface for a linereader {{{1 */
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

        virtual bool haveCompletion() const = 0;
        virtual void setCompletor(Completor *comp) = 0;
};

/* Abstract base class for line readers {{{1 */

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
        bool haveHistory() const;

        bool haveCompletion() const;
        void setCompletor(Completor *comp);

    protected:
        void setEof(bool elf);

    private:
        std::string m_prompt;
        bool m_eof;
};

#endif /* IO_H */

// vim: set sw=4 ts=4 fdm=marker et:
