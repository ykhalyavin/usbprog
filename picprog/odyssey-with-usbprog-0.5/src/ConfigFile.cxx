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
 * $Id: ConfigFile.cxx,v 1.3 2006/06/07 19:53:41 gaufille Exp $
 */
using namespace std;

#include <stdio.h>
#include <string.h>
#include <stdexcept>
#include <new>
#include "ConfigFile.h"
#include "Util.h"


ConfigFile::ConfigFile(const char *usercfg, const char *progid, const char *syscfg) {
	const char *dir;
	int userfilenamelen;
	bool dot=false;

	dir = getenv("CONFIG");
	if(dir == NULL) {
		dot = true;
		dir = getenv("HOME");
		if(dir == NULL)
			dir = ".";
	}

	if(syscfg != NULL) {
		this->sysfilename = new char[strlen(syscfg)+2];
		if(this->sysfilename == NULL)
			throw bad_alloc();
		strcpy(this->sysfilename, syscfg);
	} else {
		this->sysfilename = NULL;
	}

	userfilenamelen = strlen(dir) + strlen(usercfg) + 4;
	if(progid != NULL) userfilenamelen += (strlen(progid) + 2);
	this->userfilename=new char[userfilenamelen];
	if(this->userfilename == NULL) throw bad_alloc();

	if(progid != NULL) {
		sprintf(this->userfilename, "%s/%s%s", dir, (dot ? "." : ""), progid);
		Util::mkdir_p(this->userfilename);
		sprintf(this->userfilename, "%s/%s%s/%s", dir, (dot ? "." : ""), progid, usercfg);
	} else {
		sprintf(this->userfilename, "%s/%s%s", dir, (dot ? "." : ""), usercfg);
	}
	reread();
}


ConfigFile::~ConfigFile(void) {
	/* I hope you already did a writeback if you wanted to save modifications */
	if(this->userfilename != NULL) delete this->userfilename;
	if(this->sysfilename != NULL) delete this->sysfilename;
}


bool ConfigFile::reread(void) {
	this->cache.clear();
	if(this->sysfilename != NULL) {
		bool a, b;
		a = read_configfile(this->sysfilename, this->cache);
		b = read_configfile(this->userfilename, this->cache);
		return (a | b);
	}
	return read_configfile(this->userfilename, this->cache);
}


bool ConfigFile::read_configfile(char *filename, StringMap& c) {
	char readbuf[100];
	FILE *fp;
	int i, line;
	string section;

	line = 0;
	fp = fopen(filename, "r");
	if(fp == NULL) return false;
	while(fgets(readbuf, sizeof(readbuf), fp) != NULL) {
		line++;
		string buf(readbuf);
		/* Clear out leading and trailing whitespace, including the newline */
		buf.erase(0, buf.find_first_not_of(string(" \t")));
		i = buf.find_last_not_of(string(" \t\n\r"));
		buf.erase(i+1, buf.length()-i-1);

		/* Skip comment and blank lines */
		if((buf.size() == 0) || (buf.find('#') == 0)) continue;

		/* Check for new section header */
		if(buf.find('[') == 0) {
			i = buf.find_first_of(']');
			if(i < 0) goto parse_error;
			section = buf.erase(i);
			section.erase(0, 1);
			continue;
		}

		i = buf.find('=');
		if(i < 0) goto parse_error;

		/* Extract the value and remove leading whitespace */
		string value(buf, i+1, buf.length()-i-1);
		value.erase(0, value.find_first_not_of(string(" \t")));

		/* Extract the key and remove trailing whitespace */
		string key(buf, 0, i);
		i = key.find_last_not_of(string(" \t"));
		key.erase(i+1, key.length()-i-1);

		i = key.find('.');
		if(i < 0) {
			/* Windows ini style entry so make sure there's a section */
			if(section.size() == 0)
				THROW_ERROR(runtime_error, "Variable defined outside " \
				  "of a section in %s, line %d", filename, line);
			/* Create the actual map key */
			key = section + "." + key;
		}

		c.erase(key);
		c.insert(StringMap::value_type(key, value));
	}
	fclose(fp);
	return true;

parse_error:;
	THROW_ERROR(runtime_error, "Error parsing line %d in %s\n",
	  line, filename);
}


