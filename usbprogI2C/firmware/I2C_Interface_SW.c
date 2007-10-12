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
/*	Revised: 8/10/05 (ChrisK) added clock stretch timeout through		*/
/*		conditional compile flag										*/
/*	Revised: 8/10/05 (ChrisK) added clock stretching timeout			*/
/*																		*/
/************************************************************************/


/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */

#include "I2C_Interface_SW.h"

/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */

BOOL fAck = fTrue;


/* ------------------------------------------------------------ */
/***	PrepareAck
**
**	Synopsis:
**
**	Parameters:
**
**	Return Values:
**
**	Errors:
**
**	Description: sets the global fAck
**
*/

/* ------------------------------------------------------------ */

void PrepareAck(void) { fAck = fTrue; }


/* ------------------------------------------------------------ */
/***	PrepareNack
**
**	Synopsis:
**
**	Parameters:
**
**	Return Values:
**
**	Errors:
**
**	Description: sets the global fAck
**
*/

/* ------------------------------------------------------------ */

void PrepareNack(void) { fAck = fFalse; }



/* ------------------------------------------------------------ */
/***	I2cInit
**
**	Synopsis:	 I2cInit();
**
**	Parameters:	none
**
**	Return Value:	none
**
**	Errors:	none
**
**	Description:	Prepares the SCK and SDA pins for i2c
**		operation.
**
**	Exit conditions:	sck will be output high
**						sda will be input high
**
*/

/* ------------------------------------------------------------ */

void I2cInit(void)
{
	fAck = fTrue;

	// make sda pin an input to allow it to be pulled high
	i2cDDRD &= ~( 1 << i2cSDA );
	i2cPORTD |= ( 1 << i2cSDA );

	// make sck pin high and output
	i2cPORTC |= ( 1 << i2cSCK );
	i2cDDRC |= ( 1 << i2cSCK );

#ifdef ADD_I2C_DELAY
// if the delay is defined as a global variable
// that can be modified at runtime, uncomment this
// line to set the default delay value

//	TDELAY = 1;
#endif

	return;
}


/* ------------------------------------------------------------ */
/***	Start
**
**	Synopsis:	Start();
**
**	Parameters:		None
**
**	Return Value:	None
**
**	Errors:		None
**
**	Description:	i2c Start condition generator, while SCK
**		is high, SDA is pulled low.
**
**	Entering Cond.:		expects the sda pin to be an input high
**						expects the sck line to be output high
**
**	Exit conditions:	sck will be output low
**						sda will be output low
*/

/* ------------------------------------------------------------ */

void Start(void)
{
	fAck = fTrue;

	// sda value written as 0
	i2cPORTD &= ~( 1 << i2cSDA );

	// sda pin made an output
	i2cDDRD |= ( 1 << i2cSDA );

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	// clock low
#ifndef CLOCK_STRETCHING
	// sck driven low
	i2cPORTC &= ~( 1 << i2cSCK );
#endif
#ifdef CLOCK_STRETCHING
	// make sck an output low
	i2cPORTC &= ~( 1 << i2cSCK );
	i2cDDRC |= ( 1 << i2cSCK );
#endif

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	return;
}


/* ------------------------------------------------------------ */
/***	SendByte
**
**	Synopsis:	SendByte(bData);
**
**	Parameters:	bData, a byte to be sent via i2c
**
**	Return Value:	none
**
**	Errors:	none
**
**	Description:	after the start condition is generated
**		i2c devices will be listening, use this function to
**		transmit 1 byte of data.  shifts out a byte, msb first
**		bData is latched in on the rising edge
**		and must be stable through the entire
**		high clock pulse
**
**	Entering Cond.:	expects the sck line to be output low
**
**
**	Exit conditions:	sck will be output low
**						sda will be unknown
*/

/* ------------------------------------------------------------ */

