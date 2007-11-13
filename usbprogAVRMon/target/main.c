/* A small complete program for testing the debug monitor. On an
   STK200 board it will do some flashing of the LEDs PB0 to PB3. */

/* Needs some header files from avr-libc for the bit manipulation
   macros etc. */

#include <avr/io.h>
//#include <avr/progmem.h>

/* Include this to get the TRAP() macro and access to the SendByte()
   function from the debug monitor. */

#include "monitor.h"

/*------------------------------------------------------------------*/
/* Print strings from program memory                                */
/*------------------------------------------------------------------*/

void sendprgstr(char *s)
{
  char c;
  while ((c = PRG_RDB(s++)))
    SendByte(c);
}

/*------------------------------------------------------------------*/
/* 90s8515 (28 pins) and 90s2313 (20 pins)                          */
/*------------------------------------------------------------------*/

int main(void)
{
  while(1);
  return 0;
}

