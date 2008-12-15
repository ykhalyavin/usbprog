/* USBprogRFM12
 *
 * Copyright (C) 2005  Benedikt Sauter
 * Copyright (C) 2008  Manuel Stahl
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
#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>

#include "led.h"
#include "fifo.h"
#include "usbn2mc.h"
#include "usbn2mc/class/CDC/usbcdc.h"
#include "firmwarelib/avrupdate.h"

// Berechnungen
#define UBRR_VAL(baud) ((F_CPU+(baud<<3))/(baud<<4)-1)   // clever runden

#define UARTReady()		(UCSRA & (1<<UDRE))

typedef union {
	uint16_t u16;
	struct {
		uint8_t u8l;
		uint8_t u8h;
	};
} uint16_u;

volatile FIFO64_t sendBuffer;
volatile FIFO64_t recvBuffer;
//volatile uint8_t reset_counter = 0;


SIGNAL(SIG_UART_RECV)
{
	FIFO64_write(recvBuffer, UDR);
	LED_toggle();
}

SIGNAL(SIG_UART_DATA)
{
	if(FIFO_available(sendBuffer)) {
		LED_toggle();
		UDR = FIFO64_read(sendBuffer);
	} else {
        /* tx buffer empty, disable UDRE interrupt */
		UCSRB &= ~_BV(UDRIE);
    }
}

/*************** usb requests  **************/


/* Vendor request: needed for live update of firmware */
void USBNDecodeVendorRequest(DeviceRequest_t *req) {
	switch (req->bRequest) {
		case STARTAVRUPDATE:
			avrupdate_start();
			break;
	}
	USBNWrite(RXC0, FLUSH);
}

void USB_CDC_setLineCoding(USB_CDC_LineCoding_t* lc) {
//	uint8_t* ptr = (uint8_t*)lc;
//	for(uint8_t i=0; i < 7; i++)
//		FIFO_write(recvBuffer, ptr[i], 64);
//	reset_counter++;

	uint16_u baud;
	baud.u16 = UBRR_VAL(lc->dwDTERrate);
	UBRRH = baud.u8h;
	UBRRL = baud.u8l;
	// TODO: set stop bits, parity, char-size
}

// called from USB interrupt
void USB_CDC_rxCallback(uint8_t buf[], uint8_t len) {
	uint8_t i;
	for(i=0; i<len; i++)
		FIFO64_write(sendBuffer, buf[i]);
	UCSRB |= _BV(UDRIE);
}

void UARTInit() {
	UBRRH = UBRR_VAL(9600UL) >> 8;
	UBRRL = UBRR_VAL(9600UL);

	UCSRA = (1 << RXC) | (1 << TXC);
	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
	UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
}

/*************** main function  **************/

int main(void) {
	uint16_t timer = 0;

	LED_init();
	LED_off();
	FIFO_init(sendBuffer);
	FIFO_init(recvBuffer);

	sei();				// activate global interrupts

	UARTInit();

	USB_CDC_init();		// setup USBN as CDC device

	USBNAddStringDescriptor("USBprog EmbeddedProjects");	// add vendor
	USBNAddStringDescriptor("USBprogRS232");				// add product

	USBNInitMC();		// start USB controller
	USBNStart();		// start device stack

	for (;;) {
		uint8_t i=0;
		uint8_t buf[8];

		if(FIFO_length(recvBuffer, 64) == 8 || ++timer == 0) {
			while(FIFO_available(recvBuffer) && i < 8)
				buf[i++] = FIFO64_read(recvBuffer);

			USB_CDC_tx(buf, i);
		}

//		if(reset_counter > 4) {
//			_delay_ms(1000);
//			avrupdate_start();
//			_delay_ms(2000);
//		}
	}
}