void SendByteI2C(BYTE bData)
{
	BYTE	cbit;

	for(cbit = 0 ; cbit < 8 ; cbit++)
	{
		if( ( bData << cbit & 0b10000000 ) != 0 )
		{
#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif
			// make sda pin an input to allow it to be pulled high
			i2cDDRD &= ~( 1 << i2cSDA );
			i2cPORTD |= ( 1 << i2cSDA );

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

			// clock high
#ifndef CLOCK_STRETCHING
			// bring the clock high
			i2cPORTC |= ( 1 << i2cSCK );
#endif
#ifdef CLOCK_STRETCHING
			// make sck input HIGH
			i2cDDRC &= ~( 1 << i2cSCK );
			i2cPORTC |= ( 1 << i2cSCK );

#ifdef CLOCK_STETCH_TIME_OUT
// reset the clock count
clockCount = 0;
#endif
			// wait for the line to be pulled high
			while((i2cPINC & ( 1 << i2cSCK )) == 0)
			{
#ifdef CLOCK_STETCH_TIME_OUT
				if(clockCount == CLOCK_STRETCH_MAX)
				{
					break;
				}

			clockCount++;
#endif
			}
#endif

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

		}
		else
		{
			// sda value written as 0
			i2cPORTD &= ~( 1 << i2cSDA );
			// sda pin made an output
			i2cDDRD |= ( 1 << i2cSDA );

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

			// clock high
#ifndef CLOCK_STRETCHING
			// bring the clock high
			i2cPORTC |= ( 1 << i2cSCK );
#endif
#ifdef CLOCK_STRETCHING
			// make sck input HIGH
			i2cDDRC &= ~( 1 << i2cSCK );
			i2cPORTC |= ( 1 << i2cSCK );

#ifdef CLOCK_STETCH_TIME_OUT
// reset the clock count
clockCount = 0;
#endif
			// wait for the line to be pulled high
			while((i2cPINC & ( 1 << i2cSCK )) == 0)
			{
#ifdef CLOCK_STETCH_TIME_OUT
				if(clockCount == CLOCK_STRETCH_MAX)
				{
					break;
				}

			clockCount++;
#endif
			}
#endif

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif
		}

		// clock low
#ifndef CLOCK_STRETCHING
		// sck driven low
		i2cPORTC &= ~( 1 << i2cSCK );
#endif
#ifdef CLOCK_STRETCHING
		// make sck an output low
		i2cPORTC &= ~( 1 << i2cSCK );
		i2cDDRC |= ( 1 << i2cSCK );
#endif

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif
	}

	return;
}

/* ------------------------------------------------------------ */
/***	FReceiveAck
**
**	Synopsis:	fSuccess = FReceiveAck();
**
**	Parameters:	none
**
**	Return Value:	BOOL, fTrue indicates the an ACK was recieved
**		fFalse indicates than an ACK was NOT received
**
**	Errors:	none
**
**	Description:	FReceiveAck
**		will generate the next clock and receive
**		the acknowledge bit, It will return fFalse
**		if no ack is recieved, fTrue if it does
**		recieve an ACK
**
**	Entering Cond.:	expects sck to be output low
**
**
**	Exit conditions:	sck will be output low
**						sda will be an input
*/

/* ------------------------------------------------------------ */

BOOL FReceiveAck(void)
{
	// make the sda an input
	i2cDDRD &= ~( 1 << i2cSDA );
	i2cPORTD |= ( 1 << i2cSDA );

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	// clock high
#ifndef CLOCK_STRETCHING
	// bring the clock high
	i2cPORTC |= ( 1 << i2cSCK );
#endif
#ifdef CLOCK_STRETCHING
	// make sck input HIGH
	i2cDDRC &= ~( 1 << i2cSCK );
	i2cPORTC |= ( 1 << i2cSCK );

#ifdef CLOCK_STETCH_TIME_OUT
// reset the clock count
clockCount = 0;
#endif
			// wait for the line to be pulled high
			while((i2cPINC & ( 1 << i2cSCK )) == 0)
			{
#ifdef CLOCK_STETCH_TIME_OUT
				if(clockCount == CLOCK_STRETCH_MAX)
				{
					break;
				}

			clockCount++;
#endif
			}
#endif

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	// read the acknowledge bit
	if( ( i2cPIND & ( 1 << i2cSDA ) ) == 0 )
	{
		// clock low
#ifndef CLOCK_STRETCHING
		// sck driven low
		i2cPORTC &= ~( 1 << i2cSCK );
#endif
#ifdef CLOCK_STRETCHING
		// make sck an output low
		i2cPORTC &= ~( 1 << i2cSCK );
		i2cDDRC |= ( 1 << i2cSCK );
#endif

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

		// return a "true"
		return fTrue;
	}
	else
	{
		// clock low
#ifndef CLOCK_STRETCHING
		// sck driven low
		i2cPORTC &= ~( 1 << i2cSCK );
#endif
#ifdef CLOCK_STRETCHING
		// make sck an output low
		i2cPORTC &= ~( 1 << i2cSCK );
		i2cDDRC |= ( 1 << i2cSCK );
#endif

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

		// return a "false"
		return fFalse;
	}
}


