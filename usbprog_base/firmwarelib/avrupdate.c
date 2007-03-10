#include "avrupdate.h"

void (*avrupdate_jump_to_boot)( void ) = (void *) 0x7000;

uint8_t ee_version EEMEM = 1;


void avrupdate_start()
{
	GICR |= _BV(IVSEL); //move interruptvectors to the Boot sector
	eeprom_write_byte(&ee_version,0xFF); //force bootloader to start
	avrupdate_jump_to_boot();
}
