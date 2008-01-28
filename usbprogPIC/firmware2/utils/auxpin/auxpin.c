/*****************************************************************************
* file: auxpin.c
* description: Functions for the AUX pin
*	
* This file is part of the UsbprogPIC software, written by Nico van Leeuwen
* (c) 2008, www.liondesign.nl
* 
* Usbprog is a development of Benedikt Sauter, www.embedded-projects.net
******************************************************************************/
#define F_CPU 16000000UL

#include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "../../usbprogPIC.h"
#include "../icsp/icsp.h"
#include "auxpin.h"


/******************************************************************************
 * Function:        void SetAUX_PinState(byte aux_byte)
 * Overview:        Sets the value and direction of the AUX pin.
 *
 * Input:           aux_byte - byte formated
 *   					<7 – 2> unused
 *   					<1> AUX logic level
 *    					<0> 1= AUX input, 0= output
 * Output:          Affects bits in TRISA and LATA
 *****************************************************************************/
extern void 
SetAUX_PinState(byte aux_byte)
{/*
				// set AUX latch
				if (aux_byte & 0x02)
					AUX = 1;	
				else
					AUX = 0;

				// set AUX direction
				if (aux_byte & 0x01)
					tris_AUX = 1;	
				else
					tris_AUX = 0;
*/
}


/******************************************************************************
 * Function:        byte GetAUX_PinState(void)
 * Overview:        Gets the values of the ICSP pins.
 *
 * Input:           None
 * Output:          returns a byte with bits:
 *    					<1> AUX state
 *****************************************************************************/
extern byte 
GetAUX_PinState(void)
{

    byte states = 0;
	/*
    if (AUX_in == 1)
    {
        states |= 0x01;
    }
	*/
    return states;
}

/******************************************************************************
 * Function:        byte SPI_ReadWrite(byte outputbyte)
 * Overview:        Shifts outputbyte out on the AUX pin with PGC as SCK
 *                  At the same time, bits on PGD are shifted in and
 *                  returned as the read value. 
 *
 * PreCondition:    PGC, AUX = output ; PGD = input
 *
 * Input:           outputbyte - byte to be shifted out MSb first on AUX
 * Output:          returns the byte shifted in MSb first from PGD
 *
 * Note:            Assumes ICSP pins are already set to outputs.
 *****************************************************************************/
extern byte 
SPI_ReadWrite(byte outputbyte)
{
	byte temp = 0;
/*
	//BOOL interrupts_on = 0;
    char i;

	//if (INTCONbits.GIE == 1)
	//	interrupts_on = 1;
	//INTCONbits.GIE = 0;			// uninterruptable routine

	asm_temp1 = outputbyte;         // read byte is shifted in here as well
    asm_temp2 = 8;

    if (icsp_baud < 2)
    {
    	_asm
           WRITE8LOOPF:
    		btfss	ASM_TEMP1_RAM, 7, 0
    		bcf		LATA, 4, 0 
    		btfsc	ASM_TEMP1_RAM, 7, 0
    		bsf		LATA, 4, 0 
    		bsf		LATA, 3, 0
            rlncf   ASM_TEMP1_RAM, 1, 0
            bcf     ASM_TEMP1_RAM, 0, 0
            btfsc   PORTA, 2, 0
            bsf     ASM_TEMP1_RAM, 0, 0
            bcf     LATA, 3, 0
            decfsz  ASM_TEMP2_RAM, 1, 0
            bra     WRITE8LOOPF
    	_endasm
    }
    else
    {
        asm_temp3 = icsp_baud - 1;
    	_asm
           WRITE8LOOPS:
    		btfss	ASM_TEMP1_RAM, 7, 0
    		bcf		LATA, 4, 0 
    		btfsc	ASM_TEMP1_RAM, 7, 0
    		bsf		LATA, 4, 0 
            movf    ASM_TEMP3_RAM, 0, 0    // delay 6 cycles per count
           DELAYLOOPHI:
            nop
            nop
            nop
            decfsz  WREG, 1, 0
            bra     DELAYLOOPHI
    		bsf		LATA, 3, 0
            rlncf   ASM_TEMP1_RAM, 1, 0
            bcf     ASM_TEMP1_RAM, 0, 0
            btfsc   PORTA, 2, 0
            bsf     ASM_TEMP1_RAM, 0, 0
            movf    ASM_TEMP3_RAM, 0, 0    // delay 6 cycles per count
           DELAYLOOPLO:
            nop
            nop
            nop
            decfsz  WREG, 1, 0
            bra     DELAYLOOPLO
            bcf     LATA, 3, 0
            decfsz  ASM_TEMP2_RAM, 1, 0
            bra     WRITE8LOOPS
    	_endasm
    }

	//if (interrupts_on == 1)		// turn interrupts back on if enabled.	
	//	INTCONbits.GIE = 1;
    
    AUX = 0;        // leave low
*/
    return temp;
}
