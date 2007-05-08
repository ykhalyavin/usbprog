/*
    Turbo BDM Light ColdFire - general timing & LED handling
    Copyright (C) 2005  Daniel Malik

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "hidef.h"
#include "MC68HC908JB16.h"
#include "led.h"
#include "main.h"

#pragma DATA_SEG Z_RAM
static near unsigned char led_timer;      /* counter for timing the LED flashing */
led_state_e near led_state;               /* led state variable (BLINK, ON, OFF) */
#pragma DATA_SEG DEFAULT

/* 10ms tick */
/* handles general timing functions and blinks the LED */
interrupt void timer2_10ms_tick(void) {
  T2SC_TOF=0;       /* clear the interrupt flag */
  if (led_timer) {  /* led is in the middle of blinking */
    led_timer--;    /* decrement the counter */
    if (led_timer==LED_OFF_TIME) LED_SW_OFF(); /* switch the led off when the counter hits the off treshold */
  } else {
    if (led_state==LED_ON) {
      LED_SW_ON();    /* switch it on */
    } else if (led_state==LED_OFF) {
      LED_SW_OFF();   /* switch it off */
    } else if (led_state==LED_BLINK) {
      LED_SW_ON();								 /* switch it on */
      led_state=LED_ON;            /* change state to permanent ON */
      led_timer=LED_BLINK_PERIOD;  /* blink the LED once before it settles into the ON state */
    }
  }
}

/* initilises timer 2 for 10ms tick */
void timer2_init(void) {
  LED_INIT();                           /* initliase the LED */
  T2MOD=5000*BUS_FREQUENCY-1;           /* 10ms with prescaler of 2 */
  T2SC= T2SC_TOIE_MASK | T2SC_PS0_MASK; /* enable overflow interrupt, prescaler of 2 */
}

/* wait - time in 10us ticks */
/* only works up to 8 MHz bus frequency at the moment */
/* the real wait time is slightly longer (LDA for parameter load, JSR and RTS) */
void wait_10us(unsigned char ticks) {
  asm {
  loop:
  #if (BUS_FREQUENCY>7)
    BRSET 0,0,0		/* 5 cycles */  /* extra 10 cycles for 8 MHz bus */
    BRSET 0,0,0   /* 5 cycles */  
  #endif
  #if (BUS_FREQUENCY>6)
    BRSET 0,0,0		/* 5 cycles */  /* extra 10 cycles for 7 MHz bus */
    BRSET 0,0,0   /* 5 cycles */  
  #endif
  #if (BUS_FREQUENCY>5)
    BRSET 0,0,0		/* 5 cycles */  /* extra 10 cycles for 6 MHz bus */
    BRSET 0,0,0   /* 5 cycles */  
  #endif
  #if (BUS_FREQUENCY>4)
    BRSET 0,0,0		/* 5 cycles */  /* extra 10 cycles for 5 MHz bus */
    BRSET 0,0,0   /* 5 cycles */  
  #endif
  #if (BUS_FREQUENCY>3)
    BRSET 0,0,0		/* 5 cycles */  /* extra 10 cycles for 4 MHz bus */
    BRSET 0,0,0   /* 5 cycles */  
  #endif
  #if (BUS_FREQUENCY>2)
    BRSET 0,0,0		/* 5 cycles */  /* extra 10 cycles for 3 MHz bus */
    BRSET 0,0,0   /* 5 cycles */  
  #endif
  #if (BUS_FREQUENCY>1)
    BRSET 0,0,0		/* 5 cycles */  /* extra 10 cycles for 2 MHz bus */
    BRSET 0,0,0   /* 5 cycles */  
  #endif
    BRSET 0,0,0   /* 5 cycles */  
    NOP           /* 2 cycles = NOPS + 3 cycles = DBNZA */
    NOP
    DBNZA loop    /* 10u per iteration */ 
  }
}

/* wait - time in 1ms ticks */
/* the time is slightly longer that the desired time */
/* the real wait time is ticks * 5 cycles longer + JSR & RTS */
void wait_1ms(unsigned char ticks) {
  asm {
  loop1:
    LDX   #(BUS_FREQUENCY*25)   /* 40 cycles * BUS_FREQUENCY * 25 = 1ms */
  loop:
    BRSET 0,0,0		/* 5 cycles */  
    BRSET 0,0,0		/* 5 cycles */  
    BRSET 0,0,0		/* 5 cycles */  
    BRSET 0,0,0		/* 5 cycles */  
    BRSET 0,0,0		/* 5 cycles */  
    BRSET 0,0,0		/* 5 cycles */  
    BRSET 0,0,0		/* 5 cycles */  
    NOP				    /* 1 cycle */
    NOP				    /* 1 cycle */
    DBNZX loop    /* 3 cycles */ /* total: 7*5 + 2 + 3 = 40 cycles */
		DBNZA loop1   
  }
}
