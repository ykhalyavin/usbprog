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
 * $Id: Util.h,v 1.3 2006/06/07 19:55:58 gaufille Exp $
 */
#ifndef __Util_h
#define __Util_h

#include <stdio.h>
#include "ConfigFile.h"

/** \file */


/** Throw an exception with a printf formatted what() string.
 * \param exception The exception class to throw.
 * \param fmt The printf format string.
 * \param args An optional list of arguments for the format string.
 */
#define THROW_ERROR(exception, fmt, args...) \
	do { \
		char __error_string_buf[80]; \
		snprintf(__error_string_buf, sizeof(__error_string_buf), fmt, ##args); \
		throw exception(__error_string_buf); \
	} while(0)


/** A pointer to the main Odyssey configuration file. */
extern ConfigFile *config;


/** A class composed entirely of random static utility functions */
class Util {
public:
	/** Makes a set of directories along a path. This is equivalent to doing
	 * 'mkdir -p path'.
	 * \param path The path of directories to create.
	 * \returns A boolean value indicating if all the directories could be
	 *          created.
	 */
	static bool mkdir_p(char *path);

	/** Match a string against an extended regular expression.
	 * \param regex The regular expression.
	 * \param string The string to test against the regular expression.
	 * \returns A boolean value indicating if the string matched the
	 *          regular expression.
	 * \throws logic_error If the regex compilation fails.
	 */
	static bool regex_match(const char *regex, char *string);
};


#endif
