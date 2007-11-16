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
#include <iostream>
#include <cstdlib>
#include <stdexcept>

#include <usbprog/util.h>
#include <usbprog/firmwarepool.h>
#include <usbprog/devices.h>

#include "optionparser.h"
#include "usbprog.h"
#include "configuration.h"
#include "shell.h"
#include "commands.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

using std::string;
using std::cerr;
using std::cout;
using std::endl;
using std::exit;

/* HashNotifier {{{1 */

/* -------------------------------------------------------------------------- */
HashNotifier::HashNotifier(int width)
    : m_width(width), m_lastProgress(0)
{}

/* -------------------------------------------------------------------------- */
HashNotifier::~HashNotifier()
{
    if (m_lastProgress != 0)
        finished();
}

/* -------------------------------------------------------------------------- */
int HashNotifier::progressed(double total, double now)
{
    /* zero division */
    if (total < 0.01)
        return true;

    double percent = now / total;
    int bars = int(percent * m_width);
    while (bars > m_lastProgress) {
        cout << '#';
        m_lastProgress++;
    }

    return true;
}

/* -------------------------------------------------------------------------- */
void HashNotifier::finished()
{
    if (m_lastProgress != 0) {
        cout << endl;
        m_lastProgress = 0;
    }
}


/* Usbprog {{{1 */

/* -------------------------------------------------------------------------- */
Usbprog::Usbprog()
    : m_firmwarepool(NULL), m_progressNotifier(NULL)
{}

/* -------------------------------------------------------------------------- */
Usbprog::~Usbprog()
{
    delete m_firmwarepool;
    delete m_progressNotifier;
}

/* -------------------------------------------------------------------------- */
void Usbprog::initConfig()
    throw (ApplicationError)
{
    Configuration *conf = Configuration::config();

    string configDir = Fileutil::configDir("usbprog");
    if (configDir.size() == 0)
        throw ApplicationError("Could not determine configuration "
                "directory.");

    conf->setDataDir(configDir);
    conf->setHistoryFile(pathconcat(configDir, "history"));
    conf->setIndexUrl(DEFAULT_INDEX_URL);
}

/* -------------------------------------------------------------------------- */
void Usbprog::parseCommandLine(int argc, char *argv[])
{
    Configuration *conf = Configuration::config();

    OptionParser op;
    op.addOption("help", 'h', OT_FLAG, "Prints a help message");
    op.addOption("version", 'v', OT_FLAG, "Shows version information");
    op.addOption("datadir", 'd', OT_STRING, "Uses the specified data "
            "directory instead of " + conf->getDataDir());
    op.addOption("offline", 'o', OT_FLAG, "Use only the local cache "
            "and don't connect to the internet");
    op.addOption("debug", 'D', OT_FLAG, "Enables debug output");

    bool ret;
    ret = op.parse(argc, argv);
    if (!ret)
        throw ApplicationError("Parsing command line failed");

    if (op.getValue("debug").getFlag())
        conf->setDebug(true);

    if (op.getValue("help").getFlag()) {
        op.printHelp(cerr, PACKAGE_STRING);
        exit(EXIT_SUCCESS);
    }

    if (op.getValue("version").getFlag()) {
        cerr << PACKAGE_STRING << endl;
        exit(EXIT_SUCCESS);
    }

    OptionValue option = op.getValue("datadir");
    if (option.getType() != OT_INVALID)
        conf->setDataDir(option.getString());
    if (op.getValue("offline").getFlag())
        conf->setOffline(true);

    if (conf->getDebug())
        conf->dumpConfig(cerr);

    // batch mode?
    conf->setBatchMode(op.getArgs().size() > 0);
    m_args = op.getArgs();

    if (conf->isOffline() && !conf->getBatchMode())
        cout << "WARNING: You're using usbprog in offline mode!" << endl;

    if (!conf->getBatchMode())
        m_progressNotifier = new HashNotifier(DEFAULT_TERMINAL_WIDTH);
}

/* -------------------------------------------------------------------------- */
void Usbprog::initFirmwarePool()
    throw (ApplicationError)
{
    Configuration *conf = Configuration::config();

    try {
        m_firmwarepool = new Firmwarepool(conf->getDataDir());
        m_firmwarepool->setIndexUpdatetime(AUTO_NOT_UPDATE_TIME);
        if (!conf->isOffline())
            m_firmwarepool->downloadIndex(conf->getIndexUrl());
        m_firmwarepool->setProgress(m_progressNotifier);
        m_firmwarepool->readIndex();
    } catch (const IOError &ioe) {
        throw ApplicationError(ioe.what());
    }
}

/* -------------------------------------------------------------------------- */
void Usbprog::initDeviceManager()
    throw (ApplicationError)
{
    m_devicemanager = new DeviceManager;
}

/* -------------------------------------------------------------------------- */
void Usbprog::exec()
    throw (ApplicationError)
{
    Shell sh("(usbprog) ");

    sh.addCommand(new CopyingCommand);
    sh.addCommand(new ListCommand(m_firmwarepool));
    sh.addCommand(new InfoCommand(m_firmwarepool));
    sh.addCommand(new PinCommand(m_firmwarepool));
    sh.addCommand(new DownloadCommand(m_firmwarepool));
    sh.addCommand(new CacheCommand(m_firmwarepool));
    sh.addCommand(new DevicesCommand(m_devicemanager, m_firmwarepool));
    sh.addCommand(new DeviceCommand(m_devicemanager, m_firmwarepool));
    sh.addCommand(new UploadCommand(m_devicemanager, m_firmwarepool));
    if (Configuration::config()->getBatchMode())
        sh.run(m_args);
    else
        sh.run();
}

// vim: set sw=4 ts=4 fdm=marker et:
