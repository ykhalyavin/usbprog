/*****************************************************************************
* file: auxpin.h
* description: functions headerfile for the AUX pin
*	
* This file is part of the UsbprogPIC software, written by Nico van Leeuwen
* (c) 2008, www.liondesign.nl
* 
* Usbprog is a development of Benedikt Sauter, www.embedded-projects.net
******************************************************************************/

extern void SetAUX_PinState(byte aux_byte);
extern byte GetAUX_PinState(void);
extern byte SPI_ReadWrite(byte outputbyte);
