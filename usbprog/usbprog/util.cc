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
#include <cstring>
#include <sstream>

#include <usbprog/util.h>
#include <usbprog/date.h>

#ifdef _WIN32
#  include <windows.h>
#  include <shlobj.h>
#else
#  include <pwd.h>
#  include <sys/types.h>
#  include <unistd.h>
#endif

#include <sys/stat.h>

using std::hex;
using std::stringstream;
using std::strncmp;
using std::string;

/* Fileutil {{{1 */

/* -------------------------------------------------------------------------- */
#ifdef _WIN32
string Fileutil::homeDir()
{
    TCHAR path[MAX_PATH];

    if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE|CSIDL_FLAG_CREATE,
                    NULL, 0, path)))
        return string(path);
    else
        return string();
}
#else
string Fileutil::homeDir()
{
    struct passwd *pw;

    pw = getpwuid(getuid());
    if (pw)
        return string(pw->pw_dir);
    else
        return string();
}
#endif

/* -------------------------------------------------------------------------- */
#ifdef _WIN32
string Fileutil::configDir(const string &program)
{
    TCHAR path[MAX_PATH];

    if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE,
                    NULL, 0, path)))
        return string(path) + "/" + program;
    else
        return string();
}
#else
string Fileutil::configDir(const string &program)
{
    return homeDir() + "/." + program;
}
#endif

/* -------------------------------------------------------------------------- */
#ifdef _WIN32
bool Fileutil::mkdir(const string &dir)
{
    return SUCCEEDED(CreateDirectory(dir.c_str(), NULL));
}
#else
bool Fileutil::mkdir(const string &dir)
{
    return ::mkdir(dir.c_str(), 0777) == 0;
}
#endif

/* -------------------------------------------------------------------------- */
bool Fileutil::isDir(const string &dir)
{
    int         ret;
    struct stat my_stat;

    ret = stat(dir.c_str(), &my_stat);
    if (ret < 0)
        return false;

    return S_ISDIR(my_stat.st_mode);
}

/* -------------------------------------------------------------------------- */
bool Fileutil::isFile(const string &file)
{
    int         ret;
    struct stat my_stat;

    ret = stat(file.c_str(), &my_stat);
    if (ret < 0)
        return false;

    return S_ISREG(my_stat.st_mode);
}

/* -------------------------------------------------------------------------- */
DateTime Fileutil::getMTime(const std::string &file)
    throw (IOError)
{
    int         ret;
    struct stat my_stat;

    ret = stat(file.c_str(), &my_stat);
    if (ret < 0)
        throw IOError("File " + file + " does not exist.");

    return DateTime(my_stat.st_mtime);
}

/* global {{{1 */

/* -------------------------------------------------------------------------- */
#ifdef _WIN32
string pathconcat(const string &a, const string &b)
{
    return a + "\\" + b;
}
#else
string pathconcat(const string &a, const string &b)
{
    return a + "/" + b;
}
#endif

/* -------------------------------------------------------------------------- */
void usbprog_sleep(unsigned int seconds)
{
    usbprog_msleep(seconds * 1000);
}

/* -------------------------------------------------------------------------- */
#ifdef _WIN32
void usbprog_msleep(unsigned int msec)
{
    Sleep(msec);
}
#else
void usbprog_msleep(unsigned int msec)
{
    struct timespec ts;

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000 * 1000;
    nanosleep(&ts, NULL);
}
#endif

// vim: set sw=4 ts=4 fdm=marker et:
