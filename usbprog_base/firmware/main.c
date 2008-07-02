#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#define F_CPU 16000000
#include <util/delay.h>

#include "usbn2mc.h"

// Changes: Deleted all the UART stuff. it's just needed for debugging.

// Fixed bug discussed in threat: "USBProg doesn't save Firmware"
// by setting PD0 internal pull-up


// __heap_start is declared in the linker script

/* external interrupt from usbn9604 */
ISR (INT0_vect)
{
  USBNInterrupt ();
}

// Values of state
#define NONE		0x00

#define STARTAPP	0x01
#define WRITEPAGE	0x02
// #define GETVERSION   0x03
// #define SETVERSION   0x04
#define STOPPROGMODE	0x05

// USB device parameters
const unsigned char avrupdateDevice[] = {
  0x12,				// 18 length of device descriptor
  0x01,				// descriptor type = device descriptor 
  0x10, 0x01,			// version of usb spec. ( e.g. 1.1) 
  0x00,				// device class
  0x00,				// device subclass
  0x00,				// protocol code
  0x08,				// deep of ep0 fifo in byte (e.g. 8)
  0x81, 0x17,			// vendor id
  0x62, 0x0c,			// product id
  0x00, 0x00,			// revision id (e.g 1.02)
  0x01,				// index of manuf. string
  0x02,				// index of product string
  0x00,				// index of ser. number
  0x01				// number of configs
};

// configuration descriptor          
const unsigned char avrupdateConf[] = {
  0x09,				// 9 length of this descriptor
  0x02,				// descriptor type = configuration descriptor 
  0x19, 0x00,			// total length with first interface ... 
  0x01,				// number of interfaces
  0x01,				// number if this config. ( arg for setconfig)
  0x00,				// string index for config
  0xA0,				// attrib for this configuration ( bus powerded, remote wakup support)
  0x1A,				// power for this configuration in mA (e.g. 50mA)

//InterfaceDescriptor
  0x09,				// 9 length of this descriptor
  0x04,				// descriptor type = interface descriptor 
  0x00,				// interface number 
  0x00,				// alternate setting for this interface 
  0x01,				// number endpoints without 0
  0x00,				// class code 
  0x00,				// sub-class code 
  0x00,				// protocoll code
  0x00,				// string index for interface

//EP2 Descriptor
  0x07,				// length of ep descriptor
  0x05,				// descriptor type= endpoint
  0x02,				// endpoint address (e.g. out ep2)
  0x02,				// transfer art ( bulk )
  0x40, 0x00,			// fifo size
  0x00				// polling intervall in ms
};

uint16_t page_addr;
uint16_t page_addr_w;
uint8_t pageblock[128];
uint8_t collect128;
uint8_t state;
uint8_t address EEMEM = 1;

/* usbn2mc tiny needs this */
void
USBNDecodeVendorRequest (DeviceRequest * req)
{
}

void
USBNDecodeClassRequest (DeviceRequest * req, EPInfo * ep)
{
}

/* pointer to the beginning of application code */
void (*avrupdate_jump_to_app) (void) = 0x0000;

/*  wait function */
void
wait_ms (int ms)
{
  uint16_t i;
  for (i = 0; i < ms; i++)
    _delay_ms (1);
}

/* pogramm a page into flash 
 *	@page = number of page
 *	global pageblock = data
 */
void
avrupdate_program_page (uint32_t page)
{
  uint16_t i;
  uint8_t sreg;

  //SendHex(page);
  page = page * 128;
  sreg = SREG;
  cli ();

  eeprom_busy_wait ();

  boot_page_erase (page);
  boot_spm_busy_wait ();	// Wait until the memory is erased.
  uint8_t wbufaddr = 0;

  for (i = 0; i < SPM_PAGESIZE; i += 2)
    {
      // Set up little-endian word.
      uint16_t w = pageblock[wbufaddr++];
      w += pageblock[wbufaddr++] << 8;
      boot_page_fill (page + i, w);
    }

  boot_page_write (page);	// Store buffer in flash page.
  boot_spm_busy_wait ();	// Wait until the memory is written.

  // Reenable RWW-section again. We need this if we want to jump back
  // to the application after bootloading.

  boot_rww_enable ();

  // Re-enable interrupts (if they were ever enabled).
  SREG = sreg;
}

