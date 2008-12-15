/* jtag.h
* Copyright (C) 2007  Benedikt Sauter, sauter@ixbat.de
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
#ifndef _JTAG_H_
#define _JTAG_H_

#include <avr/io.h>
#include "bit.h"
#include "wait.h"

#define uint8_t	unsigned char

#define JTAG_IR_LENGTH		3

#define JTAG_PORT_INIT		DDRB
#define JTAG_PORT_WRITE		PORTB
#define JTAG_PORT_READ		PINB
/*
#define	TCK			0
#define TMS			7
#define TDI			5
#define TDO			6
*/
#define	TCK			5
#define TMS			0
#define TDI			6
#define TDO			4


// check if tdo == 1
#define JTAG_IS_TDO_SET()               (JTAG_PORT_READ & BIT(TDO))

// lowlevel
#define JTAG_SET_TCK()                       SETBIT( JTAG_PORT_WRITE, TCK )
#define JTAG_CLEAR_TCK()                     CLEARBIT( JTAG_PORT_WRITE, TCK )

#define JTAG_SET_TMS()                       SETBIT( JTAG_PORT_WRITE, TMS )
#define JTAG_CLEAR_TMS()                     CLEARBIT( JTAG_PORT_WRITE, TMS )

#define JTAG_SET_TDI()                       SETBIT( JTAG_PORT_WRITE, TDI )
#define JTAG_CLEAR_TDI()                     CLEARBIT( JTAG_PORT_WRITE, TDI )

// a jtag clock
#define JTAG_CLK()                      { JTAG_CLEAR_TCK(); asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");JTAG_SET_TCK(); }


// JTAG State Machine
typedef enum
{
	SELECT_DR_SCAN,
	CAPTURE_DR,
	SHIFT_DR,
	EXIT1_DR,
	PAUSE_DR,
	EXIT2_DR,
	UPDATE_DR,

	TEST_LOGIC_RESET,
	RUN_TEST_IDLE,

	SELECT_IR_SCAN,
	CAPTURE_IR,
	SHIFT_IR,
	EXIT1_IR,
	PAUSE_IR,
	EXIT2_IR,
	UPDATE_IR
} TAP_STATE;

// for openocd
void jtag_send_slice(uint8_t tck, uint8_t tms, uint8_t tdi);

// for openocd
uint8_t jtag_read_tdo(void);

// setup connection
void jtag_init(void);

// force jtag tap into reset state
int jtag_reset(void);

// goto a state
void jtag_goto_state(uint8_t state);

// write to target tdi
uint8_t jtag_read(uint8_t numberofbits, unsigned char * buf);

// read from target tdo
uint8_t jtag_write(uint8_t numberofbits, unsigned char * buf);

// write and read after every clock edge
uint8_t jtag_write_and_read(  uint8_t numberofbits,
                              unsigned char * buf,
	                            unsigned char * readbuf);


void jtag_clock_cycles(uint8_t num);


#endif /* _JTAG_H_ */
