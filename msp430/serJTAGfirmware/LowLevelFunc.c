//part of mspgcc, modified file from TI:

/*==========================================================================*\
|                                                                            |
| LowLevelFunc.c                                                             |
|                                                                            |
| Low Level Functions regarding user's Hardware                              |
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
| 1.2 06/02 ALB2        Formatting changes, added comments.                  |
| 1.3 08/02 ALB2        Initial code release with Lit# SLAA149.              |
|----------------------------------------------------------------------------|
| Designed 2002 by Texas Instruments Germany                                 |
\*==========================================================================*/

#include "LowLevelFunc.h"
#include "taskhandler.h"
#include "tasklist.h"

volatile word sysclk;

/*----------------------------------------------------------------------------
   This function switches TDO to Input, used for fuse blowing
*/
void TDOisInput(void)
{
  //JTAGOUT &= ~TDICTRL1;           // Release TDI pin on target
    Delay(5);                       // Settle MOS relay
  //JTAGOUT |=  TDICTRL2;           // Switch TDI --> TDO
    Delay(5);                       // Settle MOS relay
}

/*----------------------------------------------------------------------------
   Initialization of the Target Board (switch voltages on, preset JTAG pins)    
*/
void InitTarget(void)
{
    JTAGSEL  = 0x00;                    // Pins all I/Os
    JTAGDIR  = TEST;                    // first enable only test
    JTAGOUT  = TEST|TDI|TMS|TCK|TCLK|RST;
    Delay(2);                           // small delay until other pins become outputs too
    JTAGDIR  = TEST|TDI|TMS|TCK|TCLK|RST;
    VPPSEL  &= ~(VPPONTDI | VPPONTEST); // No special function, I/Os
    VPPOUT  &= ~(VPPONTDI | VPPONTEST); // VPPs are OFF
    VPPDIR  |=  (VPPONTDI | VPPONTEST); // VPP pins are outputs
    Delay(50);                          // Settle MOS relays, target capacitor
}
    
/*----------------------------------------------------------------------------
   Release Target Board (switch voltages off, JTAG pins are HI-Z)    
*/
void ReleaseTarget(void)
{
    VPPoff();                       // VPPs are off (safety)
    Delay(5);                       // Settle MOS relays
    JTAGDIR  =  0x00;               // VCC is off, all I/Os are HI-Z
    Delay(5);                       // Settle MOS relays
}

//----------------------------------------------------------------------------
/*  Shift a value into TDI (MSB first) and simultaneously shift out a value
    from TDO (MSB first).
    Note:      When defining SPI_MODE the embedded SPI is used to speed up by 2.
    Arguments: word Format (number of bits shifted, 8 (F_BYTE) or 16 (F_WORD))
               word Data (data to be shifted into TDI)  
    Result:    word (scanned TDO value)
*/
word Shift(word Format, word Data)
{
    word tclk = StoreTCLK();            // Store TCLK state;
    word TDOword = 0x0000;              // Initialize shifted-in word
    word MSB = 0x0000;
         
    word i;
    (Format == F_WORD) ? (MSB = 0x8000) : (MSB = 0x80);
    for (i = Format; i > 0; i--)
    {
            ((Data & MSB) == 0) ? ClrTDI() : SetTDI();
            Data <<= 1;
            if (i == 1)                 // Last bit requires TMS=1
            SetTMS();
            ClrTCK();
            SetTCK();
            TDOword <<= 1;              // TDO could be any port pin
            if (ScanTDO() != 0) TDOword++;
    }
 
    RestoreTCLK(tclk);                  // restore TCLK state
    PrepTCLK();                         // Set JTAG FSM back into Run-Test/Idle
    return(TDOword);
}

//----------------------------------------------------------------------------
static volatile word delay_to_wakeup;   //counter for the 1ms interrupt

/**
Delay function (resolution is 1 ms)
Arguments: word millisec (number of ms, max number is 0xFFFF)
*/
void Delay(word millisec)
{
    if (millisec) {
        delay_to_wakeup = millisec;     //set up delay, counted in intrrupt
        while (delay_to_wakeup) {       //wait until time has passed
            LPM0;                       //go to low power mode, ...
        }                               //may wake up on other events too, thus the loop
        //~ while (delay_towakeup) {}
    }
}


//this interrupt gets called every 1ms
interrupt(TIMERA0_VECTOR) ta0_isr() {
    static word milliseconds;
    //1 second timer
    if (++milliseconds >= 1000) {
        milliseconds = 0;
        taskreg |= TASK_watch;
        _BIC_SR_IRQ(LPM4_bits);
    }
    //count down for Delay() function
    if (delay_to_wakeup) {
        if (--delay_to_wakeup == 0) {
            _BIC_SR_IRQ(LPM4_bits);
        }
    }
}


/****************************************************************************/
/*                         END OF SOURCE FILE                               */
/****************************************************************************/
