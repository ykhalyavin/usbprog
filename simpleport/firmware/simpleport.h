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


#define BIT(x)            (1 << (x))
#define SETBIT(addr,x)    (addr |= BIT(x))
#define CLEARBIT(addr,x)  (addr &= ~BIT(x))


#define JTAG_PORT_INIT          DDRB
#define JTAG_PORT_WRITE         PORTB
#define JTAG_PORT_READ          PINB

#define TDI	    PB5
#define TDO	    PB6
#define TMS	    PB0
#define TCK	    PB7

#define LED_PIN     PA4
#define LED_PORT    PORTA

#define LED_ON()     (LED_PORT   |=  (1 << LED_PIN))   // red led
#define LED_OFF()    (LED_PORT   &= ~(1 << LED_PIN))


// check if tdo == 1
#define JTAG_IS_TDO_SET()		    (JTAG_PORT_READ & BIT(TDO))

#define JTAG_SET_TCK()                       SETBIT( JTAG_PORT_WRITE, TCK )
#define JTAG_CLEAR_TCK()                     CLEARBIT( JTAG_PORT_WRITE, TCK )
//
#define JTAG_SET_TMS()                       SETBIT( JTAG_PORT_WRITE, TMS )
#define JTAG_CLEAR_TMS()                     CLEARBIT( JTAG_PORT_WRITE, TMS )

#define JTAG_SET_TDI()                       SETBIT( JTAG_PORT_WRITE, TDI )
#define JTAG_CLEAR_TDI()                     CLEARBIT( JTAG_PORT_WRITE, TDI )

#define JTAG_SET_TCK()                       SETBIT( JTAG_PORT_WRITE, TCK )
#define JTAG_CLEAR_TCK()                     CLEARBIT( JTAG_PORT_WRITE, TCK )


    // start usb chip
    USBNStart();

    // led
    DDRA = (1 << PA4);


    // use as output
    JTAG_PORT_INIT |= (1<<TCK)|(1<<TMS)|(1<<TDI);
    // use as input
    JTAG_PORT_INIT &=~(1<<TDO);
    // pullup
    JTAG_PORT_WRITE |= (1<<TDO);


void set_direction(uint8_t direction);
void set_port(uint8_t value);
uint8_t get_port();
void set_bit(uint8_t bit, uint8_t value);
void get_get(uint8_t bit);

