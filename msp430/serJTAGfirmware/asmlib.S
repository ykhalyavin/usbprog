/**
Assembler functions.

part of serJTAGfirmware
http://mspgcc.sf.net
chris <cliechti@gmx.net>
*/

#include "hardware.h"

/**
demux timer A interrupts
*/
interrupt(TIMERA1_VECTOR)
tax_int:
        add     &TAIV, r0               ; Add TA interrupt offset to PC
        reti                            ; CCR0 - no source
        jmp     ccr1                    ; CCR1
        jmp     ccr2                    ; CCR2
        reti                            ; CCR3 - no source
        reti                            ; CCR4 - no source
taover:                                 ; TAOVER (follows directly)
        //~ inc     sysclk
        //~ bic     #LPM4, 0(r1)            ; wakeup
.taex:  reti
ccr1:   reti    ;br      #INT_TimerA_CCR1 ;redirect to an interrupt function
ccr2:   reti    ;br      #INT_TimerA_CCR2 ;redirect to an interrupt function


;;----------------------------------------------------------------------------
;;  This function generates "Amount" strobes with the Flash Timing Generator
;;  Frequency fFTG = 257..476kHz (t = 3.9..2.1us).
;;  User knows target frequency, instruction cycles, C implementation.
;;  Arguments: word Amount (number of strobes to be generated)
;;
;;  We use t=2.9us
;;
#define S_DELAY_1    ((MCLK_FREQUENCY/(10000000/29) - 6)/3 + 1)

.global TCLKstrobes
        .type TCLKstrobes,@function
TCLKstrobes:
        tst     r15
        jz      .STROBE_EXIT
        dint
.STROBE_LOOP:
        bis.b   #TCLK,&JTAGOUT
        mov     #S_DELAY_1, r14
.STROBE_DELAY:
        dec     r14
        jne     .STROBE_DELAY
        bic.b   #TCLK,&JTAGOUT
        dec     r15
        jne     .STROBE_LOOP
.STROBE_EXIT:
        eint
        ret


