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
#ifndef USBPROG_DIGEST_H
#define USBPROG_DIGEST_H

#include <string>

#include <usbprog/usbprog.h>

/* Digest {{{1 */

class Digest {
    public:
        enum Algorithm {
            DA_MD5
        };

    public:
        virtual ~Digest() {};

    public:
        virtual void process(unsigned char *buffer, size_t len) = 0;
        std::string end();
};

/* MD5Digest {{{1 */

class MD5Digest : public Digest {
    public:
        MD5Digest();
        ~MD5Digest();

    public:
        void process(unsigned char *buffer, size_t len);
        std::string end();

    private:
        void *m_md5; /* using void here because that header file should
                        not include md5 stuff and a simple forward declaration
                        because md5_t is "typedef struct { } ..." instead of
                        "struct ... { }" */
};

/* check_digest() {{{1 */

bool check_digest(const std::string &file, 
        const std::string &reference,
        Digest::Algorithm da) throw(IOError);


/* }}} */

#endif /* USBPROG_DIGEST_H */

// vim: set sw=4 ts=4 foldmethod=marker et:
