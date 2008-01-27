#include "avr32_bsr.h"

#include <avr/pgmspace.h>

#define BUF_BIT0			(BUF_T)BIT(0)

static const IOP_T J15 PROGMEM =
	{PB30/BUF_BIT_WIDTH, BUF_BIT0<<(PB30%BUF_BIT_WIDTH)};

static const IOP_T LED[] PROGMEM = {
	{PA16/BUF_BIT_WIDTH, BUF_BIT0<<(PA16 % BUF_BIT_WIDTH)},	// SYS
	{PA19/BUF_BIT_WIDTH, BUF_BIT0<<(PA19 % BUF_BIT_WIDTH)},	// A
	{PE19/BUF_BIT_WIDTH, BUF_BIT0<<(PE19 % BUF_BIT_WIDTH)}		// B
};

/* External Static Memory Bus */
static const IOP_T ADDR[ADDR_BUS_WIDTH] PROGMEM = {
	{PX16/BUF_BIT_WIDTH, BUF_BIT0<<(PX16 % BUF_BIT_WIDTH)},
	{PX17/BUF_BIT_WIDTH, BUF_BIT0<<(PX17 % BUF_BIT_WIDTH)},
	{PX18/BUF_BIT_WIDTH, BUF_BIT0<<(PX18 % BUF_BIT_WIDTH)},
	{PX19/BUF_BIT_WIDTH, BUF_BIT0<<(PX19 % BUF_BIT_WIDTH)},
	{PX20/BUF_BIT_WIDTH, BUF_BIT0<<(PX20 % BUF_BIT_WIDTH)},
	{PX21/BUF_BIT_WIDTH, BUF_BIT0<<(PX21 % BUF_BIT_WIDTH)},
	{PX22/BUF_BIT_WIDTH, BUF_BIT0<<(PX22 % BUF_BIT_WIDTH)},
	{PX23/BUF_BIT_WIDTH, BUF_BIT0<<(PX23 % BUF_BIT_WIDTH)},
	{PX24/BUF_BIT_WIDTH, BUF_BIT0<<(PX24 % BUF_BIT_WIDTH)},
	{PX25/BUF_BIT_WIDTH, BUF_BIT0<<(PX25 % BUF_BIT_WIDTH)},
	{PX26/BUF_BIT_WIDTH, BUF_BIT0<<(PX26 % BUF_BIT_WIDTH)},
	{PX27/BUF_BIT_WIDTH, BUF_BIT0<<(PX27 % BUF_BIT_WIDTH)},
	{PX28/BUF_BIT_WIDTH, BUF_BIT0<<(PX28 % BUF_BIT_WIDTH)},
	{PX29/BUF_BIT_WIDTH, BUF_BIT0<<(PX29 % BUF_BIT_WIDTH)},
	{PX30/BUF_BIT_WIDTH, BUF_BIT0<<(PX30 % BUF_BIT_WIDTH)},
	{PX31/BUF_BIT_WIDTH, BUF_BIT0<<(PX31 % BUF_BIT_WIDTH)},
	{PX32/BUF_BIT_WIDTH, BUF_BIT0<<(PX32 % BUF_BIT_WIDTH)},
	{PX33/BUF_BIT_WIDTH, BUF_BIT0<<(PX33 % BUF_BIT_WIDTH)},
	{PX34/BUF_BIT_WIDTH, BUF_BIT0<<(PX34 % BUF_BIT_WIDTH)},
	{PX35/BUF_BIT_WIDTH, BUF_BIT0<<(PX35 % BUF_BIT_WIDTH)},
	{PX36/BUF_BIT_WIDTH, BUF_BIT0<<(PX36 % BUF_BIT_WIDTH)},
	{PX37/BUF_BIT_WIDTH, BUF_BIT0<<(PX37 % BUF_BIT_WIDTH)},
	{PX38/BUF_BIT_WIDTH, BUF_BIT0<<(PX38 % BUF_BIT_WIDTH)}
};

static const IOP_T NCS[NCS_NUM] PROGMEM = {
	{PX39/BUF_BIT_WIDTH, BUF_BIT0<<(PX39 % BUF_BIT_WIDTH)},
	{PX40/BUF_BIT_WIDTH, BUF_BIT0<<(PX40 % BUF_BIT_WIDTH)},
	{PX41/BUF_BIT_WIDTH, BUF_BIT0<<(PX41 % BUF_BIT_WIDTH)}
};

static const IOP_T NRD PROGMEM =
	{PX42/BUF_BIT_WIDTH, BUF_BIT0<<(PX42 % BUF_BIT_WIDTH)};

