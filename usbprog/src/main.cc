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
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdlib>

#include <usbprog/usbprog.h>
#include <usbprog/downloader.h>
#include <usbprog/date.h>
#include <usbprog/stringutil.h>

#include "usbprog.h"
#include "io.h"

using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::runtime_error;

int main(int argc, char *argv[])
{
    Usbprog usbprog;

    try {
        usbprog.initConfig();
        usbprog.parseCommandLine(argc, argv);
        usbprog.initFirmwarePool();
        usbprog.initDeviceManager();
        usbprog.exec();
    } catch (runtime_error &e) {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

    Downloader d(cout);
    d.setUrl("http://www.bwalle.de/temp/Test.java");
    d.download();

    DateTime dt;
    cout << dt.getDateTimeString(DTF_ISO_DATETIME) << endl;

    LineReader *r = LineReader::defaultLineReader("(gdb) ");
    string s = r->readLine();
    cout << s << endl;
    delete r;

    return EXIT_SUCCESS;
}

// vim: set sw=4 ts=4 et: :collapseFolds=1:
