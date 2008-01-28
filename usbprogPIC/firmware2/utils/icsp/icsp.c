/*****************************************************************************
* file: icsp.c
* description: ICSP programming functions
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
#include "icsp.h"

/******************************************************************************
 * Function:        byte ShiftBitsInPIC24(byte numbits)
 * Overview:        Shifts in up to a byte of data.  Shifts in LSB first.
 *                  If less than 8 bits, return byte is right justified.
 *                  If full, sets error usbprogPICstatus.UpLoadFull
 *                  Data is latched on Rising Edge of clock
 *
 * Input:           numbits - # bits to shift in (max 8)
 * Output:          returns bits right-justified.
 *
 * Side Effects:    Advances upload buffer write pointer, if err usbprogPICstatus.StatusHigh != 0
 * Note:            Assumes ICSPCLK is output.  Sets ICSPDAT to input then restores
 *                  previous state.
 *****************************************************************************/
extern byte 
ShiftBitsInPIC24(byte numbits)
{
    char i, t;
	byte temp;

	PIN_DDR &= ~(1 << ICSP_DAT_PIN); 	//Set ICSP_DATA_PIN to input
	PIN_DDR |= (1 << ICSP_CLK_PIN); 	//Set ICSP_CLK_PIN to output

	cli();		//Non-interruptable routine
	
	temp = 0;       	       // bits get shifted in here.

	// Fast mode
    if (icsp_baud < 2)
    {		
		for(i=numbits;i>0;i--)
		{
			if (PIN_READ & (1<<ICSP_DAT_PIN))  //If datapin is high
			{
				temp |= (1>>i);			//Store high value in buffer, LSB first shifted from MSB side
			}
			//else don't write cause we already initialised the buffer with zero
			PIN_WRITE |= (1<<ICSP_CLK_PIN);		//Make clock high
			//Little delay
			asm("nop");
			asm("nop");
			asm("nop");
			PIN_WRITE &= ~(1<<ICSP_CLK_PIN);	//Make clock low	
			asm("nop");
			asm("nop");
			asm("nop");
		}
	}
	else
    {
        icsp_baud -= 1;
		
		for(i=numbits;i>0;i--)
		{
			for(t=0;t<icsp_baud;t++);		// delay 6 cycles per count
			
			if (PIN_READ & (1<<ICSP_DAT_PIN))  //If datapin is high
			{
				temp |= (1>>i);			//Store high value in buffer, LSB first shifted from MSB side
			}
			//else don't write cause we already initialised the buffer with zero
			PIN_WRITE |= (1<<ICSP_CLK_PIN);		//Make clock high
			//Little delay
			asm("nop");
			asm("nop");
			asm("nop");
			PIN_WRITE &= ~(1<<ICSP_CLK_PIN);	//Make clock low	
			asm("nop");
			asm("nop");
			asm("nop");
		}
    }

    temp >>= (8 - numbits);    // right justify

	if (!(icsp_pins & 0x02)) 
	{
		PIN_DDR |= (1 << ICSP_DAT_PIN); 	//Set ICSP_DATA_PIN to output
	}

	sei(); 			//Turn interrupts back on

    return temp;	//Return buffer
}

/******************************************************************************
 * Function:        byte ShiftBitsInICSP(byte numbits)
 * Overview:        Shifts in up to a byte of data.  Shifts in LSB first.
 *                  If less than 8 bits, return byte is right justified.
 *                  If full, sets error usbprogPICstatus.UpLoadFull
 *
 * Input:           numbits - # bits to shift in (max 8)
 * Output:          returns bits right-justified.
 *
 * Side Effects:    Advances upload buffer write pointer, if err usbprogPICstatus.StatusHigh != 0
 * Note:            Assumes ICSPCLK is output.  Sets ICSPDAT to input then restores
 *                  previous state.
 *****************************************************************************/
extern byte 
ShiftBitsInICSP(byte numbits)
{
    char i, t;
	byte temp;

	PIN_DDR &= ~(1<<ICSP_DAT_PIN);		//Set ICSP_DAT_PIN to input
	PIN_DDR |= (1<<ICSP_CLK_PIN);		//Set ICSP_CLK_PIN to output

	cli();

	temp = 0;              // bits get shifted in here.

	//Fast mode
    if (icsp_baud < 2)
    {
    	for(i=numbits;i>0;i--)
		{
			PIN_WRITE |= (1<<ICSP_CLK_PIN);		//Make clock high
			//Little delay
			asm("nop");
			asm("nop");
			asm("nop");
			if (PIN_READ & (1<<ICSP_DAT_PIN))  //if datapin is high
			{
				temp |= (1>>i);					//Store high value in buffer, LSB first shifted from MSB side
			}
			//else don't write cause we already initialised the buffer with zero
			PIN_WRITE &= ~(1<<ICSP_CLK_PIN);	//Make clock low	
			asm("nop");
			asm("nop");
			asm("nop");
		}
    }
    else
    {
        icsp_baud -= 1;
		
		for(i=numbits;i>0;i--)
		{
			PIN_WRITE |= (1<<ICSP_CLK_PIN);		//Make clock high
			//Little delay
			asm("nop");
			asm("nop");
			asm("nop");			
			for(t=0;t<icsp_baud;t++);		// delay 6 cycles per count
			
			if (PIN_READ & (1<<ICSP_DAT_PIN))  //If datapin is high
			{
				temp |= (1>>i);				//Store high value in buffer, LSB first shifted from MSB side
			}
			//else don't write cause we already initialised the buffer with zero
			PIN_WRITE &= ~(1<<ICSP_CLK_PIN);	//Make clock low	
			asm("nop");
			asm("nop");
			asm("nop");
		}
    }

    temp >>= (8 - numbits);    // right justify

	if (!(icsp_pins & 0x02)) 
	{
		PIN_DDR |= (1 << ICSP_DAT_PIN); 	//Set ICSP_DATA_PIN to output
	}

	sei();
    return temp;

}

/******************************************************************************
 * Function:        void ShiftBitsOutICSP(byte outputbyte, char numbits)
 * Overview:        Shifts the given # bits out on the ICSP pins 
 *
 * Input:           outputbyte - byte to be shifted out LSB first
 * Output:          None
 *
 * Note:            Assumes ICSP pins are already set to outputs.
 *****************************************************************************/
extern void 
ShiftBitsOutICSP(byte outputbyte, char numbits)
{
	char i, t;
	byte mask = 1;						//Bitmask: 00000001
	
	PIN_DDR |= (1<<ICSP_DAT_PIN);		//Set ICSP_DAT_PIN to output
	PIN_DDR |= (1<<ICSP_CLK_PIN);		//Set ICSP_CLK_PIN to output

	cli();

    if (icsp_baud < 2)
    {
    	for(i=0;i<numbits;i++)
		{
			if (outputbyte & mask)
			{
				PIN_WRITE |= (1<<ICSP_DAT_PIN);		//Write high
			}
			else
			{
				PIN_WRITE &= ~(1<<ICSP_DAT_PIN);	//Write low
			}
			asm("nop");
			PIN_WRITE |= (1<<ICSP_CLK_PIN);			//Make clock high
			asm("nop");
			PIN_WRITE &= ~(1<<ICSP_CLK_PIN);		//Make clock low
			
			mask <<= 1;								//iterate trough mask
		}
    }
    else
    {
        icsp_baud -= 1;
		for(i=0;i<numbits;i++)
		{
			if (outputbyte & mask)
			{
				PIN_WRITE |= (1<<ICSP_DAT_PIN);		//Write high
			}
			else
			{
				PIN_WRITE &= ~(1<<ICSP_DAT_PIN);	//Write low
			}
			asm("nop");
			PIN_WRITE |= (1<<ICSP_CLK_PIN);			//Make clock high
			for(t=0;t<icsp_baud;t++);				// delay 6 cycles per count
			asm("nop");
			PIN_WRITE &= ~(1<<ICSP_CLK_PIN);		//Make clock low
			
			mask <<= 1;								//iterate trough mask
		}
    }
	sei();
}


/******************************************************************************
 * Function:        void ShiftBitsOutICSPHold(byte outputbyte, char numbits)
 * Overview:        Shifts the given # bits out on the ICSP pins 
 *                  Differs from ShiftBitsOutICSP in that the instead of
 *                  Setting data, delay, clock high, delay, clock low
 *                  This routine works as
 *                  Setting data, clock high, delay, clock low, delay
 *
 * Input:           outputbyte - byte to be shifted out LSB first
 * Output:          None
 *
 * Note:            Assumes ICSP pins are already set to outputs.
 *****************************************************************************/
extern void 
ShiftBitsOutICSPHold(byte outputbyte, char numbits)
{
	char i, t;
	byte mask = 1;						//Bitmask: 00000001
	
	PIN_DDR |= (1<<ICSP_DAT_PIN);		//Set ICSP_DAT_PIN to output
	PIN_DDR |= (1<<ICSP_CLK_PIN);		//Set ICSP_CLK_PIN to output

	cli();

    if (icsp_baud < 2)
    {
    	for(i=0;i<numbits;i++)
		{
			if (outputbyte & mask)
			{
				PIN_WRITE |= (1<<ICSP_DAT_PIN);		//Write high
			}
			else
			{
				PIN_WRITE &= ~(1<<ICSP_DAT_PIN);	//Write low
			}
			PIN_WRITE |= (1<<ICSP_CLK_PIN);			//Make clock high
			asm("nop");
			PIN_WRITE &= ~(1<<ICSP_CLK_PIN);		//Make clock low
			asm("nop");
			
			mask <<= 1;								//iterate trough mask
		}
    }
    else
    {
        icsp_baud -= 1;
		for(i=0;i<numbits;i++)
		{
			if (outputbyte & mask)
			{
				PIN_WRITE |= (1<<ICSP_DAT_PIN);		//Write high
			}
			else
			{
				PIN_WRITE &= ~(1<<ICSP_DAT_PIN);	//Write low
			}
			PIN_WRITE |= (1<<ICSP_CLK_PIN);			//Make clock high
			for(t=0;t<icsp_baud;t++);				// delay 6 cycles per count
			asm("nop");
			PIN_WRITE &= ~(1<<ICSP_CLK_PIN);		//Make clock low
			asm("nop");
			
			mask <<= 1;								//iterate trough mask
		}
    }
	sei();
}


/******************************************************************************
 * Function:        void SetICSP_PinStates(byte icsp_byte)
 * Overview:        Sets the value and direction of the ICSP pins.
 *
 * Input:           icsp_byte - byte formated
 *   					<7 – 4> unused
 *   					<3> PGD logic level
 *   					<2> PGC logic level
 *   					<1> 1= PGD input, 0= output
 *    					<0> 1= PGC input, 0= output
 * Output:          Affects bits in DDRB and PORTB
 *****************************************************************************/
extern void 
SetICSP_PinStates(byte icsp_byte)
{
				// set ISCPCLK direction
				if (icsp_byte & 0x01)
					PIN_DDR &= ~(1 << ICSP_CLK_PIN); 	//Set ICSP_DATA_PIN to input	
				else
					PIN_DDR |= (1 << ICSP_CLK_PIN); 	//Set ICSP_CLK_PIN to output
				// set ISCDAT direction
				if (icsp_byte & 0x02)
					PIN_DDR &= ~(1 << ICSP_DAT_PIN); 	//Set ICSP_DATA_PIN to input
				else
					PIN_DDR |= (1 << ICSP_DAT_PIN); 	//Set ICSP_DATA_PIN to output
				
				// set ISCPCLK latch
				if (icsp_byte & 0x04)
					PIN_WRITE |= (1<<ICSP_CLK_PIN);	
				else
					PIN_WRITE &= ~(1<<ICSP_CLK_PIN);	
				// set ISCDAT latch
				if (icsp_byte & 0x08)
					PIN_WRITE |= (1<<ICSP_DAT_PIN);		
				else
					PIN_WRITE &= ~(1<<ICSP_DAT_PIN);
}

/******************************************************************************
 * Function:        byte GetICSP_PinStates(void)
 * Overview:        Gets the values of the ICSP pins.
 *
 * Input:           icsp_byte - byte formated
 *   					<7 – 4> unused
 *   					<3> PGD logic level
 *   					<2> PGC logic level
 * Output:          returns a byte with bits:
 *                      <1> PGD state
 *    					<0> PGC state
 *****************************************************************************/
extern byte 
GetICSP_PinStates(void)
{
    byte states = 0;

    if ( PIN_READ & (1<<ICSP_DAT_PIN) )	//ICSP_DAT_PIN high?
    {
        states |= 0x02;
    }
    if (PIN_READ & (1<<ICSP_CLK_PIN))  //ICSP_CLK_PIN high?
    {
        states |= 0x01;
    }

    return states;
}
