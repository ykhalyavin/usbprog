/*********************************************************************
 *
 *                  usbprogPIC firmware
 *		(c) copyright 2007 by N.C. van Leeuwen
 *
 *********************************************************************

 ***************************************************************************/

/** I N C L U D E S **********************************************************/
#define F_CPU 16000000
#include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "usbn2mc.h"

/* PIC specific interface functions */
#include "usbprogPIC.h"
#include "pk_comm.h"
#include "utils/eeprom/eeprom.h"
#include "utils/icsp/icsp.h"
#include "utils/scripts/scripts.h"
#include "utils/auxpin/auxpin.h"

/** D E C L A R A T I O N S **************************************************/
byte uc_script_buffer[SCRIPTBUF_SIZE];		// Script Buffer
byte 	uc_download_buffer[DOWNLOAD_SIZE];	// Download Data Buffer
byte 	uc_upload_buffer[UPLOAD_SIZE];		// Upload Data Buffer
byte 	*uc_ScriptBuf_ptr;
/** E X T E R N S ***************************************************/

char inbuf[BUF_SIZE];
char outbuf[BUF_SIZE];

byte icsp_pins;
byte icsp_baud;
byte aux_pin;


struct {
	byte CCPRSetPoint;
	byte UppperLimit;
	byte LowerLimit;
} Vpp_PWM;


/******************************************************************************
 * Function:        void usbprogPICInit(void)
 *****************************************************************************/
extern void 
usbprogPICInit(void)
{
    byte i;                         // index variable
	*uc_ScriptBuf_ptr = &(uc_script_buffer[0]);

	// ------
    // ICSP pins init function
    // ------
	
	//ICSP_DATA pin init
    PIN_DDR &= ~(1 << ICSP_DAT_PIN); 	//Set to input
	PIN_WRITE &= ~(1 << ICSP_DAT_PIN);	//Tristated
    PIN_WRITE &= ~(1<<ICSP_DAT_PIN);	// initialize output port to 0 (low)
   
    PIN_DDR &= ~(1 << ICSP_CLK_PIN); 	//Set to input
	PIN_WRITE &= ~(1 << ICSP_CLK_PIN);	//Tristated
    PIN_WRITE |= (1<<ICSP_CLK_PIN);	  	// initialize output port to 0 (low)

    //tris_AUX = 1;                   // RA4 Input (tristate)
    //AUX = 0;                        // initialize output latch to 0 (low)

    //MCLR_TGT = 0;                   // initialize MCLR_TGT off (no MCLR_TGT)
    //tris_MCLR_TGT = 0;              // RA5 Output

    //-------------------------
    // initialize variables
    //-------------------------

	usbprogPICstatus.StatusLow = 0;		// init status.
	usbprogPICstatus.StatusHigh = 1;    // set reset bit.

	icsp_pins = 0x03;					// default inputs
	icsp_baud = 0x00;					// default fastest
    //aux_pin = 0x01;         // default input

    for (i=0; i<63; i++) {   		// initialize input and output buffer to 0
        inbuf[i]=0;
        outbuf[i]=0;
    }

	ClearScriptTable();     			// init script table to empty.

    downloadbuf_mgmt.write_index = 0;   // init buffer to empty
    downloadbuf_mgmt.read_index = 0;
    downloadbuf_mgmt.used_bytes = 0;

    uploadbuf_mgmt.write_index = 0;     // init buffer to empty
    uploadbuf_mgmt.read_index = 0;
    uploadbuf_mgmt.used_bytes = 0; 

	cli();								//Interrupts off	
	
    // configure Timer0 (used by SCRIPT_ENGINE)

	TCCR1B = 0x00; 						// Timer0 off, 16-bit timer, 
										// internal clock, prescaler not assigned
	TCNT1H = 0x00;
	TCNT1L = 0x00;
	
	sei();								//Interrupts on

}

/******************************************************************************
 * Function:        void ReadUploadDataBuffer(void)
 * Overview:        Sends data from upload data buffer over USB.
 *
 * Input:           None
 * Output:          Transmits HID Tx report with data length and data.
 *****************************************************************************/