/* ------------------------------------------------------------ */
/***	Stop
**
**	Synopsis:	Stop();
**
**	Parameters:	none
**
**	Return Value:	none
**
**	Errors: if this function returns and the sck line is still
**		low, that is a clock stretching timeout error.
**
**	Description:	i2c STOP condition generator, pulls
**		SDA from low to high while SCK is high
**
**	Entering Cond.:	expects the sda pin to be an input
**					expects the sck line to be output low
**
**	Exit conditions:	sda will be output high
**						sck will be input
*/

/* ------------------------------------------------------------ */

void Stop(void)
{
	// make sda output low
	i2cPORTD &= ~( 1 << i2cSDA );
	i2cDDRD |= ( 1 << i2cSDA );

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	// clock high
#ifndef CLOCK_STRETCHING
	// bring the clock high
	i2cPORTC |= ( 1 << i2cSCK );
#endif
#ifdef CLOCK_STRETCHING
	// make sck input HIGH
	i2cDDRC &= ~( 1 << i2cSCK );
	i2cPORTC |= ( 1 << i2cSCK );

#ifdef CLOCK_STETCH_TIME_OUT
// reset the clock count
clockCount = 0;
#endif
			// wait for the line to be pulled high
			while((i2cPINC & ( 1 << i2cSCK )) == 0)
			{
#ifdef CLOCK_STETCH_TIME_OUT
				if(clockCount == CLOCK_STRETCH_MAX)
				{
					break;
				}

			clockCount++;
#endif
			}
#endif

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	// bring SDA high
	i2cDDRD &= ~( 1 << i2cSDA );
	i2cPORTD |= ( 1 << i2cSDA );

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	return;
}


/* ------------------------------------------------------------ */
/***	ReStart
**
**	Synopsis:	ReStart();
**
**	Parameters:		none
**
**	Return Value: none
**
**	Errors: none
**
**	Description: Generates the i2c restart condition
**
**	Entering Cond.:	expecting sck output low
**
**
**	Exit conditions:	ends with sda and sck output low
**
*/

/* ------------------------------------------------------------ */

void ReStart(void)
{
	// bring sda high
	i2cDDRD &= ~( 1 << i2cSDA );
	i2cPORTD |= ( 1 << i2cSDA );

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	// clock high
#ifndef CLOCK_STRETCHING
	// bring the clock high
	i2cPORTC |= ( 1 << i2cSCK );
#endif
#ifdef CLOCK_STRETCHING
	// make sck input HIGH
	i2cDDRC &= ~( 1 << i2cSCK );
	i2cPORTC |= ( 1 << i2cSCK );

#ifdef CLOCK_STETCH_TIME_OUT
// reset the clock count
clockCount = 0;
#endif
			// wait for the line to be pulled high
			while((i2cPINC & ( 1 << i2cSCK )) == 0)
			{
#ifdef CLOCK_STETCH_TIME_OUT
				if(clockCount == CLOCK_STRETCH_MAX)
				{
					break;
				}

			clockCount++;
#endif
			}
#endif

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	//bring SDA low
	i2cPORTD &= ~( 1 << i2cSDA );
	i2cDDRD |= ( 1 << i2cSDA );

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	// clock low
#ifndef CLOCK_STRETCHING
	// sck driven low
	i2cPORTC &= ~( 1 << i2cSCK );
#endif
#ifdef CLOCK_STRETCHING
	// make sck an output low
	i2cPORTC &= ~( 1 << i2cSCK );
	i2cDDRC |= ( 1 << i2cSCK );
#endif

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	return;
}

/* ------------------------------------------------------------ */
/***	BReceiveByte
**
**	Synopsis:	bReceived = BReceiveByte();
**
**	Parameters:	none
**
**	Return Value:	BYTE value sent from i2c slave
**
**	Errors:	none
**
**	Description:	this function reads a byte from the i2c slave
**		msb first, data latched on the rising edge.  when all
**		8 bits are received, the BYTE value is returned
**
**	Entering Cond.:	expects sck output low
**
**
**	Exit conditions:	exits with sck low, and sda an input
**
*/

/* ------------------------------------------------------------ */

