#include <inttypes.h>
#include <avr/io.h>
#include <avr/delay.h>

#include "uart.h"
#define F_CPU 16000000ULL

void wait_ms(unsigned int i) {
  while(i--)
    _delay_ms(1);
}

void UARTInit(unsigned int baud0, unsigned int baud1, char stopbits, char parity, char databits)
{
  // unsigned int32
  unsigned long long int baudrate;
  unsigned char ubrrh;
  unsigned char ubrrl;
  unsigned char ucsrc = (1 << URSEL);

  PORTA |= (1<<PA4);  //on
  wait_ms(100);
  PORTA &= ~(1<<PA4); //off
  wait_ms(100);


  // baudrate berechnen
  baudrate = (baud0 + (baud1 << 8));
  ubrrh = (unsigned char) (((F_CPU/16ULL/baudrate)-1ULL) >> 8);
  ubrrl = (unsigned char)  ((F_CPU/16ULL/baudrate)-1ULL);
  

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
      //ucsrc |= (0 << UCSZ0 | 0 << UCSZ1 | 0 << UCSZ2);
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

  UCSRA = (1 << RXC) | (1 << TXC);
  UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
  UBRRH = ubrrh;
  UBRRL = ubrrl;
  UCSRC = ucsrc;

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
  // bei neueren AVRs steht der Status in UCSRA/UCSR0A/UCSR1A, hier z.B. fuer ATmega16:
  while (!(UCSRA & (1<<UDRE))); /* warten bis Senden moeglich                   */
  	UDR = sign;                    /* schreibt das Zeichen x auf die Schnittstelle */
}


unsigned char UARTGetChar(void)
{
    while (!(UCSRA & (1<<RXC)));  // warten bis Zeichen verfuegbar
  return UDR;                   // Zeichen aus UDR an Aufrufer zurueckgeben
}

void UARTWrite(char* msg)
{
  while(*msg != '\0')
  {
     UARTPutChar (*msg++);
  }
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

