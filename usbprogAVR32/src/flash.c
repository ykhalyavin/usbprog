#include "flash.h"

#include "avr32_ebi.h"
#include "avr32_bsr.h"
#include "wait.h"


STATUS_T seq_head(uint8_t add_on);
STATUS_T seq_cmd(uint16_t cmd);
STATUS_T seq_exit(void);


void flash_init(void)
{
	avr32_ebi_init();
}

STATUS_T flash_null_read(uint8_t add_on)
{
	uint16_t temp;
	
	return avr32_ebi_read16(0, &temp, add_on);
}

STATUS_T flash_get_id(uint32_t *id)
{
	STATUS_T r = FLASH_STATUS_OK;
	uint16_t* p = (uint16_t*)id;

	r |= flash_null_read(0);

	r |= seq_head(1);
	r |= seq_cmd(0x0090);
	r |= avr32_ebi_read16(0, &p[1], 1);
	r |= avr32_ebi_read16(2, &p[0], 1);
	r |= seq_exit();

	r |= flash_null_read(1);

	return r;
}

STATUS_T flash_unlock_sector(uint32_t address)
{
	STATUS_T r = FLASH_STATUS_OK;
	
	r |= seq_cmd(0x00AA);
	r |= avr32_ebi_write16(address, 0x0070, 1);

	return r;
}

STATUS_T flash_erase_sector(uint32_t address)
{
	STATUS_T r = FLASH_STATUS_OK;
	uint8_t count = 0;
	uint16_t data;
	
	avr32_ebi_led(IOP_LED_A, IOP_LED_ON);
	r |= flash_null_read(0);

	r |= seq_head(1);
	r |= seq_cmd(0x0080);
	r |= seq_head(1);
	r |= avr32_ebi_write16(address, 0x0030, 1);
	
	/* Wait for completion */
	do {
		if (count > (uint8_t)(SECTOR_EREASE_TIME_OUT / SECTOR_EREASE_TIME))
			break;
		
		wait_ms(SECTOR_EREASE_TIME);
		count++;
		
		r |= avr32_ebi_read16(address, &data, 1);
	} while ((data != 0xffff) && !(data & 0x28));
	r |= seq_exit();
	
	avr32_ebi_led(IOP_LED_A, IOP_LED_OFF);
	r |= flash_null_read(1);

	if (count > (uint8_t)(SECTOR_EREASE_TIME_OUT / SECTOR_EREASE_TIME))
		return FLASH_STATUS_TIME_OUT;

	return r;
}

STATUS_T flash_program(uint32_t address, uint16_t data)
{
	STATUS_T r = FLASH_STATUS_OK;
	uint32_t addr = ALIGN_ADDRESS(address);

#if 1	
	r |= seq_head(1);
	r |= seq_cmd(0x00A0);
	r |= avr32_ebi_write16(addr, data, 1);
	//r |= seq_exit();

#else
	uint16_t d0, d1;
	
	r |= seq_head(1);
	r |= seq_cmd(0x00A0);
	r |= avr32_ebi_write16(addr, data, 1);
	
	/* Wait for completion */
	r |= avr32_ebi_read16(addr, &d1, 1);
	do {
		/* TODO: Timeout */
		r = avr32_ebi_read16(addr, &d0,1);
	} while ( ((d0 ^ d1) & 0x40)		// toggled
			  && !(d1 & 0x28) );		// error bits

	/* We'll need to check once again for toggle bit
	 * because the toggle bit may stop toggling as I/O5
	 * changes to "1" (ref at49bv642.pdf p9)
	 */
	r |= avr32_ebi_read16(addr, &d1,1);
	r |= avr32_ebi_read16(addr, &d0,1);
	r |= seq_exit();
	r |= flash_null_read(1);
	if ( ((d0 ^ d1) & 0x40) && (check != 0) )
		r = FLASH_STATUS_PROGRAM_ERROR;
#endif

	return r;
}

STATUS_T seq_head(uint8_t add_on)
{
	if (avr32_ebi_write16(0x0555<<1, 0x00AA, add_on) != AVR32_EBI_STATUS_OK)
		return FLASH_STATUS_SUBROUTINE_ERROR;
	if (avr32_ebi_write16(0x02aa<<1, 0x0055, 1) != AVR32_EBI_STATUS_OK)
		return FLASH_STATUS_SUBROUTINE_ERROR;

	return FLASH_STATUS_OK;
}

STATUS_T seq_cmd(uint16_t cmd)
{
	if (avr32_ebi_write16(0x0555<<1, cmd, 1) != AVR32_EBI_STATUS_OK)
		return FLASH_STATUS_SUBROUTINE_ERROR;
	else
		return FLASH_STATUS_OK;
}

STATUS_T seq_exit(void)
{
	return avr32_ebi_write16(0, 0x00F0, 1);
}
