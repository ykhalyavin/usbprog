#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>

#define F_CPU 16000000
#include <util/delay.h>

#include "uart.h"

#include "utils.h"

#include "at89.h"



void ATMegaSPIOut(char data)
{
  SPDR = data;
  while ( !(SPSR & (1<<SPIF)) ) ;
}


void ATMegaWriteCode(int addr, char data)
{
  ATMegaSPIOut(0x02 | ((addr >> 5) & 0xF8) | ((addr >> 11) & 0x04)); /* hhhh h010 */
  ATMegaSPIOut(addr & 0xFF); /* llll llll */
  ATMegaSPIOut(data);
  wait_ms(3);
}


void ATMegaFlashWrite(char *buf)
{
  int startaddr;
  //UARTWrite("start upload\r\n");
  // hex file to spi interface
  DDRB=0xff;
  
  SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1);

  PORTB=0x10;  // reset on led on and sck = low 
  wait_ms(5);
  PORTB=0x00;  // reset on led on and sck = low 
  wait_ms(5);
  PORTB=0x10;  // reset on led on and sck = low 


  //before the at89 accepts any commands, it needs to be put into command mode
  ATMegaSPIOut(0xAC);
  ATMegaSPIOut(0x53);
  ATMegaSPIOut(0x00);
  wait_ms(9);

  //74 00 f5 90  7a ff 7b 14  db fe da fa  04 80 f3
  //load programm into flash
  int i; 
  startaddr = (int)buf[1]; //high 
  startaddr = startaddr << 8;//high 
  startaddr = startaddr + (int)buf[2]; // low

  for(i=0;i<(int)buf[3];i++)
  {
    ATMegaWriteCode(startaddr+i,buf[i+4]); 
    //SendHex(startaddr+i);
    SendHex(buf[i+4]);
    //USBNDebug("\r\n");
  }
    //USBNDebug("\r\n");


  wait_ms(9);
  PORTB=0x02;
  //UARTWrite("ready...\r\n");

}
void ATMegaFlashErase()
{
  // hex file to spi interface
  DDRB=0xff;
  
  SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1);

  PORTB=0x10;  // reset on led on and sck = low 
  wait_ms(5);
  PORTB=0x00;  // reset on led on and sck = low 
  wait_ms(5);
  PORTB=0x10;  // reset on led on and sck = low 


  //before the at89 accepts any commands, it needs to be put into command mode
  ATMegaSPIOut(0xAC);
  ATMegaSPIOut(0x53);
  ATMegaSPIOut(0x00);
  wait_ms(9);

  SPDR = 0xAC;
  ATMegaSPIOut(0xAC);
  ATMegaSPIOut(0x04);
  ATMegaSPIOut(0x00);
  wait_ms(16);
  
  PORTB=0x02;
  UARTWrite("ready...\r\n");

}
