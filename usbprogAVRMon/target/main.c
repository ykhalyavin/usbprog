/* A small complete program for testing the debug monitor. On an
   STK200 board it will do some flashing of the LEDs PB0 to PB3. */

/* Needs some header files from avr-libc for the bit manipulation
   macros etc. */

#include <io.h>
#include <progmem.h>

/* Include this to get the TRAP() macro and access to the SendByte()
   function from the debug monitor. */

#include "monitor.h"

/*------------------------------------------------------------------*/
/* Some nice-to-have debug macros                                   */
/*------------------------------------------------------------------*/

/* The main switches, comment out to disable */

#define DTRAP_ON
#define DPRINT_ON

/* The definitions */

#ifdef DTRAP_ON
#  define DTRAP() TRAP()
#else
#  define DTRAP()
#endif

#ifdef DPRINT_ON
#  define DPRINT(str) sendprgstr(PSTR(str))
#else
#  define DPRINT(str)
#endif

/*------------------------------------------------------------------*/
/* Print strings from program memory                                */
/*------------------------------------------------------------------*/

void sendprgstr(prog_char *s)
{
  char c;
  while ((c = PRG_RDB(s++)))
    SendByte(c);
}

/*------------------------------------------------------------------*/
/* 90s8515 (28 pins) and 90s2313 (20 pins)                          */
/*------------------------------------------------------------------*/

#if (__AVR_AT90S8515__ || __AVR_AT90S2313__)
void Test(void)
{
  unsigned char a = 0;

  /* Prints a message on the host monitor: */
  DPRINT("Hello from AVR!\n");

  /* This is a source coded breakpoint: */
  DTRAP();

  sbi(DDRB, PB0);   /* PB0 is output */
  sbi(DDRB, PB1);   /* PB1 is output */
  sbi(DDRB, PB2);   /* PB2 is output */
  sbi(DDRB, PB3);   /* PB3 is output */

  while(1) {
    DTRAP();
    DPRINT("Loop!\n");
    outp( (inp(PINB) & 0xf0) | (~a & 0x0f), PORTB);
    a++;
    if (a == 18) {
      DTRAP();
      DPRINT("Reset!\n");
      a = 0;
    }
  }
}
#endif /* (__AVR_AT90S8515__ || __AVR_AT90S2313__) */

/*------------------------------------------------------------------*/

int main(void)
{
  Test();
  return 0;
}

