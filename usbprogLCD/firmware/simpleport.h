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

#define BIT(x)            (1 << (x))
#define SETBIT(addr,x)    (addr |= BIT(x))
#define CLEARBIT(addr,x)  (addr &= ~BIT(x))
#define TOGGLEBIT(addr,x) (addr ^= BIT(x))

#define IO1	    PB5
#define IO2  	    PB4
#define IO3  	    PB3
#define IO4  	    PB0
#define IO5  	    PB2
#define IO6  	    PB7
#define IO7  	    PB1
#define IO8  	    PB6
#define IO9 	    PD0
#define IO10  	    PD1
#define IO11 	    PA4

#define IO1_DDR    DDRB
#define IO2_DDR    DDRB
#define IO3_DDR    DDRB
#define IO4_DDR    DDRB
#define IO5_DDR    DDRB
#define IO6_DDR    DDRB
#define IO7_DDR    DDRB
#define IO8_DDR    DDRB
#define IO9_DDR    DDRD
#define IO10_DDR   DDRD
#define IO11_DDR   DDRA

#define IO1_READ    PINB
#define IO2_READ    PINB
#define IO3_READ    PINB
#define IO4_READ    PINB
#define IO5_READ    PINB
#define IO6_READ    PINB
#define IO7_READ    PINB
#define IO8_READ    PINB
#define IO9_READ    PIND
#define IO10_READ   PIND
#define IO11_READ   PINA

#define IO1_WRITE    PORTB
#define IO2_WRITE    PORTB
#define IO3_WRITE    PORTB
#define IO4_WRITE    PORTB
#define IO5_WRITE    PORTB
#define IO6_WRITE    PORTB
#define IO7_WRITE    PORTB
#define IO8_WRITE    PORTB
#define IO9_WRITE    PORTD
#define IO10_WRITE   PORTD
#define IO11_WRITE   PORTA



#define IS_IO1_SET()			    (IO1_READ & BIT(IO1))
#define IS_IO2_SET()			    (IO2_READ & BIT(IO2))
#define IS_IO3_SET()			    (IO3_READ & BIT(IO3))
#define IS_IO4_SET()			    (IO4_READ & BIT(IO4))
#define IS_IO5_SET()			    (IO5_READ & BIT(IO5))
#define IS_IO6_SET()			    (IO6_READ & BIT(IO6))
#define IS_IO7_SET()			    (IO7_READ & BIT(IO7))
#define IS_IO8_SET()			    (IO8_READ & BIT(IO8))
#define IS_IO9_SET()			    (IO9_READ & BIT(IO9))
#define IS_IO10_SET()			    (IO10_READ & BIT(IO10))
#define IS_IO11_SET()			    (IO11_READ & BIT(IO11))

#define SET_IO1()			     SETBIT( IO1_WRITE, IO1 )
#define SET_IO2()			     SETBIT( IO2_WRITE, IO2 )
#define SET_IO3()			     SETBIT( IO3_WRITE, IO3 )
#define SET_IO4()			     SETBIT( IO4_WRITE, IO4 )
#define SET_IO5()			     SETBIT( IO5_WRITE, IO5 )
#define SET_IO6()			     SETBIT( IO6_WRITE, IO6 )
#define SET_IO7()			     SETBIT( IO7_WRITE, IO7 )
#define SET_IO8()			     SETBIT( IO8_WRITE, IO8 )
#define SET_IO9()			     SETBIT( IO9_WRITE, IO9 )
#define SET_IO10()			     SETBIT( IO10_WRITE, IO10 )
#define SET_IO11()			     SETBIT( IO11_WRITE, IO11 )

#define CLEAR_IO1()			     CLEARBIT( IO1_WRITE, IO1 )
#define CLEAR_IO2()			     CLEARBIT( IO2_WRITE, IO2 )
#define CLEAR_IO3()			     CLEARBIT( IO3_WRITE, IO3 )
#define CLEAR_IO4()			     CLEARBIT( IO4_WRITE, IO4 )
#define CLEAR_IO5()			     CLEARBIT( IO5_WRITE, IO5 )
#define CLEAR_IO6()			     CLEARBIT( IO6_WRITE, IO6 )
#define CLEAR_IO7()			     CLEARBIT( IO7_WRITE, IO7 )
#define CLEAR_IO8()			     CLEARBIT( IO8_WRITE, IO8 )
#define CLEAR_IO9()			     CLEARBIT( IO9_WRITE, IO9 )
#define CLEAR_IO10()			     CLEARBIT( IO10_WRITE, IO10 )
#define CLEAR_IO11()			     CLEARBIT( IO11_WRITE, IO11 )

#define TOGGLE_IO11()			TOGGLEBIT( IO11_WRITE, IO11 )

void set_direction(uint8_t direction);
void set_port(uint8_t value, uint8_t mask);
uint8_t get_port();
void set_pin(uint8_t pin, uint8_t value);
uint8_t get_pin(uint8_t pin);

