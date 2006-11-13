#include <stdlib.h>
#include <avr/io.h>
#include <avr/boot.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#define F_CPU 16000000
#include <avr/delay.h>


#include "uart.h"


/* stage vars for avrupdate */

uint8_t state;
uint8_t page_addr;
uint8_t page_addr_w;
uint8_t pageblock[128];
uint8_t collect128;


#define NONE	    0x00
#define STARTAPP    0x01
#define WRITEPAGE   0x02
#define READCHKSUM  0x03


// pointer to the beginning of application code
void (*jump_to_app)( void ) = 0x0000;

void wait_ms(int ms)
{
	int i;
  	for(i=0;i<ms;i++)
    	_delay_ms(1);
}

void avrupdate_program_page (uint32_t page)
{
  uint16_t i;
  uint8_t sreg;

  //SendHex(page);
  page = page*128;
  sreg = SREG;
  cli();
    
  eeprom_busy_wait ();

  boot_page_erase (page);
  boot_spm_busy_wait ();      // Wait until the memory is erased.
  uint8_t wbufaddr = 0;

  for (i=0; i<SPM_PAGESIZE; i+=2)
  {
    // Set up little-endian word.
    uint16_t w = pageblock[wbufaddr++];
    w += pageblock[wbufaddr++] << 8;
    boot_page_fill (page + i, w);
  }

  boot_page_write (page);     // Store buffer in flash page.
  boot_spm_busy_wait();       // Wait until the memory is written.

  // Reenable RWW-section again. We need this if we want to jump back
  // to the application after bootloading.

  boot_rww_enable ();

  // Re-enable interrupts (if they were ever enabled).

  SREG = sreg;
}



// start programm from application sector
void avrupdate_runapp()
{
  //UARTWrite("start\r\n");
  if(collect128){
    //SendHex(page_addr);
    page_addr = page_addr/2;
    BootProgramPage (page_addr);
    UARTWrite("programm rest\r\n"); 
  }

  USBNWrite(RXC1,FLUSH);
  USBNWrite(RXC1,RX_EN);

  USBNWrite(MCNTRL,SRST);  // clear all usb registers

  GICR = _BV(IVCE);  // enable wechsel der Interrupt Vectoren
  GICR = 0x00; // Interrupts auf Application Section umschalten

  jump_to_app();	  // Jump to application sector
}


void avrupdate_update(char *buf)
{
  int i;
  // check state first ist 
  switch(state)
  {
    case WRITEPAGE:
      //UARTWrite("write\r\n");
      //SendHex(page_addr);
      
      // if page sizte= 128 collect two 64 packages to a 128
      if(SPM_PAGESIZE==0x80)
      {
	//UARTWrite("128\r\n");
	//SendHex(page_addr);
	if(page_addr%2)
	{
	  collect128=0;
	  //UARTWrite("odd\r\n");
	  // get sescond package
	  if(page_addr==1)
	    page_addr_w = 0;
	  else 
	    page_addr_w = (page_addr-1)/2;
	  
	  for(i=0;i<64;i++)
	  {
	    pageblock[i+64]=buf[i];
	  }
 
	  // write page
	  avrupdate_program_page (page_addr_w);
	  state = NONE;
	}else
	{
	  collect128=1;
	  //UARTWrite("even\r\n");
	  // get first package 
	  for(i=0;i<64;i++)
	    pageblock[i]=buf[i];

	  state = NONE;
	}	
      } else
      {
	UARTWrite("64\r\n");
	avrupdate_program_page (page_addr);
	state = NONE;
      }
    break;
    default:
      //UARTWrite("default\r\n");
      state = buf[0];
      if(state==WRITEPAGE)
	page_addr = buf[1];

      if(state==STARTAPP)
      {
	avrupdate_runapp();
	state = NONE;
      }

  }
	
}


