/*
 * Copyright (C) 2006 Benedikt Sauter 
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
#include <stdint.h>
#include <avr/io.h>

#define PIN(x)            (1 << (x))
#define SETPIN(addr,x)    (addr |= PIN(x))
#define CLEARPIN(addr,x)  (addr &= ~PIN(x))

#define PIN1  	    PB5
#define PIN2	    PB4
#define PIN3	    PB3
#define PIN4	    PB0
#define PIN5	    PB2
#define PIN6	    PB7
#define PIN7	    PB1
#define PIN8	    PB6


#define TDI   PIN1
#define SRST  PIN2
#define TRST  PIN3
#define TMS   PIN4
#define TCK   PIN6
#define TDO   PIN8

#define PIN_DDR    DDRB
#define PIN_READ   PINB
#define PIN_WRITE   PORTB

// check if tdo == 1
#define IS_PIN1_SET()			    (PIN_READ & PIN(PIN1)) 
#define IS_PIN2_SET()			    (PIN_READ & PIN(PIN2))
#define IS_PIN3_SET()			    (PIN_READ & PIN(PIN3))
#define IS_PIN4_SET()			    (PIN_READ & PIN(PIN4))
#define IS_PIN5_SET()			    (PIN_READ & PIN(PIN5))
#define IS_PIN6_SET()			    (PIN_READ & PIN(PIN6))
#define IS_PIN7_SET()			    (PIN_READ & PIN(PIN7))
#define IS_PIN8_SET()			    (PIN_READ & PIN(PIN8))


#define SET_PIN1()			     SETPIN( PIN_WRITE, PIN1 )
#define SET_PIN2()			     SETPIN( PIN_WRITE, PIN2 )
#define SET_PIN3()			     SETPIN( PIN_WRITE, PIN3 )
#define SET_PIN4()			     SETPIN( PIN_WRITE, PIN4 )
#define SET_PIN5()			     SETPIN( PIN_WRITE, PIN5 )
#define SET_PIN6()			     SETPIN( PIN_WRITE, PIN6 )
#define SET_PIN7()			     SETPIN( PIN_WRITE, PIN7 )
#define SET_PIN8()			     SETPIN( PIN_WRITE, PIN8 )

#define CLEAR_PIN1()			     CLEARPIN( PIN_WRITE, PIN1 )
#define CLEAR_PIN2()			     CLEARPIN( PIN_WRITE, PIN2 )
#define CLEAR_PIN3()			     CLEARPIN( PIN_WRITE, PIN3 )
#define CLEAR_PIN4()			     CLEARPIN( PIN_WRITE, PIN4 )
#define CLEAR_PIN5()			     CLEARPIN( PIN_WRITE, PIN5 )
#define CLEAR_PIN6()			     CLEARPIN( PIN_WRITE, PIN6 )
#define CLEAR_PIN7()			     CLEARPIN( PIN_WRITE, PIN7 )
#define CLEAR_PIN8()			     CLEARPIN( PIN_WRITE, PIN8 )


void write_tdi(char * buf, uint16_t size);
void write_tms(uint8_t  buf);
void write_and_read(char * buf, uint16_t size);
void read_tdo(char * buf, uint16_t size);

void set_direction(uint8_t direction);
void set_port(uint8_t value);
uint8_t get_port();
void set_bit(uint8_t bit, uint8_t value);
uint8_t get_bit(uint8_t bit);


void tap_shift(char * buf, uint8_t size);
void tap_shift_final(char * buf, uint8_t size);

