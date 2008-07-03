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
#include "configuration.h"

Configuration* Configuration::m_instance = 0;

using std::string;
using std::endl;

/* -------------------------------------------------------------------------- */
Configuration* Configuration::config()
{
    if (!m_instance)
        m_instance = new Configuration();

    return m_instance;
}

/* -------------------------------------------------------------------------- */
Configuration::Configuration()
    : m_debug(false), m_offline(false)
{}

/* -------------------------------------------------------------------------- */
void Configuration::setDataDir(const string &dir)
{
    m_dataDir = dir;
}

/* -------------------------------------------------------------------------- */
string Configuration::getDataDir() const
{
    return m_dataDir;
}

/* -------------------------------------------------------------------------- */
void Configuration::setHistoryFile(const string &history)
{
    m_historyFile = history;
}

/* -------------------------------------------------------------------------- */
string Configuration::getHistoryFile() const
{
    return m_historyFile;
}

/* -------------------------------------------------------------------------- */
void Configuration::dumpConfig(std::ostream &stream)
{
    stream << "dataDir     = " << m_dataDir << endl;
    stream << "offline     = " << m_offline << endl;
    stream << "debug       = " << m_debug << endl;
}

/* -------------------------------------------------------------------------- */
void Configuration::setDebug(bool debug)
{
    m_debug = debug;
}

/* -------------------------------------------------------------------------- */
bool Configuration::getDebug() const
{
    return m_debug;
}

/* -------------------------------------------------------------------------- */
void Configuration::setOffline(bool offline)
{
    m_offline = offline;
}

/* -------------------------------------------------------------------------- */
bool Configuration::isOffline() const
{
    return m_offline;
}

/* -------------------------------------------------------------------------- */
void Configuration::setIndexUrl(const std::string &url)
{
    m_indexUrl = url;
}

/* -------------------------------------------------------------------------- */
string Configuration::getIndexUrl() const
{
    return m_indexUrl;
}

/* -------------------------------------------------------------------------- */
void Configuration::setBatchMode(bool batch)
{
    m_batchMode = batch;
}

/* -------------------------------------------------------------------------- */
bool Configuration::getBatchMode() const
{
    return m_batchMode;
}


// vim: set sw=4 ts=4 et: :collapseFolds=1:
