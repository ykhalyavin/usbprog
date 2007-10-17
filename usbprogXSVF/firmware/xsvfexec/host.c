/*
 * modified by Sven Luetkemeier sven@sl-ware.de, 2007
 *
 * Copyright (C) 2004 by egnite Software GmbH. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY EGNITE SOFTWARE GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL EGNITE
 * SOFTWARE GMBH OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * For additional information see http://www.ethernut.de/
 */

/*
 * $Log$
 */

#include <stdio.h>
#include <string.h>

#include "xsvf.h"
#include "tapsm.h"
#include "host.h"

#include "../wait.h"

/*!
 * \file host.c
 * \brief Platform dependant routines..
 */

/*!
 * \addtogroup xgHost
 */
/*@{*/


#ifdef XSVF_DEBUG

/*!
 * \brief XSVF command names.
 *
 * Used for debugging output.
 */
static char *cmd_names[] = {
    "XCOMPLETE",
    "XTDOMASK",
    "XSIR",
    "XSDR",
    "XRUNTEST",
    "UNKNOWN",
    "UNKNOWN",
    "XREPEAT",
    "XSDRSIZE",
    "XSDRTDO",
    "XSETSDRMASKS",
    "XSDRINC",
    "XSDRB",
    "XSDRC",
    "XSDRE",
    "XSDRTDOB",
    "XSDRTDOC",
    "XSDRTDOE",
    "XSTATE",
    "XENDIR",
    "XENDDR",
    "XSIR2",
    "XCOMMENT",
    "XWAIT",
    "UNKNOWN"
};

/*!
 * \brief TAP state names.
 *
 * Used for debugging output.
 */
static char *tap_names[] = {
    "Test-Logic-Reset",
    "Run-Test-Idle",
    "Select-DR-Scan",
    "Capture-DR",
    "Shift-DR",
    "Exit1-DR",
    "Pause-DR",
    "Exit2-DR",
    "Update-DR",
    "Select-IR-Scan",
    "Capute-IR",
    "Shift-IR",
    "Exit1-IR",
    "Pause-IR",
    "Exit2-IR",
    "Update-IR",
    "Unknown"
};

#endif

#if 0

/*!
 * \brief Handle of XSVF file.
 */
static int fh;

#endif

/*!
 * \brief Last error occured in this module.
 */
static int xsvf_err = 0;

/*!
 * \brief Buffer containing XSVF instruction, pointer to current position
 */
static const char *buf;

/*!
 * \brief Buffer end pointer, points to the first address behind the buffer
 */
static const char *bufEnd;

/*!
 * \brief Initialize the platform dependant interface.
 *
 * All required hardware initializations should be done in this
 * routine. We may also initiate debug output. If the XSVF date
 * is located in a file system, then the file will be opened
 * here as well.
 */
void XsvfInitHost(void) {
	// use as output
	JTAG_PORT_INIT |= (1<<TCK)|(1<<TMS)|(1<<TDI);
	// use as input
	JTAG_PORT_INIT &=~(1<<TDO);
	// pullup
	JTAG_PORT_WRITE |= (1<<TDO);
	// clear pin states
	CLR_TCK();
	CLR_TDI();
	CLR_TMS();
}

/*!
 * \brief Set buffer for new XSVF instruction
 */
void XsvfSetBuf(char *newBuf, int size) {
	buf = newBuf;
	bufEnd = buf + size;
}

/*!
 * \brief Retrieve the last error occured in this module.
 *
 * \return Error code or 0 if no error occured.
 */
int XsvfGetError(void)
{
    return xsvf_err;
}

/*!
 * \brief Get next byte from XSVF buffer.
 *
 * Call XsvfGetError() to check for errors,
 *
 * \return Byte value.
 */
unsigned char XsvfGetByte(void)
{
	unsigned char rc;
	const char *newbuf = buf + sizeof(unsigned char);
	if(newbuf > bufEnd) {
		xsvf_err = XE_DATAUNDERFLOW;
		return 0;
	} else {
		rc = *((unsigned char*) &buf[0]);
		buf = newbuf;
    }
   	return rc;
}

