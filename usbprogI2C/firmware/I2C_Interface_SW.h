/************************************************************************/
/*																		*/
/*	I2C_routines.h	--	source file for the i2c packet level			*/
/*	protocol routines.  Uses the i2c interface to send and recieve		*/
/*	information. (this is a bit banged interface, does not require		*/
/*	the specific use of the i2c hardware								*/
/*																		*/
/************************************************************************/
/*	Author: Chris Keeser												*/
/************************************************************************/
/*  Module Description: 	These routines will form the basic			*/
/*	means of communicating between devices running the packet			*/
/*	level protocol.  higher level functionality will be left up to		*/
/*	the user to define.  these routines will simply make it much easier	*/
/*	to send and recieve information to and from devices running the		*/
/*	packet level protocol.  Transmission and reception of bytes will	*/
/*	be preformed in these routines and basic error checking.			*/
/*	*NOTE only operates as an I2C master								*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*	Created: 6/11/05 (ChrisK)											*/
/*	Revised: 6/21/05 (ChrisK) added ability to operate on different 	*/
/*		ports and pins in i2c routines									*/
/*	Revised: 7/16/05 (ChrisK) added Clock stretching option through		*/
/*		conditional compile flag										*/
/*	Revised: 7/26/05 (ChrisK) added delay option through				*/
/*		conditional compile flag										*/
/*	Revised: 8/10/05 (ChrisK) added clock stretching timeout			*/
/*																		*/
/************************************************************************/

// guard code
#ifndef I2C_INTERFACE_SW
#define I2C_INTERFACE_SW

/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */

// basic register accsess
#include <avr/io.h>

// BOOL (fTrue, fFalse), BYTE, WORD, DWORD all defined here
#include "GenDefs.h"


/* ------------------------------------------------------------ */
/*				Project Definitions								*/
/* ------------------------------------------------------------ */

// uncomment to enable clock stretching
#define CLOCK_STRETCHING

// uncomment to enable the clock stretching timeout
#define CLOCK_STETCH_TIME_OUT

// defines the length of time (APPROX in clock cycles)
// the i2c routines will allow the clock to be stretched
// prevents lockups and facilitates error recovery
#define CLOCK_STRETCH_MAX	100000

// used in the clock stretching timeout
#ifdef CLOCK_STETCH_TIME_OUT
DWORD clockCount;
#endif

// uncomment to enable delays
//#define ADD_I2C_DELAY

#ifdef ADD_I2C_DELAY
#include <avr/delay.h>
// modify delay length here
//#define TDELAY 0xF

// for runtime changing of the delay
// uncomment this line, and uncomment
// the initialization value in the
// I2CInit Function
WORD TDELAY;
#endif

// define the Clock pin information here
#define i2cPORTC PORTF
#define i2cDDRC DDRF
#define i2cPINC PINF
#define i2cSCK 0

// define the Data pin information here
#define i2cPORTD PORTF
#define i2cDDRD DDRF
#define i2cPIND PINF
#define i2cSDA 1

/* ------------------------------------------------------------ */
/*				Forward Declarations							*/
/* ------------------------------------------------------------ */

// initializes the I2C hardware
void I2cInit(void);

// generates the Start condition
void Start(void);

// transmits a byte
void SendByteI2C(BYTE);

// recives the acknowledge
BOOL FReceiveAck(void);

// generates the Stop condition
void Stop(void);

// generates the ReStart condition
void ReStart(void);

// recieves a byte
BYTE BReceiveByteI2C(void);

// sends the acknowledge
void SendAck(void);

// sends the No Acknowledge
void SendNack(void);

// sends the acknowledge
void PrepareAck(void);

// sends the No Acknowledge
void PrepareNack(void);

#endif
