#include "avrupdate.h"

void (*avrupdate_jump_to_boot)( void ) = (void *) 0x7000;

//uint8_t ee_version EEMEM = 1;

void avrupdate_start(void)
{
  //	eeprom_write_byte(&ee_version,0xFF); //force bootloader to start
  // uint8_t myByte=0x00;
  // while(myByte !=0xFF)
  // 	 myByte = eeprom_read_byte(&ee_version);
  PORTA |= (1 << PA4);

  cli();    // disable all interrupts
  avrupdate_jump_to_boot();
}