static const IOP_T NWE[NWE_NUM] PROGMEM = {
	{PX43/BUF_BIT_WIDTH, BUF_BIT0<<(PX43 % BUF_BIT_WIDTH)},
	{PX44/BUF_BIT_WIDTH, BUF_BIT0<<(PX44 % BUF_BIT_WIDTH)},
	{PX45/BUF_BIT_WIDTH, BUF_BIT0<<(PX45 % BUF_BIT_WIDTH)},
};

static const IOP_T DATA[DATA_BUS_WIDTH] PROGMEM = {
	{PX00/BUF_BIT_WIDTH, BUF_BIT0<<(PX00 % BUF_BIT_WIDTH)},
	{PX01/BUF_BIT_WIDTH, BUF_BIT0<<(PX01 % BUF_BIT_WIDTH)},
	{PX02/BUF_BIT_WIDTH, BUF_BIT0<<(PX02 % BUF_BIT_WIDTH)},
	{PX03/BUF_BIT_WIDTH, BUF_BIT0<<(PX03 % BUF_BIT_WIDTH)},
	{PX04/BUF_BIT_WIDTH, BUF_BIT0<<(PX04 % BUF_BIT_WIDTH)},
	{PX05/BUF_BIT_WIDTH, BUF_BIT0<<(PX05 % BUF_BIT_WIDTH)},
	{PX06/BUF_BIT_WIDTH, BUF_BIT0<<(PX06 % BUF_BIT_WIDTH)},
	{PX07/BUF_BIT_WIDTH, BUF_BIT0<<(PX07 % BUF_BIT_WIDTH)},
	{PX08/BUF_BIT_WIDTH, BUF_BIT0<<(PX08 % BUF_BIT_WIDTH)},
	{PX09/BUF_BIT_WIDTH, BUF_BIT0<<(PX09 % BUF_BIT_WIDTH)},
	{PX10/BUF_BIT_WIDTH, BUF_BIT0<<(PX10 % BUF_BIT_WIDTH)},
	{PX11/BUF_BIT_WIDTH, BUF_BIT0<<(PX11 % BUF_BIT_WIDTH)},
	{PX12/BUF_BIT_WIDTH, BUF_BIT0<<(PX12 % BUF_BIT_WIDTH)},
	{PX13/BUF_BIT_WIDTH, BUF_BIT0<<(PX13 % BUF_BIT_WIDTH)},
	{PX14/BUF_BIT_WIDTH, BUF_BIT0<<(PX14 % BUF_BIT_WIDTH)},
	{PX15/BUF_BIT_WIDTH, BUF_BIT0<<(PX15 % BUF_BIT_WIDTH)},
};


void avr32_iop_reset(BUF_T* bsr)
{
	IOP_T iop;
	uint16_t* p = (uint16_t*)&iop;
	uint8_t i;

	/* Set all pins (include the data bus and J15) as input, level is high,
	 * see BSDL, the control bit is set for safe.
	 */
	for (i=0; i<BSR_BUF_SIZE; i++)
		bsr[i] = 0xFF;

	/* Set the output pins */
	avr32_iop_set_addr(bsr, 0);						// set address
	avr32_iop_set_nrd(bsr, 1);						// set nRD = 1
	for (i=0; i<3; i++) {
		*p = pgm_read_word(&NCS[i]);
		avr32_iop_set(bsr, iop, IO_OUT, HI);		// set nCS[i] = 1
		*p = pgm_read_word(&NWE[i]);
		avr32_iop_set(bsr, iop, IO_OUT, HI);		// set nWE[i] = 1
		*p = pgm_read_word(&LED[i]);
		avr32_iop_set(bsr, iop, IO_OUT, HI);		// set LED[i] off
	}
}

uint8_t avr32_iop_get(const BUF_T* bsr, IOP_T iop)
{
	return ( (bsr[iop.index] & iop.bit) == 0 ? 0 : iop.bit );
}

void avr32_iop_set(BUF_T* bsr, IOP_T iop, uint8_t io, uint8_t d)
{
	BUF_T *p = &bsr[iop.index];
	uint8_t bit_mask = iop.bit;

	if (d)
		*p |= bit_mask;
	else
		*p &= ~bit_mask;

	bit_mask <<= 1;
	if (io)
		*p |= bit_mask;
	else
		*p &= ~bit_mask;
}

void avr32_iop_led(BUF_T* bsr, uint8_t which, uint8_t status)
{
	IOP_T led;
	uint16_t* p = (uint16_t*)&led;
	
	if (which < 3) {
		*p = pgm_read_word(&LED[which]);
		avr32_iop_set(bsr, led, IO_OUT, status);
	}
}