extern void 
ReadUploadDataBuffer(void)
{
    byte i, length;

    length = uploadbuf_mgmt.used_bytes;
    if (length > (BUF_SIZE - 1))        // limited to # bytes in USB report - length byte
    {
        length = (BUF_SIZE - 1);
    }

	outbuf[0] = length;
    for (i = 1; i<= length; i++)
    {
	    outbuf[i] = uc_upload_buffer[uploadbuf_mgmt.read_index++];
        if (uploadbuf_mgmt.read_index >= UPLOAD_SIZE)  // manage buffer wrap.
        {
            uploadbuf_mgmt.read_index = 0;
        }

    }

    uploadbuf_mgmt.used_bytes -= length;    // read out this many bytes.

    // transmit data
} 


/******************************************************************************
 * Function:        void ReadUploadDataBufferNoLength(void)
 * Overview:        Sends data from upload data buffer over USB,
 *                  but does not add a length byte.
 *
 * Input:           None
 * Output:          Transmits HID Tx report with data only.
 *****************************************************************************/
extern void 
ReadUploadDataBufferNoLength(void)
{
    byte i, length;

    length = uploadbuf_mgmt.used_bytes;
    if (length > (BUF_SIZE))        // limited to # bytes in USB report
    {
        length = (BUF_SIZE);
    }

    for (i = 0; i < length; i++)
    {
	    outbuf[i] = uc_upload_buffer[uploadbuf_mgmt.read_index++];
        if (uploadbuf_mgmt.read_index >= UPLOAD_SIZE)  // manage buffer wrap.
        {
            uploadbuf_mgmt.read_index = 0;
        }

    }

    uploadbuf_mgmt.used_bytes -= length;    // read out this many bytes.

    // transmit data
}




/******************************************************************************
 * Function:        void WriteDownloadDataBuffer(byte *usbindex)
 * Overview:        Writes a given # of bytes into the data download buffer.
 *
 * Input:           *usbindex - index to length of data in USB buffer
 * Output:          uc_download_buffer[] - updated with new data
 *                  downloadbuf_mgmt.write_index - incremented by length of data stored.
 *                  downloadbuf_mgmt.used_bytes - incremented by length of data stored. 
 *                  usbprogPICstatus.DownloadOvrFlow - set if data length > remaining buffer
 *****************************************************************************/
extern void 
WriteDownloadDataBuffer(byte *usbindex)
{
    unsigned int i, numbytes;

    numbytes = inbuf[(*usbindex)++] & 0xFF;   // i= # bytes data (length)

    if ((numbytes + downloadbuf_mgmt.used_bytes)  > DOWNLOAD_SIZE)     // not enough room for data
    {
        usbprogPICstatus.DownloadOvrFlow = 1;
        return;
    }

    for (i = 0; i < numbytes; i++)
    {
        uc_download_buffer[downloadbuf_mgmt.write_index++] = inbuf[(*usbindex)++];
        if (downloadbuf_mgmt.write_index >= DOWNLOAD_SIZE) // handle index wrap
        {
            downloadbuf_mgmt.write_index = 0;
        }
        downloadbuf_mgmt.used_bytes++;  // used another byte.
    }
}

/******************************************************************************
 * Function:        void GetStatus(void)
 * Overview:        Sends READ_STATUS response over USB.
 *
 * Input:           None
 * Output:          Transmits HID Tx report with usbprogPICstatus.
 *****************************************************************************/
extern void 
GetStatus(void)
{
    usbprogPICstatus.StatusLow &= 0xF0;    // clear bits to be tested
    
	//if (Vpp_ON_pin)         // active high
        usbprogPICstatus.VppOn = 1;
    //if (MCLR_TGT_pin)       // active high
        usbprogPICstatus.VppGNDOn = 1;
    //if (!Vdd_TGT_P_pin)     // active low
        usbprogPICstatus.VddOn = 1;
    //if (Vdd_TGT_N_pin)      // active high
        usbprogPICstatus.VddGNDOn = 1;
		
	outbuf[0] = usbprogPICstatus.StatusLow;
	outbuf[1] = usbprogPICstatus.StatusHigh;

    // Now that it's in the USB buffer, clear errors & flags
    usbprogPICstatus.StatusLow &= 0x8F;
    usbprogPICstatus.StatusHigh &= 0x00;	
} 

