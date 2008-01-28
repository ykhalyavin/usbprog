/*****************************************************************************
* file: eeprom.h
* description: Header-file for reading/writing of internal/external memory
* notes: Probably of no use for Usbprog.
*	
* This file is part of the UsbprogPIC software, written by Nico van Leeuwen
* (c) 2008, www.liondesign.nl
* 
* Usbprog is a development of Benedikt Sauter, www.embedded-projects.net
******************************************************************************/

/** Prototypes **/
/* Internal memory functions */
extern void ReadInternalEEPROM(byte *usbindex);
extern void WriteInternalEEPROM(byte *usbindex);

/* External memory functions (from target) */
extern void EE_WriteByte(byte byte_address, byte write_byte);
extern byte EE_ReadByte(byte byte_address);
