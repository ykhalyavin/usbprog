/* Copyright (C) 1999 by Denis Chertykov (denis@overta.ru)

   You can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version. */

struct packet_s
{
  unsigned char cmd;		/* command letter */
  unsigned char data;		/* data for operation */
  union addr_u
  {
    unsigned short addr;		/* operation address */
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
