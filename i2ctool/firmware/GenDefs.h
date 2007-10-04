/************************************************************************/
/*																		*/
/*	GenDefs.h	--	General definitions for AVR projects.  Used for		*/
/*	defining BYTE, WORD, and DWORD, along with definitions for			*/
/*	BOOL, fFalse, and fTrue												*/
/*																		*/
/************************************************************************/
/*	Author: 		Chris Keeser										*/
/*												*/
/************************************************************************/
/*  Module Description: This header file contains often used types		*/
/*	when programming embedded systems.  The inclusion of this header	*/
/*	file allows the use of BYTE, WORD, DWORD and BOOL with definitions	*/
/*	for fTrue and fFalse												*/
/*																		*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*		created: 6/1/05 (ChrisK)										*/
/*																		*/
/************************************************************************/

#ifndef CK_GEN_DEFS
#define CK_GEN_DEFS

#include <inttypes.h>

/* ------------------------------------------------------------ */
/*					Type Definitions							*/
/* ------------------------------------------------------------ */

// 8 bit value, unsigned int
typedef	uint8_t 	BYTE;

// 16 bit value, unsigned int
typedef	uint16_t 	WORD;

// 32 bit value, unsigned int
typedef	uint32_t	DWORD;

// definition of a boolean value
typedef	uint8_t		BOOL;

/* ------------------------------------------------------------ */
/*					BOOL Definitions							*/
/* ------------------------------------------------------------ */

// definitions of fTrue and fFalse
#define	fTrue		255
#define	fFalse		0

#endif
