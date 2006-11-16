#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <stdint.h>
#include <avr/interrupt.h>
//#include <avr/signal.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#define F_CPU 16000000
#include <util/delay.h>
#include <avr/eeprom.h>

// EEMEM wird bei aktuellen Versionen der avr-lib in eeprom.h definiert
// hier: definiere falls noch nicht bekannt ("alte" avr-libc)
#ifndef EEMEM
// alle Textstellen EEMEM im Quellcode durch __attribute__ ... ersetzen
#define EEMEM  __attribute__ ((section (".eeprom")))
#endif

#include "uart.h"
#include "usbn2mc.h"

/* external interrupt from usbn9604 */
SIGNAL(SIG_INTERRUPT0)
{
  	USBNInterrupt();
}

uint8_t state;
uint8_t page_addr;
uint8_t page_addr_w;
uint8_t pageblock[128];
uint8_t collect128;

#define NONE	    0x00
#define STARTAPP    0x01
#define WRITEPAGE   0x02
#define READCHKSUM  0x03

/* usbn2mc tiny needs this */
void USBNInterfaceRequests(DeviceRequest *req,EPInfo* ep){}
void USBNDecodeVendorRequest(DeviceRequest *req){}
void USBNDecodeClassRequest(DeviceRequest *req){}



/* pointer to the beginning of application code */
void (*avrupdate_jump_to_app)( void ) = 0x0000;

/*  wait function */
void wait_ms(int ms)
{
  	int i;
  	for(i=0;i<ms;i++)
    	_delay_ms(1);
}

/* pogramm a page into flash 
 *	@page = number of page
 *	global pageblock = data
 */
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
void avrupdate_start_app()
{
  	//UARTWrite("start\r\n");
  	if(collect128){
    	//SendHex(page_addr);
    	page_addr = page_addr/2;
    	avrupdate_program_page (page_addr);
    	UARTWrite("programm rest\r\n"); 
  	}

  	USBNWrite(RXC1,FLUSH);
  	USBNWrite(RXC1,RX_EN);

  	USBNWrite(MCNTRL,SRST);  // clear all usb registers

  	GICR = _BV(IVCE);  // enable wechsel der Interrupt Vectoren
  	GICR = 0x00; // Interrupts auf Application Section umschalten

  	avrupdate_jump_to_app();	  // Jump to application sector
}


void avrupdate_cmd(char *buf)
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
				//cli();
				avrupdate_start_app();
				state = NONE;
   			}	
	}
	
}

// __heap_start is declared in the linker script


int main(void)
{

	/* if is no program in flash start bootloader, else start programm */
	uint8_t eeFooByte EEMEM = 1;
	uint8_t myByte;
	myByte = eeprom_read_byte(&eeFooByte);

	if(pgm_read_byte(0)!=0xFF && myByte !=0x77)
		avrupdate_start_app();

  	// spm (bootloader mode from avr needs this, to use an own isr table)	
  	cli();
  	GICR = _BV(IVCE);  // enable wechsel der Interrupt Vectoren
  	GICR = _BV(IVSEL); // Interrupts auf Boot Section umschalten
  	sei();


  	// bootloader application starts here

  	const unsigned char avrupdateDevice[] =
  	{ 
  	0x12,	      // 18 length of device descriptor
    0x01,       // descriptor type = device descriptor 
    0x10,0x01,  // version of usb spec. ( e.g. 1.1) 
    0x00,	      // device class
    0x00,	      // device subclass
    0x00,       // protocol code
    0x08,       // deep of ep0 fifo in byte (e.g. 8)
    0x00,0x04,  // vendor id
    0x5d,0xc3,  // product id
    0x03,0x01,  // revision id (e.g 1.02)
    0x00,       // index of manuf. string
    0x00,	      // index of product string
    0x00,	      // index of ser. number
    0x01        // number of configs
  	};

  	// ********************************************************************
  	// configuration descriptor          
  	// ********************************************************************

  	const unsigned char avrupdateConf[] =
  	{ 
	0x09,	      // 9 length of this descriptor
    0x02,       // descriptor type = configuration descriptor 
    0x20,0x00,  // total length with first interface ... 
    0x01,	      // number of interfaces
    0x01,	      // number if this config. ( arg for setconfig)
    0x00,       // string index for config
    0xA0,       // attrib for this configuration ( bus powerded, remote wakup support)
    0x1A,        // power for this configuration in mA (e.g. 50mA)
    //InterfaceDescriptor
    0x09,	      // 9 length of this descriptor
    0x04,       // descriptor type = interface descriptor 
    0x00,	      // interface number 
    0x00,	      // alternate setting for this interface 
    0x02,	      // number endpoints without 0
    0x00,       // class code 
    0x00,       // sub-class code 
    0x00,       // protocoll code
    0x00,       // string index for interface
    //EP1 Descriptor
    0x07,	      // length of ep descriptor
    0x05,	      // descriptor type= endpoint
    0x81,	      // endpoint address (e.g. in ep1)
    0x02,	      // transfer art ( bulk )
    0x80,0x00,  // fifo size
    0x00,	      // polling intervall in ms
    //EP2 Descriptor
    0x07,	      // length of ep descriptor
    0x05,	      // descriptor type= endpoint
    0x02,	      // endpoint address (e.g. out ep2)
    0x02,	      // transfer art ( bulk )
    0x00,0x08,  // fifo size
    0x00	      // polling intervall in ms
  	};

  	UARTInit();
  
  
  
  	USBNInit(avrupdateDevice,avrupdateConf);   
  	USBNCallbackFIFORX1(&avrupdate_cmd);

  	USBNInitMC();

  	// start usb chip
  	USBNStart();
  
  	UARTWrite("\r\nbootloader is now active\r\n");

  	collect128=0;
  	// wait 2 seconds then start application

	while(1);	

	/*
  		wait_ms(2000);
  		UARTWrite("\r\nbootloader start app now");
  		avrupdate_start_app();
  		while(1);
	*/ 
}

