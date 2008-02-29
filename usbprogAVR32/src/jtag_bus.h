#ifndef __JTAG_BUS_H__
#define __JTAG_BUS_H__

#include <stdint.h>
#include <avr/io.h>


/* JTAG port */
#define JTAG_PORT_DDR		DDRB
#define JTAG_PORT_GET		PINB
#define JTAG_PORT_SET		PORTB

/* JTAG port signals' bit mask */

#define JTAG_TDI	(1<<PB5)	// SV2-1, IC1-1,  output
#define JTAG_VT				// SV2-2, JP2-2,  *must be connected to JP2-1 or JP2-3*
#define JTAG_NSRST	(1<<PB4)	// SV2-3, IC1-44, output
#define JTAG_NTRST	(1<<PB3) 	// SV2-4, IC1-43, output
#define JTAG_TMS	(1<<PB0) 	// SV2-5, IC1-40, output
#define JTAG_GPIO0	(1<<PB1) 	// SV2-6, IC1-41,
#define JTAG_TCK	(1<<PB7) 	// SV2-7, IC1-3,  output
#define JTAG_GPIO1	(1<<PB2) 	// SV2-8, IC1-42,
#define JTAG_TDO	(1<<PB6) 	// SV2-9, IC1-2,  input
#define JTAG_GND			// SV2-10, GND


/* 1:1 connection for usbprog - 10 pole cable - levelshifter - 1- pole cable - ngw100 (red = 1) */
/*
#define JTAG_TDI	(1<<PB5)	// SV2-1, IC1-1,  output
#define JTAG_VT				// SV2-2, JP2-2,  *must be connected to JP2-1 or JP2-3*
#define JTAG_NSRST	(1<<PB4)	// SV2-3, IC1-44, output
#define JTAG_NTRST	(1<<PB3) 	// SV2-4, IC1-43, output
#define JTAG_TMS	(1<<PB0) 	// SV2-5, IC1-40, output
#define JTAG_GPIO0	(1<<PB1) 	// SV2-6, IC1-41,
#define JTAG_TCK	(1<<PB7) 	// SV2-7, IC1-3,  output
#define JTAG_GPIO1	(1<<PB2) 	// SV2-8, IC1-42,
#define JTAG_TDO	(1<<PB6) 	// SV2-9, IC1-2,  input
#define JTAG_GND			// SV2-10, GND
*/
/* 直连方式下的管脚定义(注意JP2-2不能与其它脚相连)
#define JTAG_TCK	PB5	// SV2-1, IC1-1,  output
#define JTAG_GND		// SV2-2, JP2-2,  *must be unconnected*
#define JTAG_TDO	PB4	// SV2-3, IC1-44, input
#define JTAG_VT		PB3 // SV2-4, IC1-43, *must be set as input*
#define JTAG_TMS	PB0 // SV2-5, IC1-40, output
#define JTAG_NSRST	PB1 // SV2-6, IC1-41, output
#define JTAG_IO0	PB7 // SV2-7, IC1-3,  should be set as input
#define JTAG_NTRST	PB2 // SV2-8, IC1-42, output
#define JTAG_TDI	PB6 // SV2-9, IC1-2,  output
#define JTAG_IO1		// SV2-10, GND
*/

/* routines */
void jtag_bus_init(void);

uint8_t jtag_bus_trans(uint8_t tms, uint8_t tdi);
//uint8_t jtag_bus_get_tdo(void);
void jtag_bus_set_tms(uint8_t value);
void jtag_bus_set_tdi(uint8_t value);
void jtag_bus_update(void);				// clear TCK, and update the other signals
void jtag_bus_pulse_tck(void);			// set TCK

void jtag_bus_set_ntrst(uint8_t value);
void jtag_bus_set_nsrst(uint8_t value);

#define jtag_bus_get_tdo() (JTAG_PORT_GET & JTAG_TDO)

#endif  /* __JTAG_BUS_H__ */
