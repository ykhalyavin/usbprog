#include <stdlib.h>
#include <avr/io.h>
#include <avr/boot.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#define F_CPU 16000000
#include <avr/delay.h>


#include "uart.h"


/* stage vars for avrupdate */

uint8_t state;
uint8_t page_addr;
uint8_t page_addr_w;
uint8_t pageblock[128];
uint8_t collect128;


#define NONE	    0x00
#define STARTAPP    0x01
#define WRITEPAGE   0x02
#define READCHKSUM  0x03


void avrupdate_program_page (uint32_t page);

// start programm from application sector
void avrupdate_runapp();

void avrupdate_update(char *buf);


