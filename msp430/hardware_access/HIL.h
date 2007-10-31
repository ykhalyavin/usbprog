/*
    Hardware Interface Layer (HIL).

    Based on TI's HIL.dll/HIL.h
    This library capsulates the hardware access APIs of MSP430.dll/so
    
    $Id: HIL.h,v 1.7 2006/09/03 15:33:59 cliechti Exp $
*/

#ifndef HIL_H
#define HIL_H

// #includes. -----------------------------------------------------------------

#include "Basic_Types.h"

// #defines. ------------------------------------------------------------------

enum
{
    JTAG = 0,
    SPYBIWIRE,
    SPYBIWIREJTAG
};

enum
{
    POS_EDGE = 2,
    NEG_EDGE,
};

#ifdef __cplusplus
extern "C" {
#endif

// Functions. -----------------------------------------------------------------
//Note: WINAPI is removed by a #define if compiled on non Windows (see Basic_Types.h)

WINAPI STATUS_T HIL_Initialize(CHAR const * port);
WINAPI STATUS_T HIL_Open(void);
WINAPI STATUS_T HIL_Connect(void);
WINAPI STATUS_T HIL_Release(void);
WINAPI STATUS_T HIL_Close(LONG vccOff);
WINAPI LONG HIL_JTAG_IR(LONG instruction);
WINAPI LONG HIL_TEST_VPP(LONG mode);
WINAPI LONG HIL_JTAG_DR(LONG data, LONG bits);
WINAPI STATUS_T HIL_VCC(LONG voltage);
WINAPI void HIL_TST(LONG state);
WINAPI void HIL_TCK(LONG state);
WINAPI void HIL_TMS(LONG state);
WINAPI void HIL_TDI(LONG state);
WINAPI void HIL_TDO(LONG state);
WINAPI void HIL_TCLK(LONG state);
WINAPI void HIL_RST(LONG state);
WINAPI STATUS_T HIL_VPP(LONG voltage);
WINAPI void HIL_DelayMSec(LONG mSeconds);
WINAPI void HIL_StartTimer(void);
WINAPI ULONG HIL_ReadTimer(void);
WINAPI void HIL_StopTimer(void);
WINAPI LONG HIL_ReadTDO(void);
WINAPI void HIL_SetSlowdown(LONG microseconds);

WINAPI void HIL_CheckJtagFuse(void);
WINAPI void HIL_ResetJtagTap(void);
WINAPI STATUS_T HIL_Trace(BOOL OnOff, char *str);
WINAPI STATUS_T HIL_Trace(BOOL OnOff, char *str);
WINAPI STATUS_T HIL_SetProtocol(int protocol_id);
WINAPI void HIL_sbw_StepPSA(LONG Length);
WINAPI void HIL_sbw_ExecuteFuseBlow(void);

#define SetTMS()        HIL_TMS(1)
#define ClrTMS()        HIL_TMS(0)
#define SetTCK()        HIL_TCK(1)
#define ClrTCK()        HIL_TCK(0)
#define SetTDI()        HIL_TDI(1)
#define ClrTDI()        HIL_TDI(0)
#define SetTDO()        HIL_TDO(1)
#define ClrTDO()        HIL_TDO(0)
#define SetTCLK()       HIL_TCLK(1)
#define ClrTCLK()       HIL_TCLK(0)

#ifdef __cplusplus
}
#endif

#endif // HIL_H
