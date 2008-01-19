#include <inttypes.h>
#include <avr/io.h>

#include "uart.h"

#define DEBUG 1


void UARTInit(void)
{
  	//UCSRB |= (1<<TXEN);			// UART TX einschalten
  	//UCSRC |= (1<<URSEL)|(3<<UCSZ0);	// Asynchron 8N1
	//UCSRB |= ( 1 << RXEN ); // RX aktivieren
	//UCSRB |= ( 1 << RXCIE ); // RX interrupt aktivieren
	//UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
 	
	UCSRA = (1 << RXC) | (1 << TXC);
  	//UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
  	UCSRB = (1 << RXEN) | (1 << TXEN) ;
  	UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
	
	//ATmega32 bei 16MHz und für 19200 Baud
	// 4 mhz 9600 baud =25 
  	UBRRH  = 0;                                   // Highbyte ist 0
  	//UBRRL  = 25;                                  // Lowbyte ist 51 ( dezimal )
  	//UBRRL  = 51;                                  // Lowbyte ist 51 ( dezimal )
  	UBRRL  = 103;                                  // Lowbyte ist 51 ( dezimal )
    // Flush Receive-Buffer
  
  	do
  	{	
		uint8_t dummy;
      	(void) (dummy = UDR);
  	}
  	while (UCSRA & (1 << RXC));
}



void UARTPutChar(unsigned char sign)
{
	#if DEBUG
  	// bei neueren AVRs steht der Status in UCSRA/UCSR0A/UCSR1A, hier z.B. fuer ATmega16:
  	while (!(UCSRA & (1<<UDRE))); /* warten bis Senden moeglich                   */
  		UDR = sign;                    /* schreibt das Zeichen x auf die Schnittstelle */
	#endif
}


unsigned char UARTGetChar(void)
{
	#if DEBUG
    while (!(UCSRA & (1<<RXC)));  // warten bis Zeichen verfuegbar
  		return UDR;                   // Zeichen aus UDR an Aufrufer zurueckgeben
	#endif
}

void UARTWrite(char* msg)
{
	#if DEBUG
  	while(*msg != '\0')
  	{
     	UARTPutChar (*msg++);
  	}
	#endif
}

unsigned char AsciiToHex(unsigned char high,unsigned char low)
{
  	unsigned char new;

  	// check if lower equal 9 ( assii 57 )
  	if(high <= 57) // high is a number
    	high = high -48;
  	else // high is a letter
    	high = high -87;

  	high = high << 4;
  	high = high & 0xF0;
 
  	// check if lower equal 9 ( assii 57 )
  	if(low <= 57) // high is a number
    	low = low -48;
  	else // high is a letter
    	low = low -87;
  	
	low = low & 0x0F;
 
  	new = high | low;
 
  	return new;
}

void SendHex(unsigned char hex)
{
  	unsigned char high,low;
  	// get highnibble
  	high = hex & 0xF0;
  	high = high >> 4;
 
  	// get lownibble
  	low = hex & 0x0F;
 
  	if(high<=9)
    	UARTPutChar(high+48);
  	else
    	UARTPutChar(high+87);
 
 
  	if(low<=9)
    	UARTPutChar(low+48);
  	else
    	UARTPutChar(low+87);

}

