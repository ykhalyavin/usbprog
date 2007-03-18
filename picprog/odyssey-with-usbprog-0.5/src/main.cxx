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
 * $Id: main.cxx,v 1.4 2006/06/07 19:54:52 gaufille Exp $
 */
using namespace std;

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>
#include <exception>
#include <iterator>
#include <vector>
#include <string>
#include "config.h"
#include "ConfigFile.h"
#include "DataBuffer.h"
#include "HexFile.h"
#include "IO.h"
#include "Device.h"


/* Global variables */
ConfigFile *config;



static char *argv0;
static Device *device;
static IO *io;
static bool quiet;

static void do_listdevices(void);
static int do_init(int argc, char *argv[]);
static int do_write(int argc, char *argv[]);
static int do_read(int argc, char *argv[]);
static int do_verify(int argc, char *argv[]);
static int do_check(int argc, char *argv[]);
static int do_erase(int argc, char *argv[]);
static int do_blankcheck(int argc, char *argv[]);
static int do_test(int argc, char *argv[]);


static const struct cmd {
	const char *name;
	int  (*func)(int argc, char *argv[]);
	bool requires_chiptype;
	const char *desc;
} commands[] = {
	{"init", do_init, false, "Prepares the programmer board for use"},
	{"test", do_test, false, "Enters programmer board test mode"},
	{"check", do_check, true, "Checks that the PIC and board are good"},
	{"write", do_write, true, "Writes data to PIC"},
	{"read", do_read, true, "Reads the contents of a PIC"},
	{"verify", do_verify, true, "Verifies the contents of a PIC"},
	{"erase", do_erase, true, "Erases an electrically erasable PIC"},
	{"blankcheck", do_blankcheck, true, "Checks if a PIC is erased"},
};
static const int num_commands = (sizeof(commands) / sizeof(struct cmd));


static int usage(void) {
	int i;

	fprintf(stderr,
		"Usage:\n" \
		"  odyssey [options] init\n"
		"  odyssey [options] test\n"
		"  odyssey [options] chiptype command [command options]\n"
		"\n" \
		"Options:\n" \
		"  -V            Print the version of Odyssey and exit\n" \
		"  -q            Quiet mode. Don't display the progress counters.\n" \
		"  -f filename   Use an alternate configuration file in ~/.odyssey/\n" \
		"  -v s.var=val  Override a configuration file variable\n" \
		"  -l            List supported values for chiptype and exit\n" \
		"\n" \
		"Commands:\n");

	for(i=0; i<num_commands; i++)
		fprintf(stderr, "  %-14s %s\n", commands[i].name, commands[i].desc);
	fprintf(stderr, "\n");

	return 1;
}


static void sighandler(int sig) {
	fprintf(stderr, "Caught signal %d.\n", sig);
	if(io != NULL)
		delete io;
	exit(1);
}


static bool progress(void *data, long addr, int percent) {
	static int last_percent = 101;
	static int output_is_tty = -1;

	if(!quiet && (percent != last_percent)) {
		last_percent = percent;
		if(output_is_tty < 0)
			output_is_tty = isatty(1);
		if(output_is_tty) {
			printf("\033[1GAddress: 0x%06lx, % 3d%% done", addr, percent);
			fflush(stdout);
		} else {
			printf("Address: 0x%06lx, % 3d%% done\n", addr, percent);
		}
	}
	return true;
}


