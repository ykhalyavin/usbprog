/*  
  Copyright (C) 1999 by Denis Chertykov (denisc@overta.ru)
  STK200 communication added by Tor Ringstad (torhr@pvv.ntnu.no).

  You can redistribute it and/or modify it under the terms of the GNU
  General Public License as published by the Free Software Foundation;
  either version 2, or (at your option) any later version.

  This program works with the connection scheme used in Atmels "STK200
  Starter Kit". The connection allows to program the AVR with Uros
  Platise `uisp' in `-dstk200' mode.

  Bugs: For me this work only if Parellel port in EPP mode.
        May be ECP mode also work? 
*/

#include <avr/io.h>
#include "monitor.h"

/*------------------------------------------------------------------*/
/*  mon.h                                                           */
/*------------------------------------------------------------------*/

/* This is mon.h; definitions that are shared between the host and the
   target part of the monitor system. It isn't good style to duplicate
   the contents of this file here, but it makes the target part of the
   monitor a bit more self-contained. */

struct packet_s
{
  unsigned char cmd;		/* command letter */
  unsigned char data;		/* data for operation */
  union addr_u
  {
    unsigned short addr;	/* operation address */
    struct
    {
      unsigned char lo;
      unsigned char hi;
    } raw;
  } u;
};

#define NUM_BREAKPOINTS 8

#ifdef __AVR__
#  define avr_ptr_t void *
#else
#  define avr_ptr_t unsigned short
#endif

struct avr_mon_data_s
{
  unsigned char reg[32];
  unsigned char sreg;
#define BREAKPOINTS_OFFSET 37
  avr_ptr_t sp;
  avr_ptr_t pc;
  avr_ptr_t breakpoint[NUM_BREAKPOINTS];
};

struct sreg_bits_s
{
  char name;
  unsigned char mask;
};


/*------------------------------------------------------------------*/
/*  Host communication via STK200 ISP                               */
/*------------------------------------------------------------------*/

/* We use different sync characters for Tx and Rx to be able to catch
   any situation where both host and target tries to do the same
   operation at the same time. */

#define TX_SYNC 0xbd   /* 10111101 */
#define RX_SYNC 0xb9   /* 10111001 */

#if __AVR_AT90S2323__
#  define RX PB0    /* MOSI */
#  define TX PB1    /* MISO */
#  define CLK PB2   /* SCK */
#elif  (__AVR_AT90S8515__ || __AVR_AT90S2313__)
#  define RX PB5    /* MOSI */
#  define TX PB6    /* MISO */
#  define CLK PB7   /* SCK */
#else
#  warning "Device type not defined"
#  define RX PB5    /* MOSI */
#  define TX PB6    /* MISO */
#  define CLK PB7   /* SCK */

#endif

/* Initialize the three communication lines. This may cause some
   glithes depending on their current state, but that shouldn't
   matter to the master anyway. */

void InitComm(void)
{
#if 0
  cbi(DDRB, RX);   /* input */
  cbi(PORTB, RX);  /* no pullup */
  cbi(DDRB, CLK);  /* input */
  cbi(PORTB, CLK); /* no pullup */
  sbi(DDRB, TX);   /* output */
  cbi(PORTB, TX);  /* low */
#endif
}

void TxBit(uint8_t b) {
#if 0
  loop_until_bit_is_set(PINB, CLK);    /* Wait for CLK Lo -> Hi */
  if (b) sbi(PORTB, TX);               /* Output data bit */
  else cbi(PORTB, TX);
  loop_until_bit_is_clear(PINB, CLK);  /* Wait for CLK Hi -> Lo */
  cbi(PORTB, TX);                      /* Clear output */
#endif
}

void TxByte(uint8_t b) {
#if 0 
  uint8_t i;
  for(i = 0; i < 8; i++) {             /* Loop over each bit */
    TxBit(b & 0x80);                   /* Tx the MSB */
    b <<= 1;                           /* Shift left */
  }
#endif
}

uint8_t RxBit(void) {
#if 0
  uint8_t bit;
  loop_until_bit_is_set(PINB, CLK);    /* Wait for CLK Lo -> Hi */
  bit = (inp(PINB) >> RX) & 1;         /* Read input bit */
  loop_until_bit_is_clear(PINB, CLK);  /* Wait for CLK Hi -> Lo */
  return bit;
#endif
}

