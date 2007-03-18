/* Copyright (C) 2002-2003  Mark Andrew Aikens <marka@desert.cx>
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
 * $Id: Util.cxx,v 1.3 2006/06/07 19:55:46 gaufille Exp $
 */
using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>
#include <unistd.h>
#include <stdexcept>
#include "Util.h"


bool Util::mkdir_p(char *path) {
	struct stat info;
	bool retval;
	char *p;

	retval = true;
	for(p = path+1; *p; p++) {
		if(*p == '/') {
			*p = 0;	/* Temporarily NUL terminate */
			/* Check if the directory exists */
			if(stat(path, &info) < 0) {
				if(mkdir(path, 0755) < 0)
					retval = false;
			}
			*p = '/';	/* Replace the '/' */
		}
		if(!retval) break;
	}
	if(stat(path, &info) < 0) {
		if(mkdir(path, 0755) < 0)
			retval = false;
	}
	return retval;
}


bool Util::regex_match(const char *regex, char *string) {
	regex_t regbuf;
	bool retval;
	int err;

	err = regcomp(&regbuf, regex, REG_EXTENDED|REG_NOSUB);
	if(err != 0) {
		char errmsg[100];
		regerror(err, &regbuf, errmsg, sizeof(errmsg));
		throw logic_error(errmsg);
	}

	retval = false;
	if(regexec(&regbuf, string, 0, NULL, 0) == 0)
		retval = true;
	regfree(&regbuf);
	return retval;
}