/******************************************************************************
 * Function:        void SendFWVersionUSB(void)
 * Overview:        Sends firmware version over USB.
 *
 * Input:           None
 * Output:          Transmits HID Tx report with 3-byte version #.
 *****************************************************************************/
extern void 
GetFWVersion(void)
{
	outbuf[0] = MAJORVERSION;
	outbuf[1] = MINORVERSION;
	outbuf[2] = DOTVERSION;
} 


/******************************************************************************
 * Function:        void WriteByteDownloadBuffer (byte DataByte)
 * Overview:        Puts a byte in the download buffer
 *
 * Input:           DataByte - Byte to be put at Write pointer
 * Output:          write pointer and used_bytes updated.
 *****************************************************************************/
extern void 
WriteByteDownloadBuffer(byte DataByte)
{
        uc_download_buffer[downloadbuf_mgmt.write_index++] = DataByte;
        if (downloadbuf_mgmt.write_index >= DOWNLOAD_SIZE) // handle index wrap
        {
            downloadbuf_mgmt.write_index = 0;
        }
        downloadbuf_mgmt.used_bytes++;  // used another byte.
}


/******************************************************************************
 * Function:        void LongDelay(byte count)
 * Overview:        Delays in increments of 5.46ms * count.
 * 
 * Input:           count - units of delay (5.46ms each)
 * Output:          None.
 *****************************************************************************/
extern void 
LongDelay(byte count)
{
    int i;
	
	for(i=0; i<count; i++) {
		_delay_ms(5.46);
		}
}


/******************************************************************************
 * Function:        void ShortDelay(byte count)
 * Overview:        Delays in increments of 42.7us * count.
 * 
 * Input:           count - units of delay (42.7us each)
 * Output:          None.
 *****************************************************************************/
extern void 
ShortDelay(byte count)
{
    int i;
	
	for(i=0; i<count; i++) {
		_delay_us(42.7);
		}     
}



/******************************************************************************
 * Function:        void WriteUploadBuffer(byte byte2write)
 * Overview:        Attempts to write a byte to the upload buffer.
 *                  If full, sets error usbprogPICstatus.UpLoadFull
 *
 * Input:           byte2write - byte to be written
 * Output:          uc_upload_buffer - byte written to end of buffer.
 *
 * Side Effects:    Advances download buffer write pointer, if err usbprogPICstatus.StatusHigh != 0
 *****************************************************************************/
extern void 
WriteUploadBuffer(byte byte2write)
{
    if ((uploadbuf_mgmt.used_bytes + 1) > UPLOAD_SIZE)     // not enough room for data
    {
        usbprogPICstatus.UpLoadFull = 1;
        return;
    }

    uc_upload_buffer[uploadbuf_mgmt.write_index++] = byte2write;
    if (uploadbuf_mgmt.write_index >= UPLOAD_SIZE) // handle index wrap
    {
        uploadbuf_mgmt.write_index = 0;
    }
    uploadbuf_mgmt.used_bytes++;  // used another byte.
}

/******************************************************************************
 * Function:        byte ReadDownloadBuffer(void)
 * Overview:        Attempts to pull a byte from the Download Buffer.
 *                  If empty, sets error usbprogPICstatus.DownloadEmpty
 *
 * Input:           None
 * Output:          Returns byte from top of buffer.
 *
 * Side Effects:    Advances download buffer read pointer, if err usbprogPICstatus.StatusHigh != 0
 *****************************************************************************/
extern byte 
ReadDownloadBuffer(void)
{
    byte readbyte;

    if (downloadbuf_mgmt.used_bytes == 0)
    {
        usbprogPICstatus.DownloadEmpty = 1;
        return 0;
    } 

    readbyte = uc_download_buffer[downloadbuf_mgmt.read_index++];
    downloadbuf_mgmt.used_bytes--;        // just removed a byte.
    if (downloadbuf_mgmt.read_index >= DOWNLOAD_SIZE)   // circular buffer - handle wrap.
        downloadbuf_mgmt.read_index = 0; 

    return  readbyte; 
}