bool ConfigFile::writeback(void) {
	FILE *fp;

	fp = fopen(this->userfilename, "w+");
	if(fp == NULL) return false;

	StringMap globals;
	if(this->sysfilename != NULL) {
		/* Read in the system-wide vars so we can compare them. */
		read_configfile(this->sysfilename, globals);
	}

	StringMap::iterator i = this->cache.begin();
	for(; i != this->cache.end(); i++) {
		/* Check the system-wide config */
		StringMap::iterator j = globals.find(i->first);
		if(i != this->cache.end()) {
			/* The variable exists, compare the value */
			if(i->second == j->second) continue;
		}
		fprintf(fp, "%s=%s\n", i->first.c_str(), i->second.c_str());
	}
	fclose(fp);

	return true;
}


bool ConfigFile::get_integer(const string& section, const string& key,
	long *val)
{
	StringMap::iterator i;
	string index;

	index = section + string(".") + key;
	i = this->cache.find(index);
	if(i == this->cache.end())
		return false;

	string value = i->second;
	*val = strtol(value.c_str(), NULL, 0);

	return true;
}


void ConfigFile::set_integer(const string& section, const string& key,
	long val)
{
	string index, value;
	char charbuf[20];

	index = section + string(".") + key;
	snprintf(charbuf, sizeof(charbuf), "%ld", val);
	value = string(charbuf);

	this->cache.erase(index);
	this->cache.insert(StringMap::value_type(index, value));
}


bool ConfigFile::get_integerv(const string& section, const string& key,
	long *valarray, unsigned int *arraylen)
{
	StringMap::iterator i;
	string index;
	int value_pos, next_value_pos;
	unsigned int count;

	index = section + string(".") + key;
	i = this->cache.find(index);
	if(i == this->cache.end())
		return false;

	string value = i->second;
	count = 0;
	value_pos = 0;
	while((value_pos < (int)value.length()) && (count < *arraylen)) {
		next_value_pos = value.find(',', value_pos);
		if(next_value_pos < 0) {
			/* Last entry. Set the next position to the end of the string */
			next_value_pos = value.length();
		}

		if(next_value_pos > value_pos) {
			string intval = value.substr(value_pos, (next_value_pos - value_pos));
			valarray[count] = strtol(intval.c_str(), NULL, 0);
			count++;
		}
		value_pos = next_value_pos + 1;
	}
	*arraylen = count;

	return true;
}


bool ConfigFile::get_boolean(const string& section, const string& key,
	bool *val)
{
	StringMap::iterator i;
	string index;

	index = section + string(".") + key;
	i = this->cache.find(index);
	if(i == this->cache.end())
		return false;

	string value = i->second;
	if((value == "true") || (value == "1") || (value == "yes") ||
		(value == "on"))
	{
		*val = true;
	} else {
		*val = false;
	}

	return true;
}


void ConfigFile::set_boolean(const string& section, const string& key,
	bool val)
{
	string index, value;

	index = section + string(".") + key;
	if(val) {
		value = string("true");
	} else {
		value = string("false");
	}

	this->cache.erase(index);
	this->cache.insert(StringMap::value_type(index, value));
}


bool ConfigFile::get_string(const string& section, const string& key,
	char *buf, int buflen)
{
	StringMap::iterator i;
	string index;

	index = section + string(".") + key;
	i = this->cache.find(index);
	if(i == this->cache.end())
		return false;

	string value = i->second;
	strncpy(buf, value.c_str(), buflen);
	if(buflen > 0)
		buf[buflen-1] = 0;

	return true;
}


void ConfigFile::set_string(const string& section, const string& key,
	const char *buf)
{
	string index, value;

	index = section + string(".") + key;
	value = string(buf);

	this->cache.erase(index);
	this->cache.insert(StringMap::value_type(index, value));
}


vector<string> ConfigFile::get_sections(void) {
	string prev_section;
	vector<string> v;

	StringMap::iterator it = this->cache.begin();
	for(; it != this->cache.end(); it++) {
		int i = it->first.find('.');
		if(i < 0) {
			/* Should never happen */
			continue;
		}
		string section(it->first, 0, i);
		if(section != prev_section) {
			prev_section = section;
			v.push_back(section);
		}
	}

	return v;
}