int main(int argc, char *argv[]) {
	char ioname[30];
	int retval, i;
	long ioport;
	const char *user_configfile="config";
	vector<string> var_override;

	quiet = false;
	while((i = getopt(argc, argv, "qVv:f:l+")) > 0) {
		switch(i) {
		case 'q':
			quiet = true;
			break;
		case 'V':
			printf("Odyssey version %s\n", VERSION);
			return 0;
		case 'v':
			var_override.push_back(string(optarg));
			break;
		case 'f':
			user_configfile = optarg;
			break;
		case 'l':
			do_listdevices();
			return 0;
		case '?':
		case ':':
		default:;
			return usage();
		}
	}

	if(optind == argc) {
		fprintf(stderr, "%s: Not enough arguments.\n", argv[0]);
		return usage();
	}

	/* Read in the configuration file */
	try {
		config = new ConfigFile(user_configfile, PACKAGE,
		  SYSCONFDIR "/" PACKAGE ".conf");
	} catch(std::exception& e) {
		fprintf(stderr, "Error: %s\n", e.what());
		return 1;
	}

	/* Process config variables set on the command line */
	vector<string>::iterator it = var_override.begin();
	for(; it != var_override.end(); it++) {
		int j = it->find('=');
		/* Check for the form a.b=c */
		if((j < 0) || ((i = (int)it->rfind('.', j)) < 0)) {
			fprintf(stderr, "Invalid syntax for specifying a configuration variable\n");
			return 1;
		}
		string section(*it, 0, i);
		string key(*it, i+1, j-i-1);
		string value(*it, j+1);
		config->set_string(section, key, value.c_str());
	}
	var_override.clear();

	/* Default IO driver parameters */
	if(! config->get_string("io", "driver", ioname, sizeof(ioname))) {
		strcpy(ioname, "DirectPP");
	}
	if(! config->get_integer("io", "port", &ioport)) {
		ioport = 0;
	}

	/* Initialize the hardware */
	try {
		io = IO::acquire(ioname, ioport);
	} catch(std::exception& e) {
		fprintf(stderr, "\n%s: I/O init: %s\n", argv[0], e.what());
		retval = 1;
		goto exit1;
	}


	/* commands that don't need a chiptype */
	for(i=0; i<num_commands; i++) {
		if(!strcmp(argv[optind], commands[i].name)) {
			retval = 1;
			if(commands[i].requires_chiptype) {
				fprintf(stderr,
				  "\n%s: Missing chiptype for command %s\n",
				  argv[0], argv[optind]);
			} else {
				retval = commands[i].func(argc-optind, &argv[optind]);
			}
			goto exit2;
		}
	}

	/* Load the device configuration */
	device = Device::load(argv[optind]);
	if(device == NULL) {
		fprintf(stderr, "\n%s: Couldn't load configuration for '%s' device.\n",
		  argv[0], argv[optind]);
		retval = 1;
		goto exit2;
	}
	optind++;

	if(optind == argc) {
		fprintf(stderr, "%s: Need to specify a command.\n", argv[0]);
		retval = usage();
		goto exit3;
	}

	/* Catch some signals to properly shut down the hardware */
	signal(SIGHUP, sighandler);
	signal(SIGINT, sighandler);
	signal(SIGQUIT, sighandler);
	signal(SIGPIPE, sighandler);
	signal(SIGTERM, sighandler);

	device->set_iodevice(io);
	device->set_progress_cb(progress);

	retval = 1;
	argv0 = argv[0];

	/* Verify that the hardware is working and the correct chip is in
	 * the socket. */
	try {
		device->check();
	} catch(std::exception& e) {
		fprintf(stderr, "%s: %s: %s\n", argv0, device->get_name().c_str(), e.what());
		goto exit4;
	}

	for(i=0; i<num_commands; i++) {
		if(!strcmp(argv[optind], commands[i].name)) {
			retval = commands[i].func(argc-optind, &argv[optind]);
			break;
		}
	}

	if(i == num_commands) {
		fprintf(stderr, "%s: Invalid command '%s'.\n", argv[0], argv[optind]);
		retval = usage();
	}

exit4:;
	device->set_iodevice(NULL);

exit3:;
	delete device;

exit2:;
	delete io;
	io = NULL;

exit1:;
	delete config;
	return retval;
}


static void do_listdevices(void) {
	vector<string>::iterator it;
	vector<string> *devlist;
	string line;

	devlist = Device::list();
	printf("Supported values for chiptype. A * indicates that it has not yet been tested.\n");
	for(it=devlist->begin(); it != devlist->end();) {
		if(line.empty()) {
			line.append("  ");
			line.append(*it);
			it++;
		} else if(line.size() + it->size() > 74) {
			printf("%s\n", line.c_str());
			line.erase();
		} else {
			line.append(", ");
			line.append(*it);
			it++;
		}
	}
	if(! line.empty())
		printf("%s\n", line.c_str());

	delete devlist;
}

static int do_init(int argc, char *argv[]) {
	/* Hardware already configured in IO's subclasses' constructor */
	return 0;
}

