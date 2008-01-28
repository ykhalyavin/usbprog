/*
 * Copyright (C) 2007 Nico van Leeuwen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdint.h>
#include <avr/io.h>

/*
___________________usbprogPIC pinouts_________________
*	ISCP-func		|	Adapter-conn	|	usbprog-conn
*-----------------------------------------------------------------------	
*	MCLR		|		1		|	5
*	5V			|		2		|	n/a
*	GND			|		3		|	n/a
*	ICSP_DATA	|		4		|	1 - MOSI	
*	ICSP_CLK		|		5		|	7 - SCK		
*/

#define RESET_PIN   PB0
#define RESET_PORT  PORTB
#define LED_PIN     PA4
#define LED_PORT    PORTA

#define LED_on     (LED_PORT   |=  (1 << LED_PIN))   // red led
#define LED_off    (LED_PORT   &= ~(1 << LED_PIN))
#define RESET_high (RESET_PORT |=  (1 << RESET_PIN))
#define RESET_low  (RESET_PORT &= ~(1 << RESET_PIN)) // reset

#define PIN(x)            (1 << (x))
#define SETPIN(addr,x)    (addr |= PIN(x))
#define CLEARPIN(addr,x)  (addr &= ~PIN(x))

#define PIN_DDR    DDRB
#define PIN_READ   PINB
#define PIN_WRITE   PORTB

/** D E F I N I T I O N S ****************************************************/
#define MAJORVERSION    2
#define MINORVERSION    10
#define DOTVERSION      0

#define BUF_SIZE        64			// USB buffers

//#define SCRIPTBUF_SIZE	768			// Script buffer size
#define SCRIPTBUF_SIZE	128			// Script buffer size
#define SCRIPT_ENTRIES	32			// Script Table entries
#define SCRIPT_MAXLEN	61			// maximum length of script
#define SCRIPTRSV_SIZE	0			// size of reserved memory at end of script buffer (may be used for canned scripts)
#define SCRIPTBUFSPACE	(SCRIPTBUF_SIZE - SCRIPTRSV_SIZE)

#define DOWNLOAD_SIZE	256			// download buffer size
#define UPLOAD_SIZE		128			// upload buffer size

// CONFIG2L definitions
#define cfg2l_address   0x300002
#define cfg2l_mask      0x06        // CONFIG2L & cfg2l_mask = 0 if BOR bits are correct

// PIC18F2550 EEPROM MAP
// Voltage calibration EEPROM locations
#define UNIT_ID         0xF0    // Locations 0xF0 - 0xFF reserved for Unit ID.

/** V A R I A B L E S ********************************************************/

typedef unsigned char byte;
typedef unsigned char bool;

struct {
    unsigned int	write_index;        // buffer write index
    unsigned int	read_index;			// buffer read index
    unsigned int    used_bytes;         // # bytes in buffer
} downloadbuf_mgmt; 

struct {
    unsigned int	write_index;		// buffer write index
    unsigned int	read_index;			// buffer read index
    unsigned int    used_bytes;         // # bytes in buffer
} uploadbuf_mgmt; 

union {		// Status bits
	struct {
		byte	StatusLow;
		byte	StatusHigh;
	};
	struct{
		// StatusLow
		unsigned VddGNDOn:1;	// bit 0
		unsigned VddOn:1;
		unsigned VppGNDOn:1;
		unsigned VppOn:1;
		unsigned VddError:1;
		unsigned VppError:1;
        unsigned ButtonPressed:1;
		unsigned :1;
		//StatusHigh
        unsigned Reset:1;       // bit 0
		unsigned UARTMode:1;			
        unsigned ICDTimeOut:1;
		unsigned UpLoadFull:1;
		unsigned DownloadEmpty:1;
		unsigned EmptyScript:1;
		unsigned ScriptBufOvrFlow:1;
		unsigned DownloadOvrFlow:1;
	};
} usbprogPICstatus;

extern char inbuf[BUF_SIZE];
extern char outbuf[BUF_SIZE];

extern byte icsp_pins;
extern byte icsp_baud;
extern byte aux_pin;

extern byte *uc_ScriptBuf_ptr;
extern byte uc_script_buffer[SCRIPTBUF_SIZE];		// Script Buffer
extern byte uc_download_buffer[DOWNLOAD_SIZE];	// Download Data Buffer
extern byte uc_upload_buffer[UPLOAD_SIZE];		// Upload Data Buffer

/** P R I V A T E  P R O T O T Y P E S ***************************************/
//void UARTModeService(void);
//void EnterUARTMode(byte *usbindex);
//void ExitUARTMode(void);

/** P U B L I C  P R O T O T Y P E S *****************************************/
extern void usbprogPICInit(void);

//void EnterBootloader(void);
extern void GetFWVersion(void);

extern byte ReadDownloadBuffer(void);
extern void WriteDownloadDataBuffer(byte *usbindex);
extern void WriteByteDownloadBuffer(byte DataByte);
extern void WriteUploadBuffer(byte byte2write);
extern void ReadUploadDataBuffer(void);
extern void ReadUploadDataBufferNoLength(void);

extern void ShortDelay(byte count);
extern void LongDelay(byte count);

extern void GetStatus(void);
extern void GetVddVpp(void);