/* called when Data was received via USB*/
void
avrupdate_cmd (char *buf)
{
  cli ();			// disable Interrupts
  uint8_t i;

  // check state 
  if (state == WRITEPAGE)
    {
      // if page sizte= 128 collect two 64 packages to a 128
      //UARTWrite("128\r\n");
      //SendHex(page_addr);
      if (page_addr % 2)
	{
	  collect128 = 0;
	  //UARTWrite("odd\r\n");
	  // get sescond package
	  if (page_addr == 1)
	    page_addr_w = 0;
	  else
	    page_addr_w = (page_addr - 1) / 2;

	  for (i = 0; i < 64; i++)
	    {
	      pageblock[i + 64] = buf[i];
	    }

	  // write page
	  avrupdate_program_page (page_addr_w);
	  state = NONE;
	}
      else
	{
	  collect128 = 1;
	  //UARTWrite("even\r\n");
	  // get first package 
	  for (i = 0; i < 64; i++)
	    pageblock[i] = buf[i];

	  state = NONE;
	}
    }
  else
    {
      state = buf[0];
      if (state == WRITEPAGE){
	page_addr  = buf[1];
	page_addr |= buf[2] << 8;
      }
      if (state == STARTAPP)
	{
	  if (collect128)
	    {
	      // if just half a page (64bytes) were received, write that half page now
	      page_addr = page_addr / 2;
	      avrupdate_program_page (page_addr);
	    }

	  // switch to run app mode
	  USBNWrite (RXC1, FLUSH);
	  USBNWrite (RXC1, RX_EN);

	  USBNWrite (MCNTRL, SRST);	// clear all usb registers

	  GICR = _BV (IVCE);	// enable wechsel der Interrupt Vectoren
	  GICR = 0x00;		// Interrupts auf Application Section umschalten
	  sei ();

	  // Reenable flash RWW-section again, where new application was written to
	  boot_rww_enable ();

	  avrupdate_jump_to_app ();
	}
      if (state == STOPPROGMODE)
	{
	  // section is not used (bootloader could be restarted)
	}
    }
  sei ();			// enable Interrupts again
}

/* main program */
int
main (void)
{
  // Initialize
  DDRA = 0x00;			// First all pins input
  DDRA |= (1 << PA4);		// then configure PIN with red LED as output

  DDRD = 0x00;			// Configure all pins as input
  DDRB = 0x00;
  DDRC = 0x00;

  PORTD |= (1 << PD1);		// TXD/PD1: Set internal pull-up
  PORTD |= (1 << PD0);		// RXD/PD0: Set internal pull-up
  wait_ms (1);			// wait for PD1 to be pulled up

  if (bit_is_clear (PIND, PD1))	// Check for wrong Jumper Setting; Jumper between TXD and GND
    goto wrong_jumper_setting;


  // Start bootloader?
  // First condition
  if (bit_is_set (PINA, PA4))	// update-mode request by update-tool -> LED is turned on
    goto start_update_mode;

  // or second condition
  if (eeprom_read_byte (&address) == 0x01)
    {				// update-mode request by update-tool -> byte in eeprom = 0x01
      eeprom_busy_wait ();
      eeprom_write_byte (&address, 0x00);	// reset byte in eeprom

      goto start_update_mode;
    }

  PORTD &= ~(1 << PD1);		// TXD/PD1 -> low
  DDRD |= (1 << PD1);		// TXD/PD1 -> output
  wait_ms (1);			// wait for PD0 to be pulled down by jumper or not

  // or third condition
  if (bit_is_clear (PIND, PD0))	// check if Jumper is set between RX and TX of UART-Connector
    goto start_update_mode;

  DDRD &= ~(1 << PD1);		// TXD/PD1 als Ausgang festlegen
  PORTD |= (1 << PD1);		// TXD/PD1: Set internal pull-up

  // or forth condition
  if (pgm_read_byte ((void *) 0) == 0xFF)	// check for first test after bootloader is installed
    goto start_update_mode;	// no programm in flash memory


  // Jump to Application section
  avrupdate_jump_to_app ();


  // bootloader application starts here
start_update_mode:

  collect128 = 0;		// state of received data: one page of flashmemory has 128Bytes
  // 64bytes have to be send in two steps via USB

  // Init USB [usbn2mc.h]
  USBNInit (avrupdateDevice, avrupdateConf);
  USBNCallbackFIFORX1 (&avrupdate_cmd);	// avrupdate_cmd is called yb interrupt when Data has been received
  _USBNAddStringDescriptor ("");	//pseudo lang
  _USBNAddStringDescriptor ("USBprog EmbeddedProjects");
  _USBNAddStringDescriptor ("usbprogBase Mode");
  _USBNCreateStringField ();

  cli ();			//reset "Global Interrupt enable" [interrupt.h]
  GICR = _BV (IVCE);		//IVCE = 1, necessary to change Interrupt Vector, disables Interrupts
  GICR = _BV (IVSEL);		//IVSEL = 1, place Interrupt Vector at beginning of boot loader section
  sei ();			//set "Global Interrupt enable" [interrupt.h]

  USBNInitMC ();
  USBNStart ();			// start usb chip


  // endless loop
  while (1)
    {
      PORTA |= (1 << PA4);	// red LED: on-off-on-off--------
      wait_ms (100);
      PORTA &= ~(1 << PA4);
      wait_ms (100);
      PORTA |= (1 << PA4);
      wait_ms (100);
      PORTA &= ~(1 << PA4);
      wait_ms (800);
    }

wrong_jumper_setting:
  // endless loop
  while (1)
    {
      PORTA |= (1 << PA4);	// red LED: on-----off-
      wait_ms (500);
      PORTA &= ~(1 << PA4);
      wait_ms (100);
    }


}