static int do_write(int argc, char *argv[]) {
	if(argc < 2) {
		fprintf(stderr, "\n%s: %s requires a filename.\n", argv0, argv[0]);
		fprintf(stderr,
			"Usage:\n  %s [options] chiptype %s filename.hex\n\n",
			  argv0, argv[0]);
		return 1;
	}

	DataBuffer buf(device->get_wordsize());

	try {
		/* Read the hex file into the data buffer */
		HexFile *hf = HexFile::load(argv[1]);

		hf->read(buf);
		delete hf;
	} catch(std::exception& e) {
		fprintf(stderr, "%s: %s\n", argv[1], e.what());
		return 1;
	}

	try {
		device->program(buf);
		printf("\n"); /* Newline to finish up the progress meter */
	} catch(std::exception& e) {
		printf("\n"); /* Newline to finish up the progress meter */
		fprintf(stderr, "%s: %s: %s\n", argv0, device->get_name().c_str(), e.what());
		return 1;
	}

	return 0;
}


static int do_read(int argc, char *argv[]) {
	if(argc < 2) {
		fprintf(stderr, "\n%s: %s requires a filename.\n", argv0, argv[0]);
		fprintf(stderr,
			"Usage:\n  %s [options] chiptype %s filename.hex\n\n",
			  argv0, argv[0]);
		return 1;
	}

	DataBuffer buf(device->get_wordsize());
	HexFile *hf;

	try {
		/* Read data from the device into the buffer */
		device->read(buf);
		printf("\n");
	} catch(std::exception& e) {
		printf("\n");
		fprintf(stderr, "%s: %s: %s\n", argv0, device->get_name().c_str(), e.what());
		return 1;
	}

	try {
		/* Open the hex file */
		hf = new HexFile_ihx8(argv[1]);
	} catch(std::exception& e) {
		fprintf(stderr, "%s: %s\n", argv[1], e.what());
		return 1;
	}

	try {
		/* Get the device memory map so we know what parts of the buffer
		 * are valid and save those parts to the hex file. */
		IntPairVector mmap = device->get_mmap();
		IntPairVector::iterator n = mmap.begin();
		for(; n != mmap.end(); n++)
			hf->write(buf, n->first, n->second);
	} catch(std::exception& e) {
		delete hf;
		fprintf(stderr, "%s: %s\n", argv[0], e.what());
		return 1;
	}
	delete hf;

	return 0;
}


static int do_verify(int argc, char *argv[]) {
	if(argc < 2) {
		fprintf(stderr, "\n%s: %s requires a filename.\n", argv0, argv[0]);
		fprintf(stderr,
			"Usage:\n  %s [options] chiptype %s filename.hex\n\n",
			  argv0, argv[0]);
		return 1;
	}

	DataBuffer buf(device->get_wordsize());

	try {
		/* Read data from the file into the buffer */
		HexFile *hf = HexFile::load(argv[1]);
		hf->read(buf);
		delete hf;
	} catch(std::exception& e) {
		fprintf(stderr, "%s: %s\n", argv[1], e.what());
		return 1;
	}

	try {
		/* Verify the contents of the device */
		device->read(buf, true);
		printf("\n");
	} catch(std::exception& e) {
		printf("\n");
		fprintf(stderr, "%s\n", e.what());
		return 1;
	}

	printf("Verification succeeded.\n");
	return 0;
}


static int do_check(int argc, char *argv[]) {
	/* The check has already been done so just return */
	return 0;
}


static int do_erase(int argc, char *argv[]) {
	try {
		device->erase();
	} catch(std::exception& e) {
		fprintf(stderr, "\n%s: %s: %s\n", argv0, device->get_name().c_str(), e.what());
		return 1;
	}
	return 0;
}


static int do_blankcheck(int argc, char *argv[]) {
	DataBuffer buf(device->get_wordsize());

	device->set_config_default(buf);

	printf("Verifying a device is blank.\n");
	try {
		/* Verify the contents of the device */
		device->read(buf, true);
		printf("\n");
	} catch(std::exception& e) {
		printf("\n");
		fprintf(stderr, "%s\nDevice is not blank.\n", e.what());
		return 1;
	}
	printf("Device is blank.\n");
	return 0;
}


void print_testmode_help(void) {
	printf(
"Commands:   q = quit   h = Help\n"
"            <action> <signal [signal ...] [action signal ...]\n"
"            t<h|l><shift-clock-time>     set shifting clock high/low time\n"
"            o<value>[*][/<numbits>]      shift out (numbits of) value\n"
"            i[q]<numbits>                shift in value\n"
"<action>s:  + = set    - = unset\n"
"<signal>s:  p = power  v = vpp  c = clk   d = data (out)\n");
}


