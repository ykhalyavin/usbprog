/*****************************************************************************
* file: eeprom.c
* description: Original Pickit2 functions for reading/writing of internal/external memory
* notes: Probably of no use for Usbprog.
*	
* This file is part of the UsbprogPIC software, written by Nico van Leeuwen
* (c) 2008, www.liondesign.nl
* 
* Usbprog is a development of Benedikt Sauter, www.embedded-projects.net
******************************************************************************/

#include "../../usbprogPIC.h"
#include "eeprom.h"

/******************************************************************************
 * Function:        void ReadInternalEEPROM(byte *usbindex)
 * Overview:        Writes a given # of bytes into the internal MCU EEPROM.
 *
 * Input:           *usbindex - index to start address in USB buffer
 * Output:          Transmits HID Tx report with data.
 *
 * Note:            If the length byte is > 32, only the first 32 bytes are 
 *                  Read.
 *****************************************************************************/
extern void 
ReadInternalEEPROM(byte *usbindex)
{
} 

/******************************************************************************
 * Function:        void WriteInternalEEPROM(byte *usbindex)
 * Overview:        Writes a given # of bytes into the internal MCU EEPROM.
 *
 * Input:           *usbindex - index to start address of data in USB buffer
 * Output:          Internal EEPROM - updated with new data
 *
 * Note:            If the length byte is > 32, only the first 32 bytes are 
 *                  written.
 *****************************************************************************/
extern void 
WriteInternalEEPROM(byte *usbindex)
{
}

/******************************************************************************
 * Function:        void EE_WriteByte(byte byte_address, byte write_byte)
 * Overview:        Writes value write_byte to the internal EEPROM address byte_address. 
 *
 * Input:           byte_address - EEPROM address to be written to
 *                  write_byte - byte value to write to EEPROM
 * Output:          Specified EE byte address is written with given value.
 *
 * Side Effects:    Interrupts are disabled during EE write sequence.
*******************************************************************************/
extern void 
EE_WriteByte(byte byte_address, byte write_byte)
{
}

/******************************************************************************
 * Function:        byte EE_ReadByte(byte byte_address)
 * Overview:        Reads a byte from EEPROM at the given address. 
 *
 * Input:           byte_address - EEPROM address to be read from
 * Output:          Returns value of EE byte at byte_address   
 *****************************************************************************/
extern byte 
EE_ReadByte(byte byte_address)
{
    /*
	EEADR = byte_address;
    EECON1 = 0;
    EECON1bits.RD = 1;
	*/
    return 0;// previously EEDATA;
      
}
