#ifndef _HOST_H_
#define _HOST_H_

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

#include <avr/io.h>

/*!
 * \file host.h
 * \brief Platform header file.
 */

/*!
 * \addtogroup xgHost
 */
/*@{*/


/*! \brief Executor version string. */
#define XSVFEXEC_VERSION    "1.0.1"

/* Uncomment to enable debug output */
//#define XSVF_DEBUG

/* Hardware specific routines */
#define PIN(x)            (1 << (x))
#define SETPIN(addr,x)    (addr |= PIN(x))
#define CLEARPIN(addr,x)  (addr &= ~PIN(x))

#define JTAG_PORT_INIT		DDRB
#define JTAG_PORT_WRITE		PORTB
#define JTAG_PORT_READ		PINB

#define	TCK			7
#define TMS			0
#define TDI			5
#define TDO			6
#define PE			4

#define SET_TMS()		SETPIN(JTAG_PORT_WRITE, TMS)
#define CLR_TMS()		CLEARPIN(JTAG_PORT_WRITE, TMS)
#define SET_TDI()		SETPIN(JTAG_PORT_WRITE, TDI)
#define CLR_TDI()		CLEARPIN(JTAG_PORT_WRITE, TDI)
#define SET_TCK()		SETPIN(JTAG_PORT_WRITE, TCK)
#define CLR_TCK()		CLEARPIN(JTAG_PORT_WRITE, TCK)
#define SET_PE()		SETPIN(JTAG_PORT_WRITE, PE)
#define CLR_PE()		CLEARPIN(JTAG_PORT_WRITE, PE)

#define GET_TDO()   (JTAG_PORT_READ & PIN(TDO))

/*! \brief Set TMS high and toggle TCK. */
#define SET_TMS_TCK()   { SET_TMS(); CLR_TCK(); SET_TCK(); }

/*! \brief Set TMS low and toggle TCK. */
#define CLR_TMS_TCK()   { CLR_TMS(); CLR_TCK(); SET_TCK(); }

extern void XsvfInitHost(void);
extern void XsvfCloseHost(void);
extern void XsvfSetBuf(char *newBuf, int size);
extern int XsvfGetError(void);
extern unsigned char XsvfGetCmd(void);
extern unsigned char XsvfGetState(unsigned char state0, unsigned char state1);
extern unsigned char XsvfGetByte(void);
extern short XsvfGetShort(void);
extern long XsvfGetLong(void);
extern int XsvfReadBitString(void *buf, int num);
extern int XsvfSkipComment(void);

extern void XsvfDelay(long usecs);

/*@}*/


#endif