/*!
 * \brief Get next command byte from XSVF buffer.
 *
 * \return XSVF command or XUNKNOWN if an error occured.
 */
unsigned char XsvfGetCmd(void)
{
	unsigned char rc;
	const char *newbuf = buf + sizeof(unsigned char);
	if(newbuf > bufEnd) {
		rc = XUNKNOWN;
	} else {
		rc = *((unsigned char*) &buf[0]);
		buf = newbuf;
		if(rc >= XUNKNOWN) {
			rc = XUNKNOWN;
		}
	}
	return rc;
}

/*!
 * \brief Get next byte from XSVF buffer and select a TAP state.
 *
 * \param state0 Returned state, if the byte value is zero.
 * \param state1 Returned state, if the byte value is one.
 * 
 * \return TAP state or UNKNOWN_STATE if an error occured.
 */
unsigned char XsvfGetState(unsigned char state0, unsigned char state1)
{
	unsigned char rc;
	const char *newbuf = buf + sizeof(unsigned char);
	if(newbuf > bufEnd) {
		rc = UNKNOWN_STATE;
	} else {
		rc = *((unsigned char*) &buf[0]);
		buf = newbuf;
		if(rc > 1) {
			rc = UNKNOWN_STATE;
		} else if(rc) {
			return state1;
		} else {
			return state0;
		}
	}
	return rc;
}

/*!
 * \brief Get next short value from XSVF buffer.
 *
 * Call XsvfGetError() to check for errors,
 *
 * \return Short value.
 */
short XsvfGetShort(void)
{
	short rc;
	const char *newbuf = buf + sizeof(short);
	if(newbuf > bufEnd) {
		xsvf_err = XE_DATAUNDERFLOW;
		return -1;
	} else {
		rc = *((short*) &buf[0]);
		/* convert to little endian */
		rc = (rc >> 8) | (rc << 8);
		buf = newbuf;
    	return rc;
    }
}

/*!
 * \brief Get next long value from XSVF buffer.
 *
 * Call XsvfGetError() to check for errors,
 *
 * \return Long value.
 */
long XsvfGetLong(void)
{
	long rc;
	const char *newbuf = buf + sizeof(long);
	if(newbuf > bufEnd) {
		xsvf_err = XE_DATAUNDERFLOW;
		return 0;
	} else {
		rc = *((long*) &buf[0]);
		/* convert to little endian */
		rc = ((rc & 0x000000ff) << 24) |
		     ((rc & 0x0000ff00) << 8)  |
		     ((rc & 0x00ff0000) >> 8)  |
		     ((rc & 0xff000000) >> 24);
		buf = newbuf;
    	return rc;
    }
}

/*!
 * \brief Read a specified number of bits from XSVF buffer.
 *
 * \param buf Pointer to the buffer which receives the bit string.
 * \param num Number of bits to read.
 *
 * \return Error code or 0 if no error occured.
 */
int XsvfReadBitString(void *bitstr, int num)
{
    int len = (num + 7) / 8;

    if (len > MAX_BITVEC_BYTES) {
        xsvf_err = len = XE_DATAOVERFLOW;
    } else if (buf + len > bufEnd) {
    	xsvf_err = len = XE_DATAUNDERFLOW;
    } else {
    	memcpy(bitstr, buf, len);
    	buf += len;
    }

    return len;
}

/*!
 * \brief Skip comment in the XSVF buffer.
 *
 * \return Error code or 0 if no error occured.
 */
int XsvfSkipComment(void)
{
	for(;;) {
		if(buf >= bufEnd) {
			return (xsvf_err = XE_DATAUNDERFLOW);
		}
		if(*(buf++) == 0) {
			break;
		}
	}
    
    return 0;
}

/*!
 * \brief Microsecond delay.
 *
 * \param usecs Number of microseconds.
 */
void XsvfDelay(long usecs)
{
	wait_ms((usecs + 500U) / 1000U);
}

/*@}*/
