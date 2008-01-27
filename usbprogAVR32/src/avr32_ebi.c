#include "avr32_ebi.h"

#include "avr32.h"
#include "avr32_bsr.h"


/* Boundary Scan Register */
static BUF_T g_bsr[BSR_BUF_SIZE];

/* The routines in this file are based on the following assumption:
 * 1. nCS to nRD or nWE setup time Tas = 0 ns
 * 2. nCS to nRD or nWE hold time Tah = 0 ns
 * 3. address to nRD or nWE setup time Tas = 0 ns
 * 4. data to nRD or nWE hold time Tdh = 0 ns
 * when oprating on a flash chip.
 */

#define SET_ADDRESS(addr)	{ avr32_iop_set_addr(g_bsr, addr); }
#define SET_DATA(data)		{ avr32_iop_set_data_out(g_bsr, data); }
#define SET_DATA_IN()		{ avr32_iop_set_data_in(g_bsr); }
#define GET_DATA()			(avr32_iop_get_data(g_bsr))
#define ASSERT_READ()		{											\
							  avr32_iop_cs(g_bsr, IOP_NCS0);			\
							  avr32_iop_set_nrd(g_bsr, LO);				\
							}
#define ASSERT_WRITE()		{											\
							  avr32_iop_cs(g_bsr, IOP_NCS0);			\
							  avr32_iop_set_nwe0(g_bsr, LO);			\
}
#define DISASSERT()			{											\
							  avr32_iop_cs(g_bsr, IOP_NCS_NONE);		\
							  avr32_iop_set_nrd(g_bsr, HI);				\
							  avr32_iop_set_nwe0(g_bsr, HI);			\
							}

void avr32_ebi_init(void)
{
	avr32_iop_reset(g_bsr);
}

STATUS_T avr32_ebi_read16(uint32_t address, uint16_t *data, uint8_t add_on)
{
	uint32_t addr = ALIGN_ADDRESS(address);
	
	SET_ADDRESS(addr);
	SET_DATA_IN();
	ASSERT_READ();
	if (avr32_extest(g_bsr, NULL, add_on) != AVR32_STATUS_OK)
		return AVR32_EBI_STATUS_SUBROUTINE_ERROR;

	DISASSERT();
	if (avr32_extest(g_bsr, g_bsr, 1) != AVR32_STATUS_OK)
		return AVR32_EBI_STATUS_SUBROUTINE_ERROR;
	*data = GET_DATA();

	return AVR32_EBI_STATUS_OK;
}

STATUS_T avr32_ebi_blockread16(uint32_t address, uint16_t num, uint16_t *data)
{
	uint32_t addr = ALIGN_ADDRESS(address);
	uint16_t i, *p;
	
	if (num == 0)
		return AVR32_EBI_STATUS_OK;

	SET_ADDRESS(addr);
	SET_DATA_IN();
	ASSERT_READ();
	if (avr32_extest(g_bsr, NULL, 0) != AVR32_STATUS_OK)
		return AVR32_EBI_STATUS_SUBROUTINE_ERROR;

	/* Read data except the last */
	i = 0;
	p = data;
	while (i++ < (num-1)) {	// for (i=0; i<(num-1); i++) 这里用for循环似乎会多读一次
		addr += 2;								// Setup to next address
		SET_ADDRESS(addr);
		if (avr32_extest(g_bsr, g_bsr, 1) != AVR32_STATUS_OK)
			return AVR32_EBI_STATUS_SUBROUTINE_ERROR;
		*p++ = GET_DATA();
	}
/*
//20080121H 此处发现bug: 当size=1时，会陷入死循环。另外，增加addon参数?
	i = size - 1;
	do {
		addr += 2;								// Setup to next address
		SET_ADDRESS(addr);
		if (avr32_extest(g_bsr, g_bsr, 1) != AVR32_STATUS_OK)
			return AVR32_EBI_STATUS_SUBROUTINE_ERROR;

		*p++ = GET_DATA();
	} while (--i != 0);
*/
	DISASSERT();
	if (avr32_extest(g_bsr, g_bsr, 1) != AVR32_STATUS_OK)
		return AVR32_EBI_STATUS_SUBROUTINE_ERROR;
	*p = GET_DATA();

	return AVR32_EBI_STATUS_OK;
}

STATUS_T avr32_ebi_write16(uint32_t address, uint16_t data, uint8_t add_on)
{
	uint32_t addr = ALIGN_ADDRESS(address);

	SET_ADDRESS(addr);
	SET_DATA(data);
	ASSERT_WRITE();
	if (avr32_extest(g_bsr, NULL, add_on) != AVR32_STATUS_OK)
		return AVR32_EBI_STATUS_SUBROUTINE_ERROR;

	DISASSERT();
	SET_DATA_IN();							// Restore data bus as input
	if (avr32_extest(g_bsr, NULL, 1) != AVR32_STATUS_OK)
		return AVR32_EBI_STATUS_SUBROUTINE_ERROR;

	return AVR32_EBI_STATUS_OK;
}

void avr32_ebi_led(uint8_t which, uint8_t status)
{
	avr32_iop_led(g_bsr, which, status);
}

uint8_t avr32_ebi_j15(void)
{
	return avr32_iop_get_j15(g_bsr);
}

