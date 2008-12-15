#ifndef _AVRUPDATE_H_
#define _AVRUPDATE_H_


#include <stdlib.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>


// EEMEM wird bei aktuellen Versionen der avr-lib in eeprom.h definiert
// hier: definiere falls noch nicht bekannt ("alte" avr-libc)
#ifndef EEMEM
// alle Textstellen EEMEM im Quellcode durch __attribute__ ... ersetzen
#define EEMEM  __attribute__ ((section (".eeprom")))
#endif

//uint8_t ee_version EEMEM = 1;
//uint8_t ee_state EEMEM = 2;
//char ee_title EEMEM = 3;

#define STARTAVRUPDATE	0x01	// vendor request to start avrupdate

void avrupdate_start(void);

#endif
