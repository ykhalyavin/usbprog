#include <inttypes.h>
#include <avr/io.h>

#include "uart.h"

#define F_CPU 16000000UL

void uart_init(uint32_t baudrate, char stopbits, char parity, char databits)
{
  uint16_t ubrr = (uint16_t) ((uint32_t) F_CPU/(16*baudrate) - 1);
  uint8_t ucsrc = (1 << URSEL);

  // num of stopbits (0=1, 1=1.5, 2=2)
  // we cannot support 1.5 stopbits
  if(stopbits == 2)
  {
    // 2 stopbits
    ucsrc |= (1 << USBS);
  }

  // parity (0=none, 1=odd, 2=even, 3=mark, 4=space)
  // we cannot support parity mark or space
  switch(parity)
  {
    case 1:
      // parity bit odd
      ucsrc |= (1 << UPM0 | 1 << UPM1);
      break;
    case 2:
      // parity bit even
      ucsrc |= (1 << UPM1);
      break;
    // default:
    // no parity bit UPM0 = 0, UPM1 = 0
  }
  
  //data bits (5,6,7,8 or 16)
  switch(databits)
  {
    case 5:
      ucsrc |= (0 << UCSZ0 | 0 << UCSZ1 | 0 << UCSZ2);
      break;
    case 6:
      ucsrc |= (1 << UCSZ0 | 0 << UCSZ1 | 0 << UCSZ2);
      break;
    case 7:
      ucsrc |= (0 << UCSZ0 | 1 << UCSZ1 | 0 << UCSZ2);
      break;
    default:
      // 8 databits
      ucsrc |= (1 << UCSZ0 | 1 << UCSZ1 | 0 << UCSZ2);
      break;
  }

  UBRRH = (uint8_t) (ubrr >> 8);
  UBRRL = (uint8_t) (ubrr);

  // Flush Receive-Buffer
  do
  {
    uint8_t dummy;
    (void) (dummy = UDR);
  }
  while (UCSRA & (1 << RXC));

  UCSRB = (1 << RXEN | 1 << TXEN);
  UCSRC = ucsrc;
}

/*
void uart_init(uint8_t ubrrh, uint8_t ubrrl, uint8_t stopbits, uint8_t parity, uint8_t databits)
{
  uint8_t ucsrc = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);

  UBRRH = ubrrh;
  UBRRL = ubrrl;

  // stopbits
  if(stopbits == 2)
    ucsrc = (1 << USBS);

  // parity
  switch(parity)
  {
    case 1: // odd
      ucsrc |= (1 << UPM1 | 1 << UPM0); break;
    case 2: // even
      ucsrc |= (1 << UPM1); break;
  }

  // databits
  switch(databits)
  {
    case 5:                                                   break;
    case 6:  ucsrc |= (1 << UCSZ0);                           break;
    case 7:  ucsrc |= (1 << UCSZ1);                           break;
    case 9:  ucsrc |= (1 << UCSZ0 | 1 << UCSZ1 | 1 << UCSZ2); break;
    default: ucsrc |= (1 << UCSZ0 | 1 << UCSZ1);              break; // 8 bits
  }

  // UART Receiver und Transmitter anschalten 
  // Data mode 8N1, asynchron 
  UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
  UCSRC = ucsrc;

  // Flush Receive-Buffer (entfernen evtl. vorhandener ungültiger Werte) 
  do { UDR; } while (UCSRA & (1 << RXC));

  UCSRA = (1 << RXC) | (1 << TXC);
}
*/


void uart_putchar(unsigned char sign)
{
  // bei neueren AVRs steht der Status in UCSRA/UCSR0A/UCSR1A, hier z.B. fuer ATmega16:
  while (!(UCSRA & (1<<UDRE))); /* warten bis Senden moeglich                   */
  	UDR = sign;                    /* schreibt das Zeichen x auf die Schnittstelle */
}


char uart_getchar(void)
{
    while (!(UCSRA & (1<<RXC)));  // warten bis Zeichen verfuegbar
  return UDR;                   // Zeichen aus UDR an Aufrufer zurueckgeben
}

void uart_puts(char *s)
{
  while(*s)
     uart_putchar(*s++);
}

char AsciiToHex(char high,char low)
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
    uart_putchar(high+48);
  else
    uart_putchar(high+87);
 
 
  if(low<=9)
    uart_putchar(low+48);
  else
    uart_putchar(low+87);

}

/*
void uartInit(ulong baudrate, uchar parity, uchar stopbits, uchar databits)
{
usbDWord_t   br;

    br.dword = F_CPU / (8L * baudrate) - 1;
	UCSR0A  |= (1<<U2X0);

#if DEBUG_LEVEL < 1
    //    USART configuration    
    UCSR0B  = 0;
    UCSR0C  = URSEL_MASK | ((parity==1? 3:parity)<<UPM00) | ((stopbits>>1)<<USBS0) | ((databits-5)<<UCSZ00);
    UBRR0L  = br.bytes[0];
    UBRR0H  = br.bytes[1];
#else
    DBG1(0xf0, br.bytes, 2);
#endif // DEBUG_LEVEL 

#ifdef USE_UART_RXD_INTERRUPT
    UCSR0B  = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);
#else
    UCSR0B  = (1<<RXEN0) | (1<<TXEN0);
#endif
}
*/

