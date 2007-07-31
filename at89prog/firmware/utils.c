#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <inttypes.h>

#define F_CPU 16000000
#include <util/delay.h>

#include "uart.h"
#include "utils.h"


void wait_ms(int ms)
{
  int i;
  for(i=0;i<ms;i++)
   _delay_ms(1);
}

