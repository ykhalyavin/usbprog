#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "cmd.h"
#include "cmd_tap_jtag.h"
#include "cmd_flash.h"

#define COMMAND_DETECT						1
#define COMMAND_READ						2
#define COMMAND_BLINK						3
#define COMMAND_PROG						4
#define COMMAND_TEST						5

#define BUFFER_SIZE							512


char g_file_name[256] = ""; //".\\flash.bin";
int  g_size = -1;
int  g_address = 0;
int  g_verify = 0;
int  g_command = COMMAND_BLINK;
FLASH_STR * g_flash = NULL;

void welcome(void);
void help(void);
int parse_options(int argc, char* argv[]);
int option_l(char* param);
int option_s(char* param);
int option_f(char* param);
int parse_command(int argc, char* argv[]);
int check(void);
int detect(void);
int read(void);
int program(void);
int test(void);
int blink(void);
void swap(char* buf, int num);

#pragma argsused
int main(int argc, char* argv[])
{
	int r, ver;
	U8 state;

	welcome();
	if (argc == 1) {
		help();
		return STATUS_OK;
	}

	if (parse_options(argc, argv) != STATUS_OK)
		return STATUS_ERROR;
	if (parse_command(argc, argv) != STATUS_OK)
		return STATUS_ERROR;
	if (check() != STATUS_OK)
		return STATUS_ERROR;

	/* Open the USBProg */
	if (usbprog_open() != USBPROG_STATUS_OK)
		return STATUS_ERROR;
	if (cmd_comm_firmware_version(&ver) != CMD_STATUS_OK) {
		puts("Could not get the firmware version.");
		return STATUS_ERROR;
    }
	printf("Firmware version is %d.\n", ver);

	//test();

	/* Execute */
	cmd_comm_init();
	cmd_tap_set_srst(0);
	if (detect() != STATUS_OK)
		return STATUS_ERROR;
	switch (g_command) {
	case COMMAND_BLINK:
		blink();
		break;
	case COMMAND_DETECT:
		/* Have done before */
		break;
	case COMMAND_READ:
		read();
		break;
	case COMMAND_PROG:
		program();
		break;
	case COMMAND_TEST:
		test();
		break;
	default:
		break;
	}
	cmd_tap_reset(100);
	cmd_tap_set_srst(0);

	usbprog_close();
	return 0;
}

void welcome(void)
{
	puts("USBProg AVR32 programmer\n");
}

void help(void)
{
	puts(
		"Usage: avr32prog [options] command\n"\
		"command:\n"\
		"  detect  detect some target information\n"\
		/*"  id      read target's idcode\n"\*/
		/*"  erase   erase the flash chip\n"\*/
		"  prog    program the flash\n"\
		"  read    read from the flash\n"\
		/*"  verify  program the flash chip\n"\*/
		"  help    show this help\n"\
		"options:\n"\
		/*"  -l [start btye address in decimal]  default=0\n"\*/
		"  -s [size in decimal]\n"\
		"  -f [binary_file_name]"\
		/*"  -v  verify\n"\*/
	);
}

int parse_options(int argc, char* argv[])
{
	int i = 0;
	char *str;

	for (i = 1; i < argc; i++) {
		str = argv[i];
		if ( *str == '-' ) {
			switch ( *(++str) ) {
			case 'l':
				if (option_l(argv[++i]) != STATUS_OK)
					return STATUS_ERROR;
				break;
			case 's':
				if (option_s(argv[++i]) != STATUS_OK)
					return STATUS_ERROR;
				break;
			case 'f':
				if (option_f(argv[++i]) != STATUS_OK)
					return STATUS_ERROR;
				break;
			case 'v':
				g_verify = 1;
				break;
			default:
				printf("Error: invalid option \'%s\'!", argv[i]);
				return STATUS_ERROR;
			}
			*argv[i] = '-';	// 让options的参数第一字符变为'-'以防函数parse_command再作处理
		}
	}

	return STATUS_OK;
}

int option_f(char* param)
{
	if ( (param == NULL) || (strlen(param) == 0) ) {
		puts("Error: no parameter with option \'-f\'!");
		return STATUS_INVALID_PARAM;
	}
	if (strlen(param) > 256) {
		puts("Error: parameter of option \'-f\' is too long!");
		return STATUS_INVALID_PARAM;
	}

	strcpy(g_file_name, param);

	return STATUS_OK;
}

int option_l(char* param)
{
	char *endptr;
	U32  addr;

	if ( (param == NULL) || (strlen(param) == 0) ) {
		puts("Error: no parameter with option \'-l\'!");
		return STATUS_INVALID_PARAM;
	}

	addr = (U32)strtod(param, &endptr);
	if ( *endptr != '\0' ) {
		puts("Error: parameter of option \'-l\' should be a decimal integral number!");
		return STATUS_INVALID_PARAM;
	}
	if ( addr <= 0 ) {
		puts("Error: address should be positive!");
		return STATUS_INVALID_PARAM;
	}
	if ( (addr % 2) != 0 ) {
		puts("Error: address should be 2 bytes aligned!");
		return STATUS_INVALID_PARAM;
	}
	g_address = addr;

	return STATUS_OK;
}

int option_s(char* param)
{
	char *endptr;
	U32  size;

	if ( (param == NULL) || (strlen(param) == 0) ) {
		puts("Error: no parameter with option \'-s\'!");
		return STATUS_INVALID_PARAM;
	}

	size = (U32)strtod(param, &endptr);
	if ( *endptr != '\0' ) {
		puts("Error: parameter of option \'-s\' should be a decimal integral number!");
		return STATUS_INVALID_PARAM;
	}
	if ( size <= 0 ) {
		puts("Error: size should be positive!");
		return STATUS_INVALID_PARAM;
	}
	if ( (size % 2) != 0 ) {
		puts("Error: size should be 2 bytes aligned!");
		return STATUS_INVALID_PARAM;
	}
	g_size = size;

	return STATUS_OK;
}

int parse_command(int argc, char* argv[])
{
	int r;
	char* cmdl = NULL;

	for (r = 1; r < argc; r++) {
		cmdl = argv[r];
		if ( *cmdl != '-' )		// if not options then command
			break;
	}

	if ( cmdl == NULL ) {
		puts("There is not any command, program exited.");
		return STATUS_ERROR;
	}

	for (r = 0; r < (int)strlen(cmdl); r++)
		cmdl[r] = (char)tolower(cmdl[r]);

	if ( !strcmp(cmdl, "help") ) {
		help();
		r = STATUS_ERROR;		// let the programm exit
	} else {
		r = STATUS_OK;
		if ( !strcmp(cmdl, "blink") ) {
			g_command = COMMAND_BLINK;
		} else if ( !strcmp(cmdl, "detect") ) {
			g_command = COMMAND_DETECT;
		} else if ( !strcmp(cmdl, "read") ) {
			g_command = COMMAND_READ;
		} else if ( !strcmp(cmdl, "prog") ) {
			g_command = COMMAND_PROG;
		} else if ( !strcmp(cmdl, "test") ) {
			g_command = COMMAND_TEST;
		} else {
			printf("Error: invalid command \'%s\'!", cmdl);
			r = STATUS_ERROR;
		}
	}
/*
	if (r == STATUS_OK) {
		printf("    command: \'%s\'\n", cmdl);
		printf("       file: \'%s\'\n", g_file_name);
		printf("  file size: %d\n", g_file_size);
		printf("       size: %d\n", g_size);
	}
*/
	return r;
}

int check(void)
{
	return STATUS_OK;
}

int detect(void)
{
	int r, m, n;
	U32 id;

	printf("Detecting ...\n  ");
	r = cmd_tap_reset(10);
	r = cmd_jtag_detect_ir(256, &m);
	r = cmd_jtag_detect_dr(32, &n);
	if (n > 1) {
		puts("There are more than one targets on the JTAG chain!");
		return STATUS_ERROR;
	}
	if (m != 5) {
		puts("Target is not AVR32!");
		return STATUS_ERROR;
	}
	r = cmd_tap_reset(10);
	r = cmd_jtag_detect_dr(64, &n);
	if (n != 32) {
		puts("The target is not AVR32!");
		return STATUS_ERROR;
	}
	r = cmd_jtag_data(NULL, &id, 32);
	if ( IDCODE_LSB(id) != 1 ) {
		puts("Can not read target\'s IDCODE!");
		return STATUS_ERROR;
	}
	m = (id & (IDCODE_MANUFACTUER_MASK | IDCODE_PARTNUMBER_MASK));
	if (m != ID_AP7000) {
		puts("Target is not AVR32 AP7000!");
		return STATUS_ERROR;
	}
	printf("Found AVR32 AP7000, Rev.%d.\n  ", IDCODE_REVISION(id));

	/* Reading the flash chip id code */
	r = cmd_flash_id(&id);
	g_flash = cmd_flash_index(id);
	if (g_flash == NULL) {
		printf("Unsupported flash chip (ID 0x%08X)!\n", id);
		r = STATUS_ERROR;
	} else {
		printf("Found flash chip %s.\n", g_flash->type);
		r = STATUS_OK;
	}

	return r;
}

int read(void)
{
	char buf[BUFFER_SIZE];
	int i, k, r, size;
	FILE *file;
	time_t t;
	U16 data;

	if (g_size < 0)
		g_size = BUFFER_SIZE;

	if ( (file = fopen(g_file_name, "wb")) == NULL ) {
		puts("Error: cannot open file to write.");
		return STATUS_ERROR;
	}

	printf("Reading ");
	t = time(NULL);
	cmd_comm_led(1);
	i = 0;
	k = 0;
	while (i < g_size) {
		if ((k++ % 8) == 0)
			putchar('.');

		size = g_size - i;
		if (size > BUFFER_SIZE)
			size = BUFFER_SIZE;
		r = cmd_flash_read(g_address+i, size/2, (U16*)buf);
		if (r != STATUS_OK) {
			puts("[ERROR]\n");
			r = STATUS_ERROR;
			break;
		}
		swap(buf, size);
		fwrite(buf, 1, size, file);

		i += size;
	}
	cmd_comm_led(0);
	fclose(file);
	if (r == STATUS_OK) {
		printf("[DONE]\n  Read %d bytes.  Used %d seconds.\n", i, time(NULL) - t);
	}

	return r;
}

int blink(void)
{
	puts("This function is not implemented yet:)");
	return STATUS_OK;
}

int test(void)
{
	cmd_tap_set_srst(0);
	sleep(1000);
	cmd_tap_set_srst(1);

	return STATUS_OK;
}

int program(void)
{
	int r;
	time_t t;

	if (g_flash == NULL) {
		puts("Error: unknown flash chip!");
		return STATUS_ERROR;
	}

	FILE *file = fopen(g_file_name, "rb");
	if (file == NULL) {
		puts("Error: cannot open the file.");
		return STATUS_ERROR;
	}
	fseek(file, 0L, SEEK_END);
	int file_size = ftell(file);
	fseek(file, 0L, SEEK_SET);
	if (file_size == 0) {
		puts("Error: file size is zero!");
		return STATUS_ERROR;
	}
	if (g_size < 0) {
		g_size = file_size;
		printf("Note: not specify programming size,\n"
			   "      using the file size (%d) instead.\n", g_size);
	}
	if (file_size < g_size) {
		g_size = file_size;
		printf("Note: the file size small than the given size,\n"
			   "      using the file size (%d) instead.\n", g_size);
	}
	if (g_size > g_flash->size) {
		puts("Error: given size exceeds the size of the flash chip!");
		return STATUS_ERROR;
	}

	/* Confirm operating */

	/* Erase sector(s) */
	printf("Erasing sectors ");
	int group_index, count, index, size, addr;

	cmd_comm_led(1);
	t = time(NULL);
	group_index = 0;
	count = 0;
	index = 0;
	addr = 0;
	size = g_flash->group[group_index].size;
	while (addr < g_size) {
		putchar('.');
		/* Erase current sector */
		if ( (g_flash->id == FLASH_DEVICE_AT49BV6416) ||
			 (g_flash->id == FLASH_DEVICE_AT49BV6416T) ) {
			r = cmd_flash_unlock_sector((U32*)&addr, 1);
			//* Debug */r = CMD_STATUS_OK;
			if (r != CMD_STATUS_OK) {
				r = -1;
				break;
			}
		}
		r = cmd_flash_erase_sector((U32*)&addr, 1);
		//* Debug */r = CMD_STATUS_OK;
		if (r != CMD_STATUS_OK) {
			r = -2;
			break;
		}
		//* Debug */printf("\n  [%02d](%d-%02d) 0x%08X", count, group_index, index, addr);

		/* Verify erased sector */
		r = cmd_flash_erase_verify( addr, addr + size - 2);
		//* Debug */r = CMD_STATUS_OK;
		if (r != CMD_STATUS_OK) {
			r = -3;
			break;
		}

		/* Select next sector */
		addr += size;
		count++;
		index++;
		if (index >= g_flash->group[group_index].num) {
			if (++group_index >= g_flash->group_num) {
				r = -4;
				break;
			}
			index = 0;
			size = g_flash->group[group_index].size;
		}
	}
	cmd_comm_led(0);
	if (r == STATUS_OK) {
		printf("[DONE]\n  Erased %d sector(s).  Used %d seconds.\n", count, time(NULL) - t);
	} else {
		printf("[FAILED]\n  ");
		switch (r) {
		case -1:
			printf("Can not unlock the %dth sector!", count);
			break;
		case -2:
			printf("Can not erase the %dth sector!", count);
			break;
		case -3:
			printf("Erase the %dth sector error!", count);
			break;
		case -4:
			printf("Sector number overflow!");
			break;

		default:
			break;
		}
		puts(" ");
		return STATUS_ERROR;
	}

	/* Program */
	printf("Programming ");
	char buf[BUFFER_SIZE];

	t = time(NULL);
	cmd_comm_led(1);
	addr = 0;
	while (addr < g_size) {
		putchar('.');
		size = g_size - addr;
		if (size > BUFFER_SIZE)
			size = BUFFER_SIZE;
		count = fread(buf, 1, size, file);
		//* Debug */printf("\n  %06X: (size=%d, count=%d) ", addr, size, count);
		if (count != size) {
			r = -1;
			break;
		}
		swap(buf, size);
		//* Debug */printf("%04X %04X %04X %04X", ((U16*)buf)[0], ((U16*)buf)[1], ((U16*)buf)[2], ((U16*)buf)[3]);
		r = cmd_flash_program(addr, size/2, (U16*)buf);
		//* Debug */r = CMD_STATUS_OK;
		if (r != CMD_STATUS_OK) {
			r = -2;
			break;
		}

		addr += size;
	}
	cmd_comm_led(0);
	fclose(file);
	if (r == STATUS_OK) {
		printf("[DONE]\n  Programmed %d bytes.  Used %d seconds.\n", addr, time(NULL) - t);
	} else {
		switch (r) {
		case -1:
			printf("Read error at %d (size=%d, count=%d)!", addr, size, count);
			break;
		case -2:
			printf("Program error between %d to %d!", addr, addr+size);
			break;

		default:
			break;
		}
		puts(" ");
		return STATUS_ERROR;
	}
	if (g_verify == 0)
		return STATUS_OK;

	/* Verify */
	//TODO

	return STATUS_OK;
}

inline void swap(char* buf, int num)
{
	char temp;

	for (int i=0, next=1; i<num; i+=2, next+=2) {
		temp = buf[i];
		buf[i] = buf[next];
		buf[next] = temp;
	}
}