BYTE BReceiveByteI2C(void)
{
	BYTE cbit, bReceived = 0;

	// make SDA an input
	i2cDDRD &= ~( 1 << i2cSDA );
	i2cPORTD |= ( 1 << i2cSDA );

	for( cbit =0 ; cbit < 8 ; cbit++ )
	{

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

		// clock high
#ifndef CLOCK_STRETCHING
		// bring the clock high
		i2cPORTC |= ( 1 << i2cSCK );
#endif
#ifdef CLOCK_STRETCHING
		// make sck input HIGH
		i2cDDRC &= ~( 1 << i2cSCK );
		i2cPORTC |= ( 1 << i2cSCK );

#ifdef CLOCK_STETCH_TIME_OUT
// reset the clock count
clockCount = 0;
#endif
			// wait for the line to be pulled high
			while((i2cPINC & ( 1 << i2cSCK )) == 0)
			{
#ifdef CLOCK_STETCH_TIME_OUT
				if(clockCount == CLOCK_STRETCH_MAX)
				{
					break;
				}

			clockCount++;
#endif
			}
#endif

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

		if( ( i2cPIND & 1 << i2cSDA ) != 0 )
		{
			bReceived |= 1 << ( 7 - cbit );
		}

		// clock low
#ifndef CLOCK_STRETCHING
		// sck driven low
		i2cPORTC &= ~( 1 << i2cSCK );
#endif
#ifdef CLOCK_STRETCHING
		// make sck an output low
		i2cPORTC &= ~( 1 << i2cSCK );
		i2cDDRC |= ( 1 << i2cSCK );
#endif
	}

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	if(fAck == fTrue) SendAck();
	else SendNack();

	return bReceived;
}


/* ------------------------------------------------------------ */
/***	SendAck
**
**	Synopsis:	SendAck();
**
**	Parameters:	none
**
**	Return Value: none
**
**	Errors:	none
**
**	Description: when receiving information, this function
**		generates the ACK to indicate to the slave to continue
**		sending information
**
**	Entering Cond.:	 clock low sda input
**
**
**	Exit conditions:	clock low, sda output low
**
*/

/* ------------------------------------------------------------ */

void SendAck(void)
{
	//bring SDA low
	i2cPORTD &= ~( 1 << i2cSDA );
	i2cDDRD |= ( 1 << i2cSDA );

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	// clock high
#ifndef CLOCK_STRETCHING
	// bring the clock high
	i2cPORTC |= ( 1 << i2cSCK );
#endif
#ifdef CLOCK_STRETCHING
	// make sck input HIGH
	i2cDDRC &= ~( 1 << i2cSCK );
	i2cPORTC |= ( 1 << i2cSCK );

#ifdef CLOCK_STETCH_TIME_OUT
// reset the clock count
clockCount = 0;
#endif
			// wait for the line to be pulled high
			while((i2cPINC & ( 1 << i2cSCK )) == 0)
			{
#ifdef CLOCK_STETCH_TIME_OUT
				if(clockCount == CLOCK_STRETCH_MAX)
				{
					break;
				}

			clockCount++;
#endif
			}
#endif

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	// clock low
#ifndef CLOCK_STRETCHING
	// sck driven low
	i2cPORTC &= ~( 1 << i2cSCK );
#endif
#ifdef CLOCK_STRETCHING
	// make sck an output low
	i2cPORTC &= ~( 1 << i2cSCK );
	i2cDDRC |= ( 1 << i2cSCK );
#endif

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	return;
}

/* ------------------------------------------------------------ */
/***	SendNack
**
**	Synopsis:	SendNack();
**
**	Parameters:	none
**
**	Return Value: none
**
**	Errors:	none
**
**	Description: when receiving information, this function
**		generates the NACK to indicate to the slave to stop
**		sending information
**
**	Entering Cond.:	 clock low sda input
**
**
**	Exit conditions:	clock low, sda input high
**
*/

/* ------------------------------------------------------------ */

void SendNack(void)
{
	// make SDA an input
	i2cDDRD &= ~( 1 << i2cSDA );
	i2cPORTD |= ( 1 << i2cSDA );

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	// clock high
#ifndef CLOCK_STRETCHING
	// bring the clock high
	i2cPORTC |= ( 1 << i2cSCK );
#endif
#ifdef CLOCK_STRETCHING
	// make sck input HIGH
	i2cDDRC &= ~( 1 << i2cSCK );
	i2cPORTC |= ( 1 << i2cSCK );

#ifdef CLOCK_STETCH_TIME_OUT
// reset the clock count
clockCount = 0;
#endif
			// wait for the line to be pulled high
			while((i2cPINC & ( 1 << i2cSCK )) == 0)
			{
#ifdef CLOCK_STETCH_TIME_OUT
				if(clockCount == CLOCK_STRETCH_MAX)
				{
					break;
				}

			clockCount++;
#endif
			}
#endif

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	// clock low
#ifndef CLOCK_STRETCHING
	// sck driven low
	i2cPORTC &= ~( 1 << i2cSCK );
#endif
#ifdef CLOCK_STRETCHING
	// make sck an output low
	i2cPORTC &= ~( 1 << i2cSCK );
	i2cDDRC |= ( 1 << i2cSCK );
#endif

#ifdef ADD_I2C_DELAY
	_delay_loop_2(TDELAY);
#endif

	return;
}