void print_testmode_examples(void) {
	printf("\n"
"Shift data is always shifted LSB first even though it is written\n"
"MS nybble/triplet/bit first.  The length defaults to the number of bits\n"
"in the nybbles/triplets/bits specified (must be specified for decimal).\n"
"Leading `0' of `0x' (hex), `0b' (binary) may be omitted.  You may also\n"
"use `o' or `0o' for octal as well as the usual leading-zero notation.\n"
"For example:\n"
"  +pv-d   sets the Pwr and Vpp signals, unsets DataOut and\n"
"          leaves Clk unchanged\n"
"  th1000  sets the high clock time (data setup) when shifting to 1us\n"
"  tl400   sets the low clock time (= shift out hold) to 400ns\n"
"  o0x74   shifts out 8 bits (lsb first)  0 0 1 0  1 1 1 0\n"
"  ox5*/32 shifts out 32 bits (lsb first)  1 0 1 0  1 0 1 0 .... 1 0 1 0\n"
"  oxc/8   shifts out 8 bits  0 0 1 1  0 0 0 0\n"
"  o000    shifts out 12 bits\n"
"  o70/400 shifts out 400 bits  0 1 1 0  0 0 1 0  0 ...\n"
"  ob1000  shifts out 4 bits  0 0 0 1\n"
"  i40     shifts in 40 bits and prints in hex, octal, binary, decimal\n"
"  iq10000 shifts in 10000 bits and does not print them at all\n");
}


static void test_shift_clocks(const char *buffer, nanotime_t *tl,
  nanotime_t *th, int *next_i)
{
	const char *p = &(buffer[*next_i]);
	unsigned long v;
	enum { set_th, set_tl } which;
	char *endp;

	p = &(buffer[*next_i]);
	switch(*(p+1)) {
	case 'h':
		which = set_th;
		break;
	case 'l':
		which = set_tl;
		break;
	case '\0':
		printf("No time specifier given. Should be either 'h' or 'l'\n");
		return;
	default:
		printf("Invalid time specifier '%c', can only be 'h' or 'l'\n", *(p+1));
		return;
	}

	/* Parse the new value */
	v = strtoul(p+2, &endp, 0);
	if(endp == p) {
		printf("Invalid number `%s' following t[h][l] time-setting command\n",
		  p);
	} else {
		switch(which) {
		case set_th:
			printf("Setting th to %lu\n", v);
			*th = v;
			break;
		case set_tl:
			printf("Setting tl to %lu\n", v);
			*tl = v;
			break;
		}
	}
	*next_i = endp - buffer - 1;
}


static void test_shift_out(IO *io, const char *buffer, nanotime_t tl,
  nanotime_t th, int *next_i)
{
	static const char digits[]= "0123456789abcdef";
	const char *p;
	int skipped_zero=0, base, bits_per_digit, numbits;
	bool repeat=false;

	p = &(buffer[(*next_i) + 1]);
	if(*p == '0') {
		skipped_zero=1;
		p++;
	}

	switch(*p) {
	case 'd':
		bits_per_digit = -1; base = 10;
		break;
	case 'x':
		bits_per_digit = 4; base = 16;
		break;
	case 'o':
		bits_per_digit = 3; base =  8;
		break;
	case 'b':
		bits_per_digit = 1; base = 2;
		break;
	default:
		if(skipped_zero) {
			bits_per_digit = 3; base =  8;
		} else {
			bits_per_digit = -1; base = 10;
		}
		break;
	}
	p++;

	const char *p1 = p;
	for (;;) {
		if (!*p) break;
		if (!memchr(digits,*p,base)) break;
		p++;
	}
	if (p==p1) {
		printf("Invalid shift out: no digits\n");
		*next_i = p - buffer;
		return;
	}
	const char *pend = p;
	if (*p=='*') {
		repeat = true;
		p++;
	}
	if (*p=='/') {
		char *ep;
		numbits = strtoul(p+1,&ep,0);
		if (ep==p+1) {
			printf("Invalid number of bits `%s'\n", p+1);
			*next_i = p + 1 - buffer;
			return;
		}
		*next_i = ep - buffer - 1;
	} else {
		*next_i = p - buffer - 1;
		if (bits_per_digit == -1) {
			printf("Must specify a number of bits to\n"
			       "shift out a base 10 value.\n");
			return;
		}
		numbits = (pend - p) * bits_per_digit;
	}
	if (bits_per_digit == -1) {
		if (repeat) {
			printf("Repeat not possible with decimal.\n");
			return;
		}
		unsigned long v = strtoul(p,0,10);
		io->shift_bits_out(v, numbits, th, tl);
	} else {
		p = p1;
		if (!repeat) p1 = "0-";
		while (numbits > 0) {
			const char *ch = !*p ? 0 :
				(const char*)memchr(digits,*p,base);
			if (!ch) { p = p1; continue; }
			io->shift_bits_out(ch - digits, bits_per_digit, th, tl);
			p++;
			numbits -= bits_per_digit;
		}
	}
}