uint8_t avr32_iop_get_j15(const BUF_T* bsr)
{
	IOP_T iop;
	uint16_t* p = (uint16_t*)&iop;
	
	*p = pgm_read_word(&J15);
	
	return avr32_iop_get(bsr, iop);
}

void avr32_iop_cs(BUF_T* bsr, uint8_t which)
{
	IOP_T ncs;
	uint16_t* p = (uint16_t*)&ncs;
	uint8_t i;

	for (i=0; i<3; i++) {
		*p = pgm_read_word(&NCS[i]);
		avr32_iop_set(bsr, ncs, IO_OUT, (i == which ? LO : HI));
	}
}

void avr32_iop_set_addr(BUF_T* bsr, uint32_t address)
{
	IOP_T addr;
	uint16_t* p = (uint16_t*)&addr;
	uint32_t bit_mask;
	uint8_t i;

	for (i=0, bit_mask=1; i<ADDR_BUS_WIDTH; i++, bit_mask<<=1) {
		*p = pgm_read_word(&ADDR[i]);
		avr32_iop_set(bsr, addr, IO_OUT, ((address & bit_mask) ? 1 : 0));
	}
/*
	const IOP_T *p;
	uint8_t i;
	uint32_t bit_mask;

	p = &ADDR[0];
	for (i = 0, bit_mask = 1; i < ADDR_BUS_WIDTH; i++, bit_mask <<= 1)
		avr32_iop_set(bsr, *p++, IO_OUT, ((address & bit_mask) ? 1 : 0));
*/
}

void avr32_iop_set_data_out(BUF_T* bsr, uint16_t value)
{
	IOP_T data;
	uint16_t* p = (uint16_t*)&data;
	uint16_t bit_mask;
	uint8_t i;

	for (i=0, bit_mask=1; i<DATA_BUS_WIDTH; i++, bit_mask<<=1) {
		*p = pgm_read_word(&DATA[i]);
		avr32_iop_set(bsr, data, IO_OUT, ((value & bit_mask) ? 1 : 0));
	}
/*
	const IOP_T* p = DATA;
	uint16_t bit_mask;
	uint8_t i;

	for (i=0, bit_mask=1; i<DATA_BUS_WIDTH; i++, bit_mask<<=1)
		avr32_iop_set(bsr, *p++, IO_OUT, ((value & bit_mask) ? 1 : 0));
*/
}

void avr32_iop_set_data_in(BUF_T* bsr)
{
	IOP_T data;
	uint16_t* p = (uint16_t*)&data;
	uint8_t i;

	for (i=0; i<DATA_BUS_WIDTH; i++) {
		*p = pgm_read_word(&DATA[i]);
		avr32_iop_set(bsr, data, IO_IN, HI);
	}
/*
	const IOP_T* p = DATA;
	uint8_t i;

	for (i=0; i<DATA_BUS_WIDTH; i++)
		avr32_iop_set(bsr, *p++, IO_IN, HI);
*/
}

uint16_t avr32_iop_get_data(const BUF_T* bsr)
{
	IOP_T data;
	uint16_t* p = (uint16_t*)&data;
	uint8_t i;
	uint16_t bit_mask;
	uint16_t value = 0x0000;

	for (i=0, bit_mask=1; i<DATA_BUS_WIDTH; i++, bit_mask<<=1) {
		*p = pgm_read_word(&DATA[i]);
		if (avr32_iop_get(bsr, data) != 0)
			value |= bit_mask;
	}
/*
	const IOP_T *p;
	uint8_t i;
	uint16_t bit_mask;
	uint16_t value = 0x0000;

	p = &DATA[0];
	for (i = 0, bit_mask = 1; i < DATA_BUS_WIDTH; i++, bit_mask <<= 1)
		if (avr32_iop_get(bsr, *p++) != 0)
			value |= bit_mask;
*/
	return value;
}

void avr32_iop_set_nwe0(BUF_T* bsr, uint8_t value)
{
	IOP_T nwe0;
	uint16_t* p = (uint16_t*)&nwe0;
	
	*p = pgm_read_word(&NWE[0]);
	avr32_iop_set(bsr, nwe0, IO_OUT, value);
}

void avr32_iop_set_nrd(BUF_T* bsr, uint8_t value)
{
	IOP_T nrd;
	uint16_t* p = (uint16_t*)&nrd;

	*p = pgm_read_word(&NRD);
	avr32_iop_set(bsr, nrd, IO_OUT, value);
}


