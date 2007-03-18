/* Copyright (C) 2002-2004  Mark Andrew Aikens <marka@desert.cx>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: ConfigFile.h,v 1.2 2005/11/25 22:16:34 uid20104 Exp $
 */
#ifndef __ConfigFile_h
#define __ConfigFile_h

#include <string>
#include <vector>
#include <map>

/** \file */



/** A shortcut to a map with strings as the key and value. This is used to
 * store the configuration file in memory. */
typedef map<string, string> StringMap;


/** This class is a wrapper for configuration files. It reads the config file
 * in when the object is created so accessing variables is fast. It also
 * supports writing back the variables to the config file with the writeback()
 * method.
 */
class ConfigFile {
public:
	/** Create a ConfigFile object and read in the configuration file(s).
	 * If the CONFIG environment variable is set, the usercfg file will be
	 * opened/created in the directory specified by that variable. Otherwise
	 * it will be opened/created in the user's HOME directory. The reread()
	 * method is then called to read in the configuration variables.
	 *
	 * \param usercfg The name of the user's configuration file.
	 * \param progid If this is not NULL, the config file will be located
	 *        under a subdirectory with this name.
	 * \param syscfg The name of an optional global system-wide config file.
	 * \throws bad_alloc If there is insufficient memory.
	 */
	ConfigFile(const char *usercfg, const char *progid=NULL,
		const char *syscfg=NULL);

	/** Frees all the memory and resources associated with this object */
	virtual ~ConfigFile();

	/** Clear the internal cache and re-read the configuration files.
	 * If a system-wide configuration file is specified, that will be read
	 * before the user's file and the user's values will override the system
	 * defaults.
	 * \returns A boolean value indicating if the read succeeded.
	 * \throws runtime_error Contains a textual description of the error.
	 */
	virtual bool reread(void);

	/** Write the contents of the internal cache back to the configuration
	 * file. If a system-wide configuration file has been specified, any
	 * values different than, or not defined in the system-wide defaults
	 * will be written to the user's config file.
	 * \returns A boolean value indicating if the writeback succeeded.
	 */
	virtual bool writeback(void);

	/** Gets the integer value of a configuration variable from the internal
	 * cache.
	 * \param section The section name in the configuration file to look
	 *        for the key.
	 * \param key The name of the configuration variable to retrieve from
	 *        the configuration section.
	 * \param val A pointer to the location in which to store the value.
	 * \returns A boolean value indicating if the value was found or not.
	 */
	bool get_integer(const string& section, const string& key, long *val);

	/** Sets the value of an integer configuration variable in the internal
	 * cache. The value will not be written to any file until writeback() is
	 * called.
	 * \param section The section name of the configuration file to set the
	 *        variable in.
	 * \param key The name of the configuration variable to set in the
	 *        section.
	 * \param val The value of the integer variable to set.
	 */
	void set_integer(const string& section, const string& key, long val);

	/** Gets an array of integer values from a configuration variable in the
	 * internal cache.
	 * \param section The section name in the configuration file to look
	 *        for the key.
	 * \param key The name of the configuration variable to retrieve from
	 *        the configuration section.
	 * \param valarray A pointer to the location of the array where the values
	 *        will be stored.
	 * \param arraylen Pointer to the length of the array valarray. Only this
	 *        many values will be returned.
	 * \returns A boolean value indicating if the value was found or not. The
	 *        value of the arraylen variable will be set to the number of
	 *        elements actually returned in the array.
	 */
	bool get_integerv(const string& section, const string& key, long *valarray, unsigned int *arraylen);

	/** Gets the boolean value of a configuration variable from the internal
	 * cache.
	 * \param section The section name in the configuration file to look
	 *        for the key.
	 * \param key The name of the configuration variable to retrieve from
	 *        the configuration section.
	 * \param val A pointer to the location in which to store the value.
	 * \returns A boolean value indicating if the value was found or not.
	 */
	bool get_boolean(const string& section, const string& key, bool *val);

	/** Sets the value of a boolean configuration variable in the internal
	 * cache. The value will not be written to any file until writeback() is
	 * called.
	 * \param section The section name of the configuration file to set the
	 *        variable in.
	 * \param key The name of the configuration variable to set in the
	 *        section.
	 * \param val The value of the boolean variable to set.
	 */
	void set_boolean(const string& section, const string& key, bool val);

	/** Gets the value of a string configuration variable from the internal
	 * cache.
	 * \param section The section name in the configuration file to look
	 *        for the key.
	 * \param key The name of the configuration variable to retrieve from
	 *        the configuration section.
	 * \param buf A pointer to a memory region where the string will be copied.
	 * \param buflen The length of the buffer.
	 * \returns A boolean value indicating if the value was found or not. On
	 *        success, the string will have been copied to the provided buffer
	 *        and NUL terminated.
	 */
	bool get_string(const string& section, const string& key, char *buf, int buflen);

	/** Sets the value of a string configuration variable in the internal
	 * cache. The value will not be written to any file until writeback()
	 * is called.
	 * \param section The section name of the configuration file to set the
	 *        variable in.
	 * \param key The name of the configuration variable to set in the
	 *        section.
	 * \param buf A pointer to the value of the string.
	 */
	void set_string(const string& section, const string& key, const char *buf);

	/** Gets a list of the sections defined in the configuration file.
	 * \returns A vector of string objects, each containing the name of
	 *     a section.
	 */
	vector<string> get_sections(void);

protected:
	/** An internal helper function that will read the configuration values
	 * of a configuration file into a StringMap.
	 * \param filename The name of the configuration file to read.
	 * \param c A StringMap in which to store the configuration variables.
	 * \returns A boolean value indicating if the read succeeded.
	 */
	bool read_configfile(char *filename, StringMap& c);

	/** The full path to the configuration file where user-specific variables
	 * are stored. This is normally under $HOME. */
	char *userfilename;

	/** The full path to the system-wide configuration file. */
	char *sysfilename;

	/** The map which stores all the configuration variables in memory for
	 * fast access. */
	StringMap cache;
};


#endif
