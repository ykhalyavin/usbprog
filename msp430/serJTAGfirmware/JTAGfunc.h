//part of mspgcc, modified file from TI:

/*==========================================================================*\
|                                                                            |
| JTAGfunc.h                                                                 |
|                                                                            |
| JTAG Function Prototypes and Definitions                                   |
|----------------------------------------------------------------------------|
| Project:              JTAG Functions                                       |
| Developed using:      IAR Embedded Workbench 2.31C                         |
|----------------------------------------------------------------------------|
| Author:               FRGR                                                 |
| Version:              1.2                                                  |
| Initial Version:      04-17-02                                             |
| Last Change:          08-29-02                                             |
|----------------------------------------------------------------------------|
| Version history:                                                           |
| 1.0 04/02 FRGR        Initial version.                                     |
| 1.1 06/02 ALB2        Formatting changes, added comments.                  |
| 1.2 08/02 ALB2        Initial code release with Lit# SLAA149.              |
|----------------------------------------------------------------------------|
| Designed 2002 by Texas Instruments Germany                                 |
\*==========================================================================*/

#include "defs.h"
#ifndef __BYTEWORD__
#define __BYTEWORD__
typedef unsigned int  word;
typedef unsigned char byte;
#endif

/****************************************************************************/
/* Define section for constants                                             */
/****************************************************************************/

// Constants for the JTAG instruction register (IR, requires LSB first). 
// The MSB has been interchanged with LSB due to use of the same shifting 
// function as used for the JTAG data register (DR, requires MSB first).

// Instructions for the JTAG control signal register
#define IR_CNTRL_SIG_16BIT	0xC8	// 0x13 original values
#define IR_CNTRL_SIG_CAPTURE	0x28	// 0x14
#define IR_CNTRL_SIG_RELEASE	0xA8	// 0x15
// Instructions for the JTAG Fuse
#define IR_PREPARE_BLOW		0x44	// 0x22
#define IR_EX_BLOW		0x24	// 0x24 
// Instructions for the JTAG data register
#define IR_DATA_16BIT		0x82	// 0x41
#define IR_DATA_QUICK		0xC2	// 0x43
// Instructions for the JTAG PSA mode
#define IR_DATA_PSA		0x22	// 0x44
#define IR_SHIFT_OUT_PSA	0x62	// 0x46    
// Instructions for the JTAG address register
#define IR_ADDR_16BIT		0xC1	// 0x83
#define IR_ADDR_CAPTURE		0x21	// 0x84
#define IR_DATA_TO_ADDR		0xA1	// 0x85
// Bypass instruction
#define IR_BYPASS		0xFF	// 0xFF

// JTAG identification value for all existing Flash-based MSP430 devices
#define JTAG_ID			0x89

// Constants for data formats, dedicated addresses
#define	F_BYTE			8
#define F_WORD			16
#define V_RESET			0xFFFE

// Constants for flash erasing modes
#define ERASE_MASS		0xA506
#define ERASE_MAIN		0xA504
#define ERASE_SGMT		0xA502

// Constants for VPP connection at Blow-Fuse
#define VPP_ON_TDI		0
#define VPP_ON_TEST		1

/****************************************************************************/
/* Function prototypes                                                      */
/****************************************************************************/

// Low level JTAG functions
void PrepTCLK(void);
word DR_Shift16(word Data);
word IR_Shift(byte Instruction);
void ResetTAP(void);
word ExecutePUC(void);
word SetInstrFetch(void);
void SetPC(word Addr);
void HaltCPU(void);
void ReleaseCPU(void);
word VerifyPSA(word StartAddr, word Length, word *DataArray);

// High level JTAG functions
word GetDevice(void);
void ReleaseDevice(word Addr);
void WriteMem(word Format, word Addr, word Data);
void WriteMemQuick(word StartAddr, word Length, word *DataArray);
void WriteFLASH(word StartAddr, word Length, word *DataArray);
word WriteFLASHallSections(word *DataArray);
word ReadMem(word Format, word Addr);
void ReadMemQuick(word StartAddr, word Length, word *DataArray);
void EraseFLASH(word EraseMode, word EraseAddr);
word EraseCheck(word StartAddr, word Length);
word VerifyMem(word StartAddr, word Length, word *DataArray);
word BlowFuse(void);
word IsFuseBlown(void);
