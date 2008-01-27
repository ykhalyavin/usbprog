#include "jtag_bus.h"

//volatile register uint8_t port_data asm ("r20");
static uint8_t port_data;	// if the port_data can be defined as register variable, the program would be run faster

void jtag_bus_init(void)
{
	/* Set the pins for output */
	JTAG_PORT_DDR = JTAG_TCK | JTAG_TMS | JTAG_TDI | JTAG_NSRST | JTAG_NTRST | JTAG_GPIO0 | JTAG_GPIO1;

	/* Initializing the output pins' status of the JTAG port:
	 * TCK=0, TMS=1, TDI=0, NSRST=1, NTRST=1
	 */
	port_data = JTAG_NTRST | JTAG_NSRST | JTAG_TMS;
	JTAG_PORT_SET = port_data;
}

/*
uint8_t jtag_bus_get_tdo(void)
{
	return (JTAG_PORT_GET & JTAG_TDO);
}
*/

void jtag_bus_set_tms(uint8_t value)
{
	if (value)
		port_data |= JTAG_TMS;
	else
		port_data &= ~JTAG_TMS;
}

void jtag_bus_set_tdi(uint8_t value)
{
	if (value)
		port_data |= JTAG_TDI;
	else
		port_data &= ~JTAG_TDI;
}

void jtag_bus_update(void)
{
	JTAG_PORT_SET = port_data;
}

void jtag_bus_pulse_tck(void)
{
	JTAG_PORT_SET = port_data | JTAG_TCK;
}

uint8_t jtag_bus_trans(uint8_t tms, uint8_t tdi)
{
	jtag_bus_set_tms(tms);
	jtag_bus_set_tdi(tdi);
	jtag_bus_update();
	//__asm__ volatile("nop");
	jtag_bus_pulse_tck();

	return jtag_bus_get_tdo();
}

void jtag_bus_set_ntrst(uint8_t value)
{
	if (value)
		port_data |= JTAG_NTRST;
	else
		port_data &= ~JTAG_NTRST;
	jtag_bus_update();
}

void jtag_bus_set_nsrst(uint8_t value)
{
	if (value)
		port_data |= JTAG_NSRST;
	else
		port_data &= ~JTAG_NSRST;
	jtag_bus_update();
}
