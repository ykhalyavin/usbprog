//part of mspgcc, modified file from TI:

/*==========================================================================*\
|                                                                            |
| LowLevelFunc.h                                                             |
|                                                                            |
| Low Level function prototypes, macros, and pin-to-signal assignments       |
| regarding to user's hardware                                               |
|----------------------------------------------------------------------------|
| Project:              MSP430 Replicator                                    |
| Developed using:      IAR Embedded Workbench 2.31C                         |
|----------------------------------------------------------------------------|
| Author:               FRGR                                                 |
| Version:              1.3                                                  |
| Initial Version:      04-17-02                                             |
| Last Change:          08-29-02                                             |
|----------------------------------------------------------------------------|
| Version history:                                                           |
| 1.0 04/02 FRGR        Initial version.                                     |
| 1.1 04/02 FRGR        Included SPI mode to speed up shifting function by 2.|
|                       (JTAG control now on Port5)                          |
| 1.2 06/02 ALB2        Formatting changes, added comments. Removed code used|
|                       for debug purposes during development.               |
| 1.3 08/02 ALB2        Initial code release with Lit# SLAA149.              |
| 1.3 03/03 ENOK        Changed port for VPP from port 3 to port 2 to        |
|                       match serial JTAG hardware as proposed by Chris      |
|                       Also removed macros VPPONxxx                         |
|----------------------------------------------------------------------------|
| Designed 2002 by Texas Instruments Germany                                 |
\*==========================================================================*/

#ifndef LOWLEVELFUNC_H
#define LOWLEVELFUNC_H

#include "defs.h"
#include "hardware.h"
#include "JTAGfunc.h"

#ifndef __BYTEWORD__
#define __BYTEWORD__
typedef unsigned int    word;
typedef unsigned char   byte;
#endif

// Constants for runoff status
#define STATUS_ERROR    0        // false
#define STATUS_OK       1        // true
#define STATUS_ACTIVE   2            
#define STATUS_IDLE     3

//----------------------------------------------------------------------------
// Pin-to-Signal Assignments
//----------------------------------------------------------------------------

/*----------------------------------------------------------------------------
   Macros for processing the JTAG port and Vpp pins
*/
#define ClrTMS()        ((JTAGOUT) &= (~TMS))
#define SetTMS()        ((JTAGOUT) |= (TMS))
#define ClrTDI()        ((JTAGOUT) &= (~TDI))
#define SetTDI()        ((JTAGOUT) |= (TDI))
#define ClrTCK()        ((JTAGOUT) &= (~TCK))
#define SetTCK()        ((JTAGOUT) |= (TCK))
#define ClrTCLK()       ((JTAGOUT) &= (~TCLK))
#define SetTCLK()       ((JTAGOUT) |= (TCLK))
#define StoreTCLK()     ((JTAGOUT  &   TCLK))
#define RestoreTCLK(x)  (x == 0 ? ClrTCLK() : SetTCLK())
#define ScanTDO()       ((JTAGIN   &   TDO) ? 1 : 0)
#define VPPon(x)        (x == VPP_ON_TEST ? (VPPOUT |= VPPONTEST) : (VPPOUT |= VPPONTDI))
#define VPPoff()        ((VPPOUT)  &= (~(VPPONTDI | VPPONTEST)))

/*----------------------------------------------------------------------------
   Low Level function prototypes
*/
void Delay(word Millisec);
void InitTarget(void);
void ReleaseTarget(void);
word Shift(word Format, word Data);    // used for IR- as well as DR-shift
void TDOisInput(void);
void TCLKstrobes(word Amount);

extern volatile word sysclk;

#endif
