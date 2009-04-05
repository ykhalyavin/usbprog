/*
 * (c) 2007-2008, Robert Schilling
 *                Bernhard Walle <bernhard.walle@gmx.de>
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
#include <cstdio>
#include <usbprog/optionparser.h>

#include "usbprogApp.h"
#include "usbprogFrm.h"
#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

using std::cerr;
using std::endl;

IMPLEMENT_APP(usbprogFrmApp)

/* -------------------------------------------------------------------------- */
void parse_command_line(int argc, wxChar **argv)
{
    char **nargv = new char*[argc];

    for (int i = 0; i < argc; i++)
        nargv[i] = strdup(wxString(argv[i]).mb_str());

    OptionParser op;
    op.addOption("help", 'h', OT_FLAG, "Prints a help message");
    op.addOption("version", 'v', OT_FLAG, "Shows version information");
    op.addOption("debug", 'D', OT_FLAG, "Enables debug output");

    bool ret;
    ret = op.parse(argc, nargv);
    if (!ret) {
        cerr << "Parsing command line failed" << endl;
        exit(EXIT_FAILURE);
    }

    if (op.getValue("debug").getFlag())
        Debug::debug()->setLevel(Debug::DL_TRACE);

    if (op.getValue("help").getFlag()) {
        op.printHelp(cerr, "usbprog " USBPROG_VERSION_STRING);
        exit(EXIT_SUCCESS);
    }

    if (op.getValue("version").getFlag()) {
        cerr << "usbprog-gui " << USBPROG_VERSION_STRING << endl;
        exit(EXIT_SUCCESS);
    }

    for (int i = 0; i < argc; i++)
        free(nargv[i]);
    delete[] nargv;
}

/* -------------------------------------------------------------------------- */
bool usbprogFrmApp::OnInit()
{
    parse_command_line(argc, argv);

    usbprogFrm* frame = new usbprogFrm(NULL);
    frame->Show(true);
    SetTopWindow(frame);

#ifdef __APPLE__
    // this is necessary because of
    // http://wiki.wxwidgets.org/WxMac_Issues#My_app_can.27t_be_brought_to_the_front.21
    ProcessSerialNumber PSN;
    GetCurrentProcess(&PSN);
    TransformProcessType(&PSN, kProcessTransformToForegroundApplication);
#endif

    return true;
}

/* -------------------------------------------------------------------------- */
int usbprogFrmApp::OnExit()
{
	return 0;
}

// vim: set sw=4 ts=4 fdm=marker et: :collapseFolds=1:
