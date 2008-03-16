/*
 * Copyright (C) 2007 Sven Luetkemeier 
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
 */
//#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "../firmware/xsvfexec/xsvf.h"
#include "../firmware/defines.h"

#include "xsvfprog.h"

#define BITS_TO_BYTES(b)	(((b) + 7) / 8)

#define BUFSIZE		64

FILE *file;
#ifdef DEBUG
FILE *debugfile;
#endif

char buf[BUFSIZE];
int bufsize = 0;
int quit = 0;

int drSize = 0;
int drSize2 = 0;
int irSize;

int filesize;
int filepos = 0;

char ch;

int ret;

struct xsvfprog* xsvfprog;

/* This function has been copied from Ethernut's XSVF executor code.
 * For license and additional information see ../firmware/xsvfexec/xsvfexec.c
 * and http://www.ethernut.de/
 */
static int BitStringOnes(int len, unsigned char *op)
{
    int rc = 0;
    unsigned char mask;
    int i = (int)((len + 7UL) / 8UL);

    while (i--) {
        for(mask = op[i]; mask; mask >>= 1) {
            rc += mask & 1;            
        }
    }
    return rc;
}

void exit_err(char *msg) {
	fprintf(stderr, msg);
	fclose(file);
#ifdef DEBUG
	fclose(debugfile);
#endif
	xsvfprog_close(xsvfprog);
	exit(1);
}

void file2buf(int size) {
	if(bufsize + size > BUFSIZE) {
		exit_err("Instruction too long.\n");
	}
	fread(&buf[bufsize], 1, size, file);
	bufsize += size;
}

int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("Usage: xsvfplayer <filename>\n");
		return 1;
	}
	
	if(NULL == (file = fopen(argv[1], "r"))) {
		printf("Cannot open file.\n");
		return 1;
	}
	fseek(file, 0, SEEK_END);
	filesize = ftell(file);
	rewind(file);
	
	if(NULL == (xsvfprog = xsvfprog_open())) {
		printf("Cannot connect to XSVF player.\n");
		return 1;
	}
	if(SUCCESS != xsvfprog_init(xsvfprog)) {
		printf("Initialization of XSVF player failed.\n");
		return 1;
	}

#ifdef DEBUG
	debugfile = fopen("debug.xsvf", "w");
#endif
	printf("Programming...");
	while(!feof(file) && !quit) {
		bufsize = 0;
		/* get command */
		file2buf(1);
#ifdef DEBUG
		printf("instr 0x%02x: ", buf[0]);
#endif
		/* read additional command parameters */
		switch(buf[0]) {
		
		case XCOMPLETE:
			/* quit if XCOMPLETE command reached */
			quit = 1;
			break;
		
		case XREPEAT:
		case XSTATE:
		case XENDIR:
		case XENDDR:
			/* these commands need one additional byte */
			file2buf(1);
			break;
		
		case XRUNTEST:
			/* this command needs four additional bytes */
			file2buf(4);
			break;
		
		case XTDOMASK:
		case XSDR:
		case XSDRB:
		case XSDRC:
		case XSDRE:
			/* these commands need drSize additional bits */
			file2buf(BITS_TO_BYTES(drSize));
			break;
		
		case XSDRTDO:
		case XSDRTDOB:
		case XSDRTDOC:
		case XSDRTDOE:
			/* these commands need 2x drSize additional bits */
			file2buf(2 * BITS_TO_BYTES(drSize));
			break;
		
		case XSDRSIZE:
			file2buf(4);
			drSize = ntohl(*((long*) &buf[1]));
#ifdef DEBUG
			printf("drSize = %d, ", drSize);
#endif
			if (BITS_TO_BYTES(drSize) > MAX_BITVEC_BYTES) {
				exit_err("\nData Register too long.\n");
			}
			break;
		
		case XSIR:
			file2buf(1);
			irSize = *((unsigned char*) &buf[1]);
#ifdef DEBUG
			printf("irSize = %d, ", irSize);
#endif
			if (BITS_TO_BYTES(irSize) > MAX_BITVEC_BYTES) {
				exit_err("\nInstruction Register too long.\n");
			}
			file2buf(BITS_TO_BYTES(irSize));
			break;
		
		case XSIR2:
			file2buf(2);
			irSize = ntohs(*((short*) &buf[1]));
#ifdef DEBUG
			printf("irSize = %d, ", irSize);
#endif
			if (BITS_TO_BYTES(irSize) > MAX_BITVEC_BYTES) {
				exit_err("\nInstruction Register too long.\n");
			}
			file2buf(BITS_TO_BYTES(irSize));
			break;
		
		case XSETSDRMASKS:
			file2buf(2 * BITS_TO_BYTES(drSize));
			drSize2 = BitStringOnes(drSize, &buf[1 + BITS_TO_BYTES(drSize)]);
#ifdef DEBUG
			printf("drSize2 = %d, ", drSize2);
#endif
			if (BITS_TO_BYTES(drSize2) > MAX_BITVEC_BYTES) {
				exit_err("\nData Register2 too long.\n");
			}
			break;
		
		case XSDRINC:
			file2buf(BITS_TO_BYTES(drSize) + 1);
			int num = *((unsigned char*) &buf[1 + BITS_TO_BYTES(drSize)]);
			file2buf(num * BITS_TO_BYTES(drSize2));
			break;
		
		case XCOMMENT:
			do {
				ch = fgetc(file);
			} while(ch != 0 && ch != EOF);
			/* we don't send comments to the programmer... */
			continue;
			break;
		
		default:
			exit_err("\nIllegal XSVF command.\n");
			break;
		
		}
		
#ifdef DEBUG
		printf("%d bytes long\n", bufsize);
		fwrite(buf, 1, bufsize, debugfile);
#endif
		
		ret = xsvfprog_exec(xsvfprog, buf, bufsize);
#ifdef DEBUG
		//printf("%d\n", ret);
#endif

		switch(ret) {
		
		case 0:
			/* success, do nothing, just go on */
			break;
		
		case UNKNOWN_COMMAND:
			exit_err("\nProgramming error: Unknown command.\n");
			break;
		
		case XE_TDOMISMATCH:
			exit_err("\nProgramming error: Captured TDO value differs from expected TDO value.\n");
			break;
		
		case XE_ILLEGALCMD:
			exit_err("\nProgramming error: Illegal XSVF command.\n");
			break;
		
		case XE_ILLEGALSTATE:
			exit_err("\nProgramming error: Illegal TAP state.\n");
			break;
		
		case XE_DATAOVERFLOW:
			exit_err("\nProgramming error: Bit string overflow.\n");
			break;
		
		case XE_DATAUNDERFLOW:
			exit_err("\nProgramming error: End of command buffer reached expecting more data.\n");
			break;
		
		default:
			exit_err("\nUnknown programming error.\n");
			break;
		
		}

		filepos += bufsize;
		printf("\rProgramming... %3u%%", 100*filepos/filesize);
		fflush(stdout);
	}
	printf("\nDone.\n");
	
	fclose(file);
#ifdef DEBUG
	fclose(debugfile);
#endif
	xsvfprog_close(xsvfprog);
}
