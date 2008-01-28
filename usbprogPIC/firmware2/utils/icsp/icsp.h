/*****************************************************************************
* file: icsp.h
* description: ICSP programming functions headerfile
*	
* This file is part of the UsbprogPIC software, written by Nico van Leeuwen
* (c) 2008, www.liondesign.nl
* 
* Usbprog is a development of Benedikt Sauter, www.embedded-projects.net
******************************************************************************/

#define ICSP_DAT_PIN	PB5
#define ICSP_CLK_PIN	PB7

extern byte ShiftBitsInPIC24(byte numbits);
extern byte ShiftBitsInICSP(byte numbits);
extern void ShiftBitsOutICSP(byte outputbyte, char numbits);
extern void ShiftBitsOutICSPHold(byte outputbyte, char numbits);
extern void SetICSP_PinStates(byte icsp_byte);
extern byte GetICSP_PinStates(void);

