/**
see README.txt for details.

Main application of the Serial-JTAG adapter

part of serJTAGfirmware
http://mspgcc.sf.net
chris <cliechti@gmx.net>
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "hardware.h"
#include "JTAGfunc.h"
#include "LowLevelFunc.h"
#include "serialComm.h"
#include "taskhandler.h"
#include "tasklist.h"

//-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
/**
task which handles a keypress.
*/
void keyTask(void) {
    word *ptr = (void *)0x1000;         //here is the pointer to the data area
    word *userdata;                     //pointer to the data area, initliaized below
    word ok = 0;

    Delay(10);                          //wait 10ms ...
    if ((P2IN & STARTKEY) == 0) {       //only if key still pressed, neg logic (debounce)
        AutoPowerOffDisable();          //disable auto power off timer
        P2OUT |= LEDRT;                 //both LEDs on during operation

        if ((*ptr != 0xffff) && (*ptr != 0)){   //valid pointer to user program / data area?
            userdata = (void *)*ptr;    //take the address which is stored at 0x1000
            //blink with green LED as confirmation that a user program was found
            P6OUT |= LEDGN;
            Delay(20);
            P6OUT &= ~LEDGN;
            txDebug("user prog ok");
            //now lets programm the target
            InitTarget();               //Connect JTAG
            Delay(250);                 //load caps on target
            if (GetDevice() == STATUS_OK) {     //sync JTAG
                txDebug("found device");
                if (userdata[0]) EraseFLASH(userdata[0], 0xfffe);  //erase with the method selected by the user prog
                P6OUT |= LEDGN;         //again feedback for the user - erase sucessful
                //write flash, data from table...
                if (WriteFLASHallSections(&userdata[1]) == STATUS_OK) {
                    //~ if (...) {      //no fuse blowing hardware...
                        //~ BlowFuse();
                    //~ }
                    ok = 1;
                    txDebug("success");
                } else {
                    txDebug("error: could not write flash");
                }
            } else {
                txDebug("error: no device found");
            }
            ReleaseDevice(V_RESET);     //reset target and ...
            ReleaseTarget();            //release JTAG
            //optical success
            Delay(500);                 //a delay, so that the first two flashes are better visible
            if (ok) {
                P6OUT |= LEDGN;         //keep LED on, will be switched off in the auto power off task
                P2OUT &= ~LEDRT;        //switch LED off
            } else {
                P6OUT &= ~LEDGN;        //switch LED off
                P2OUT |= LEDRT;         //keep LED on, will be switched off in the auto power off task
            }
        } else {
            txDebug("keypress, but got no user program");
        }
        AutoPowerOffReset();            //reset auto power off timer
    }
    P2IFG = 0;                          //make sure no interrupts are pending (bouncing key)
    P2IE |= STARTKEY;                   //renable key interrupts
}

wakeup interrupt(PORT2_VECTOR) port2_isr() {
    taskreg |= TASK_keyHandler;
    P2IE &= ~STARTKEY;
    P2IFG = 0;
}

//-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

/**
dummy task for unused tasktable entries.
*/
void idleTask(void) {
}

//-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
/**
switch off everything to go to a low power mode
wakeup trough key press or receive on serial port.
*/
void powerDown() {
    P1DIR = 0;                  //VCC is off, all I/Os are HI-Z
    P2OUT = 0;                  //switch off VCC26 -> jumper power down
    P3OUT = 0;
    P3SEL = 0;                  //dettach TX|RX -> make sure MAX232 chip is not powered over data lines
#ifdef __msp430_have_port4
    P4OUT = 0;                  //switch off VCC40 -> jumper power down
#endif
#ifdef __msp430_have_port5
    P5OUT = 0;
#endif
#ifdef __msp430_have_port6
    P6OUT = 0;
#endif
    
    //enable interrupts on P2.2 -> serial line and key may wake up device
    P2IFG = 0;
    P1IES = RXBSL|STARTKEY;     //neg edge triggers
    P2IE = RXBSL|STARTKEY;
    //power down peripherals
    U0CTL = 0;                  //disable UART
    TACTL = 0;                  //disbale timer
    //now enter low power mode
    //~ P6OUT |= LEDGN;     //DEBUG!!!
    LPM4;
    //after wakeup, generate a watchdog reset that way we have a clean startup
    WDTCTL = 0;
}

/**
task called on auto power off delay.
check if there is a serial device active else swtich off the device.
*/
void powerDownTask(void) {
    P6OUT &= ~LEDGN;                    //switch LED off, might be on after sucessful JTAG programming
    P2OUT &= ~LEDRT;                    //switch LED off, might be on after failed JTAG programming
    //check if serial port is connected
    if (((P3IN & CTS) == 0) &&          //CTS is active?
        (P2IN & RXBSL))                 //and RX IDLE?
    {   
        AutoPowerOffReset();            //reset auto power off timer
    } else {
        powerDown();
    }
    //~ powerDown();
}

static volatile word delay_to_powerdown;//counter for the 1ms interrupt

/**
disable auto power off.
*/
void AutoPowerOffDisable(void) {
    delay_to_powerdown = 0;
}