uint8_t RxByte(void) {
#if 0
  uint8_t i;
  uint8_t b = 0;
    for(i = 0; i < 8; i++) {
    b = b << 1;
    b |= RxBit();
  }
  return b;
#endif
}

/* High level routine for sending one byte of data. Blocks until the
   host is ready to receive. */

void SendByte(uint8_t data)
{
  InitComm();                          /* Always init, we're paranoid :) */
  RxBit();                             /* Wait a couple of clock cycles... */
  RxBit();                             /* ...in case there's any garbage */
  TxByte(RX_SYNC);                     /* Tell host we're ready to TX */
  TxByte(data);                        /* Tx the data byte */
}

/* High level routine for receiving one byte of data. Blocks until the
   host is ready to send. */

uint8_t RecvByte(void)
{
  InitComm();                          /* Always init, we're paranoid :) */
  RxBit();                             /* Wait a couple of clock cycles... */
  RxBit();                             /* ...in case there's any garbage */
  TxByte(TX_SYNC);                     /* Tell host we're ready to RX */
  return RxByte();                     /* Rx and return the data byte */
}

/*------------------------------------------------------------------*/
/*  Monitor code                                                    */
/*------------------------------------------------------------------*/

#define TIMER0_CK 1
#define EXTREMAL

/* Extract high 8 bit of address */
#define hi8(x) \
 ({ unsigned char t; asm volatile ("ldi %0, hi8 (%1)" :"=r"(t) :"i"(x)); t;})
/* Extract low 8 bit of address */
#define lo8(x) \
 ({ unsigned char t; asm volatile ("ldi %0, lo8 (%1)" :"=r"(t) :"i"(x)); t;})

static struct packet_s packet;
static struct avr_mon_data_s regs;

/* Make sure the monitor stack is large enough. When the size is
   changed here, remember to also change the value used in the inline
   assembly code below. */

unsigned char mon_stack[20];

int
parse_packet (struct packet_s *pp)
{
  char t;
  t = pp->cmd;
  if (t == 'r')			/* Read memory */
    pp->data = *(unsigned char*)pp->u.addr;
  else if (t == 'w')		/* Write memory */
    *(unsigned char*)pp->u.addr = pp->data;
  else if (t == 'q')		/* Quit (packet.data == 0) */
    ;				/* Or Step (packet.data==1) */
				/* Or Stepping (packet.data == 2) */
  else if (t == 'p')		/* Read program memory */
#ifndef EXTREMAL
    asm volatile ("push r30\n\t"
		  "push r31\n\t"
		  "mov r30, %A1\n\t"
		  "mov r31, %B1\n\t"
		  "lpm\n\t"
		  "mov %0,r0\n\t"
		  "pop r31\n\t"
		  "pop r30"
		  : "=r" (pp->data)
		  : "r" (pp->u.addr));
#else
    asm volatile ("lpm\n\t"
		  "mov %0,r0\n\t"
		  : "=r" (pp->data)
		  : "z" (pp->u.addr));
#endif
  else
    return 'E';
  return pp->cmd;
}

void ret_from_gdb_break ();
void gdb_break ();

/* I don't use SIGNAL(SIG_OVERFLOW0) because it's a special function.
   __attribute__ ((naked)) seems better */