static void test_shift_in(IO *io, const char *buffer, nanotime_t tl,
  nanotime_t th, int *next_i)
{
	char *ep;
	bool quiet = false;

	(*next_i)++;
	if (buffer[*next_i] == 'q') {
		quiet = true;
		(*next_i)++;
	}
	int numbits = strtoul(buffer + *next_i, &ep, 0);
	if (ep == buffer + *next_i) {
		printf("Invalid number of bits to shift in.\n");
		return;
	}
	*next_i = ep - buffer - 1;
	if (quiet) {
		io->shift_bits_in(numbits, tl, th);
	} else {
		while (numbits > 0) {
			int now = numbits > 32 ? 32 : numbits;
			unsigned long v = io->shift_bits_in(now, tl, th);
			printf(" %*s0x%0*lx %*s0o%0*lo ",
			       (8-(now+3)/4), "", (now+3)/4, v,
			       (11-(now+2)/3), "", (now+2)/3, v);
			for (int i=32; i>=now; i--)
				if (i>=now)
					printf(" ");
			printf(" 0b");
			for (int i=now-1; i>=0; i--)
				printf("%ld", (v >> i) & 1LU);
			printf(" %11lu\n", v);
			numbits -= now;
		}
	}
}


static int do_test(int argc, char *argv[]) {
	bool get_out;
	char buffer[100];
	enum { undefined, set_signal, unset_signal } action;
	nanotime_t tl, th;
	int i;

	tl = 20;
	th = 20;

	printf("\nProgrammer board test mode. Make sure you didn't " \
	  "leave a PIC in the programmer.\n\n");
	print_testmode_help();

	get_out = false;
	while(! get_out) {
		printf("\n\n");
		printf("Current Status:\n");
		try {
			printf("Pwr     = %d    Vpp    = %d    Clk = %d    LVP = %d\n",
			  (int)io->get_pwr(), (int)io->get_vpp(), (int)io->get_clk(),
			  (int)io->get_lvp());
			printf("DataOut = %d    DataIn = %d\n",
			  (int)io->get_data(), (int)io->read_data());
		} catch(std::exception& e) {
			printf("\nError determining status:\n  %s\n\n", e.what());
		}
		printf("th = %luns  tl = %luns\n\n", th, tl);
		printf(" %% ");
		fflush(stdout);

		if(fgets(buffer, sizeof(buffer), stdin) == NULL)
			break;

		action = undefined;
		for(i = 0; buffer[i] != 0; i++) {
			switch(buffer[i]) {
			case '\n':
			case '\r':
				break;
			case 'q':
			case 'Q':
				get_out = true;
				printf("\n");
				goto next_line;
			case ' ':
				break;
			case 'h':
			case 'H':
			case '?':
				print_testmode_help();
				print_testmode_examples();
				goto next_line;
			case '+':
				action = set_signal;
				break;
			case '-':
				action = unset_signal;
				break;
			case 't':
				test_shift_clocks(buffer, &tl, &th, &i);
				break;
			case 'o':
				test_shift_out(io, buffer, tl, th, &i);
				break;
			case 'i':
				test_shift_in(io,buffer,tl,th,&i);
				break;
			case 'p':
			case 'P':
			case 'v':
			case 'V':
			case 'c':
			case 'C':
			case 'd':
			case 'D':
			case 'l':
			case 'L':
				if(action == undefined) {
					printf("Undefined action (set/unset) for %c\n",
					  toupper(buffer[i]));
					break;
				}
				try {
					switch(toupper(buffer[i])) {
					case 'P':
						io->set_pwr(action == set_signal);
						break;
					case 'V':
						io->set_vpp (action == set_signal);
						break;
					case 'C':
						io->set_clk (action == set_signal);
						break;
					case 'D':
						io->set_data(action == set_signal);
						break;
					case 'L':
						io->set_lvp(action == set_signal);
						break;
					}
				} catch(std::exception& e) {
					printf("\nError processing command:\n  %s\n\n", e.what());
				}
				break;
			default:
				printf("Unknown signal '%c'\n", toupper(buffer[i]));
				goto next_line;
			}
		}
next_line:;
	}

	io->off();
	return 0;
}
