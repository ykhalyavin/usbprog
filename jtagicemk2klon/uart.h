#ifndef UART_H
#define UART_H

#include "jtag_avr_defines.h"

#ifdef DEBUG_ON
void UARTInit(void);
void UARTPutChar(unsigned char sign);
unsigned char UARTGetChar(void);
void UARTWrite(const char* msg);

extern unsigned char debug_verbose;

unsigned char AsciiToHex(unsigned char high,unsigned char low);
void SendHex(unsigned char hex);
#endif
#endif



