#ifndef __AVR32_EBI_H__
#define __AVR32_EBI_H__

#include <stdint.h>
#include "../include/basic.h"

/* Status codes */
#define AVR32_EBI_STATUS_OK					0x00
#define AVR32_EBI_STATUS_INVALID_PARAM		0x01
#define AVR32_EBI_STATUS_INVALID_IDCODE		0x02
#define AVR32_EBI_STATUS_SUBROUTINE_ERROR	0x03
#define AVR32_EBI_STATUS_ERROR				0x04

#define ALIGN_ADDRESS(addr) 				((addr) & 0xFFFFFFFE)

void avr32_ebi_init(void);

STATUS_T avr32_ebi_read16(uint32_t address, uint16_t *data, uint8_t add_on);
STATUS_T avr32_ebi_blockread16(uint32_t address, uint16_t num, uint16_t *data);

STATUS_T avr32_ebi_write16(uint32_t address, uint16_t data, uint8_t add_on);

void avr32_ebi_led(uint8_t which, uint8_t status);
uint8_t avr32_ebi_j15(void);

#endif  /* __AVR32_EBI_H__ */