/**
reset auto power off timer, starting a new count down.
*/
void AutoPowerOffReset(void) {
    delay_to_powerdown = POWER_DOWN_TIME;
}

/**
periodic task
*/
void watchTask(void) {
    if (delay_to_powerdown) {
        if (--delay_to_powerdown == 0) {
            taskreg |= TASK_powerDown;
        }
    }
    if (!(P2OUT & LEDRT)) {             //only if not yet lighting
        P2OUT |= LEDRT;                 //on
        Delay(7);                       //want to see the activity LED
        P2OUT &= ~LEDRT;                //off
    }
}

//-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

/**
Main function with init and an endless loop that is synced with the
interrupts trough the lowpower mode (taskhandler).
*/
int main(void) {
    word i;
    
    //flash key violation flag is abused as flag to start the BSL
    if (FCTL3 & KEYV) {                 //flash key violation -> enter BSL
        FCTL3 = FWKEY;                  //reset flag
        //~ P6OUT = P6DIR = LEDGN;          //green LED is switched on
        __asm__ __volatile__("br &0x0c00");  //enter BSL
    }
    
    WDTCTL = WDTCTL_INIT;               //Init watchdog timer

    P1OUT  = P1OUT_INIT;                //Init output data of port1
    P1SEL  = P1SEL_INIT;                //Select port or module -function on port1
    P1DIR  = P1DIR_INIT;                //Init port direction register of port1
    P1IES  = P1IES_INIT;                //init port interrupts
    P1IFG  = 0;                         //we dont want to process interrupts on startup
    P1IE   = P1IE_INIT;

    P2OUT  = P2OUT_INIT;                //Init output data of port2
    P2SEL  = P2SEL_INIT;                //Select port or module -function on port2
    P2DIR  = P2DIR_INIT;                //Init port direction register of port2
    P2IES  = P2IES_INIT;                //init port interrupts
    P2IFG  = 0;                         //we dont want to process interrupts on startup
    P2IE   = P2IE_INIT;
    CAPD   = CAPD_INIT;                 //input drivers disable

    P3OUT  = P3OUT_INIT;                //Init output data of port3
    P3SEL  = P3SEL_INIT;                //Select port or module -function on port3
    P3DIR  = P3DIR_INIT;                //Init port direction register of port3

#ifdef __msp430_have_port4
    P4OUT  = P4OUT_INIT;                //Init output data of port4
    P4SEL  = P4SEL_INIT;                //Init port or module -function on port4
    P4DIR  = P4DIR_INIT;                //Init port direction register of port4
#endif
#ifdef __msp430_have_port5
    P5OUT  = P5OUT_INIT;                //Init output data of port5
    P5SEL  = P5SEL_INIT;                //Init port or module -function on port5
    P5DIR  = P5DIR_INIT;                //Init port direction register of port5
#endif
#ifdef __msp430_have_port6
    P6OUT  = P6OUT_INIT;                //Init output data of port6
    P6DIR  = P6DIR_INIT;                //Init port direction register of port6
    P6SEL  = P6SEL_INIT;                //Init port or module -function on port6
#endif

    IE1    = 0;
    IE2    = 0;
    ME1    = 0;
    ME2    = 0;

    P2OUT  |= LEDRT;                    //light LED during init
    DCOCTL  = DCOCTL_INIT;
    BCSCTL1 = BCSCTL1_INIT;
    BCSCTL2 = BCSCTL2_INIT;
    
    do {
        IFG1 &= ~OFIFG;                         //Clear OSCFault flag 
        for (i = 0xff; i > 0; i--) nop();       //Time for flag to set 
    }  while ((IFG1 & OFIFG) != 0);             //OSCFault flag still set? 
    IFG1 &= ~OFIFG;                             //Clear OSCFault flag again 
    
    //Init of USART0 Module
    U0ME  |= UTXE0|URXE0;               //Enable USART0 transmiter and receiver (UART mode)

    U0CTL  = SWRST;                     //reset
    U0CTL  = U0CTL_INIT;                //init & release reset
    
    U0TCTL = U0TCTL_INIT;
    U0RCTL = U0RCTL_INIT;
    
    //~ // TODO: check jumpers for baudrate
    U0BR1  = U0BR1_INIT;
    U0BR0  = U0BR0_INIT;
    U0MCTL = U0MCTL_INIT;
    U0IE  |= URXIE0;                    //Enable USART0 receive interrupts (UART mode)

    //init of timer
    TACCR0 = TA_FREQUENCY/1000;         //Generate interrupt every 1ms
    TACTL  = TACTL_INIT;                //setup timer (still stopped)
    CCTL0  = CCIE;                      //timer A0 interrupt is used for timing purposes
    CCTL1  = 0;                         //
    CCTL2  = 0;                         //
    TACTL |= MC_UPTO_CCR0;              //start timer

    AutoPowerOffReset();                //initialize auto power off

    eint();                             //enable interrupts
    P2OUT &= ~LEDRT;                    //switch LED off
    if ((P2IN & STARTKEY) == 0) {       //check if start key is pressed
        i = TASK_keyHandler;            //start task if so
    } else {
        i = 0;                          //no tasks otherwise
    }
    taskhandler(i);                     //enter the taskhandler
}