void _overflow0_ (void) __attribute__ ((naked));
void _overflow0_ (void)
{
  asm volatile
    (".global	gdb_break\n"
     "gdb_break:\t"
     "push	r27\n"
     "in	r27, __SREG__\n" /* Save SREG */
     "cli\n"			/* Disable interrupts. Needed for stepping. */
     "sts	regs+28, r28\n"
     "sts	regs+29, r29\n"
     "ldi	r28, lo8 (regs)\n"
     "ldi	r29, hi8 (regs)\n"
     "std	Y+32, r27\n"	/* Put SREG value to his plase */
     "pop	r27\n"		/* Restore previous r27 value */
     "std	Y+27, r27\n"	/* and save it */
     "std	Y+30, r30\n"
     "std	Y+31, r31\n"
     "ldi	r31,0\n"
     "ldi	r30,0\n"
     "push_loop:\n"
     "ld	r27, Z+\n"	/* save registers from 0 to 26 */
     "st	Y+, r27\n"
     "cpi	r30,27\n"
     "brne	push_loop\n"
     "pop	r27\n"		/* pop high byte of ret address hi8(pm(pc)) */
     "pop	r26\n"		/* pop low byte of ret address lo8(pm(pc)) */
     "cpi	r27, hi8 (pm (_overflow0_))\n"
     "ldi	r24, lo8 (pm (_overflow0_))\n"
     "cpc	r26, r24\n"
     "brne	__no_self_trace\n"
     "pop	r27\n"		/* pop high byte of ret address hi8(pm(pc)) */
     "pop	r26\n"		/* pop low byte of ret address lo8(pm(pc)) */
     "__no_self_trace:\t"
     "std	Y+35-27, r26\n"	/* save return address (PC) */
     "std	Y+36-27, r27\n"
     "in	r30, __SP_L__\n"
     "in	r31, __SP_H__\n"
     "std	Y+33-27, r30\n"	/* save SP */
     "std	Y+34-27, r31\n"
     "clr	__zero_reg__\n"
     "ldi	r30, lo8 (mon_stack+19)\n"  /* last byte in mon_stack */
     "ldi	r31, hi8 (mon_stack+19)\n"
     "out	__SP_L__, r30\n" /* setup internal monitor stack */
     "out	__SP_H__, r31");

  /* This C text expands to IDEAL assembler equivalent
     I'm happy ;-) */
  if (packet.data == 2)
    {
      char i;
      void *pc = regs.pc;
      void **bp = regs.breakpoint;
    
      for (i = 7; i >= 0; --i)
	if (*bp++ == pc)
	  goto exec_mon;	/* Bad style. but code quality more costly */
      goto next_step;
    }
  exec_mon:
    _monitor_ ();
  next_step:
  asm volatile
    ("ldi	r31,0\n"
     "ldi	r30,0\n"
     "ldi	r28, lo8 (regs)\n"
     "ldi	r29, hi8 (regs)\n"
     "pop_loop:\n"
     "ld	r27, Y+\n"
     "st	Z+, r27\n"
     "cpi	r30, 27\n"
     "brne	pop_loop\n"
     "ldd	r30, Y+6\n"
     "ldd	r31, Y+7\n"
     "out	__SP_L__, r30\n"
     "out	__SP_H__, r31\n"
     "ldd	r27, Y+8\n"
     "push	r27\n"
     "ldd	r27, Y+9\n"
     "push	r27\n"
     "ld	r27, Y\n"
     "ldd	r30, Y+3\n"
     "push	r30\n"		/* real value of r30 */
     "ldd	r30, Y+5\n"	/* r30 = regs.sreg */
     "lds	r31, packet+1\n" /* r31 = packet.data */
     "tst	r31\n"
     "ldd	r31, Y+4\n"
     "ldd	r28, Y+1\n"
     "lds	r29, regs+29\n"
     "brne	_stepping_");
  asm volatile
    ("out	__SREG__, r30\n"
     "pop	r30\n"
     "ret");
  asm volatile ("_stepping_:push r31");
  
  asm volatile
    ("in	r31, %0\n"
     "ori	r31, %1\n"
     "out	%0, r31" :: "I" (TIMSK), "I" (BV (TOIE0)));
  asm volatile			/* set timer 0 counter */
    ("ldi	r31,-4\n"
     "out	%0, r31" :: "I" (TCNT0));
  asm volatile			/* start timer 0 clocks */
    ("ldi	r31, %0\n"
     "out	%1, r31" :: "I"(TIMER0_CK), "I"(TCCR0));
  asm volatile
    ("pop	r31\n"
     "andi	r30,0x7f\n"	/* clear I flag */
     "out	__SREG__, r30\n"
     "pop	r30\n"
     "reti");
}

void _monitor_ (void)
{
  struct packet_s * pp;
  SendByte (0xfc);	 /* Send this byte for switching to monitor mode */
  SendByte (lo8 (&regs));
  SendByte (hi8 (&regs));
  pp = &packet;
  do
    {
      /* Receive command from PC */
      pp->cmd = RecvByte ();
      pp->data = RecvByte ();
      pp->u.raw.lo = RecvByte ();
      pp->u.raw.hi = RecvByte ();

      pp->cmd = parse_packet (pp);

      SendByte (pp->cmd);	/* Sending back */
      SendByte (pp->data);
      SendByte (pp->u.raw.lo);
      SendByte (pp->u.raw.hi);
    } while (pp->cmd != 'q');
}
