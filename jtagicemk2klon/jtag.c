/* jtag.h
* Copyright (C) 2007  Benedikt Sauter, sauter@ixbat.de
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "jtag.h"
#include "bit.h"
#include "uart.h"


volatile TAP_STATE tapstate;

void jtag_init(void)
{
	// use as output
	JTAG_PORT_INIT |= (1<<TCK)|(1<<TMS)|(1<<TDI);
	// use as input
	JTAG_PORT_INIT &=~(1<<TDO);

	// pullup
	JTAG_PORT_WRITE |= (1<<TDO);
	//JTAG_PORT_WRITE |= (1<<TDO);

	// use as input
	JTAG_CLEAR_TCK();
	JTAG_CLEAR_TMS();
	JTAG_CLEAR_TDI();
	JTAG_CLEAR_TCK();

	jtag_reset();
}

uint8_t jtag_read_tdo(void)
{
	if(JTAG_IS_TDO_SET())
		return 1;
	else
		return 0;
}

void jtag_send_slice(uint8_t tck, uint8_t tms, uint8_t tdi)
{

	if(tms)
		JTAG_PORT_WRITE |= (1<<TMS);
	else
		JTAG_PORT_WRITE &= ~(1<<TMS);

	if(tdi)
		JTAG_PORT_WRITE |= (1<<TDI);
	else
		JTAG_PORT_WRITE &= ~(1<<TDI);

	if(tck)
		JTAG_PORT_WRITE |= (1<<TCK);
	else
		JTAG_PORT_WRITE &= ~(1<<TCK);

}

int jtag_reset(void)
{
	int i;
	JTAG_SET_TMS();
	for(i=0;i<5;i++) {
		JTAG_CLK();
	}
	JTAG_CLEAR_TMS();
	tapstate = TEST_LOGIC_RESET;

	return 1;
}

uint8_t jtag_read(uint8_t numberofbits, unsigned char * buf)
{
  int receivedbits = 0;
	JTAG_CLEAR_TMS();				// last one with tms
	JTAG_CLK();

  while(numberofbits--) {
		if(numberofbits==0){
	 		JTAG_SET_TMS();				// last one with tms
			if(tapstate==SHIFT_IR){
				tapstate = EXIT1_IR;
			} else {
				tapstate = EXIT1_DR;
			}
		}

		//if(numberofbits==1)
	  //	JTAG_SET_TMS();			// last one with tms

		if(JTAG_IS_TDO_SET())
			buf[receivedbits/8] |= (1 << (receivedbits & 7));
		else
			buf[receivedbits/8] &= ~(1 << (receivedbits & 7));

		receivedbits++;
		JTAG_CLK();
  }

	return receivedbits;
}

uint8_t jtag_write(uint8_t numberofbits, unsigned char * buf)
{
	int sendbits=0;

	// if numbers is not vaild
	if(numberofbits<=0)
		return -1;

	JTAG_CLEAR_TMS();				// last one with tms

	//numberofbits--;
	while(numberofbits--)
	{
		if(numberofbits==0)
		{
	 		JTAG_SET_TMS();				// last one with tms
			if(tapstate==SHIFT_IR)
			{
				tapstate = EXIT1_IR;
			}
			else
			{
				tapstate = EXIT1_DR;
			}
		}

		if(buf[sendbits/8] >> (sendbits & 7) & 1)
		{
			JTAG_SET_TDI();
		}
		else
		{
			JTAG_CLEAR_TDI();
		}

		sendbits++;
		JTAG_CLK();
	}
	return sendbits;
}



uint8_t jtag_write_and_read(	uint8_t numberofbits,
															unsigned char * buf,
															unsigned char * readbuf)
{
	int bits=0;

	// if numbers is not vaild
	if(numberofbits<=0)
		return -1;

	JTAG_CLEAR_TMS();				// last one with tms

  while(numberofbits--) {
		if(numberofbits==0){
	 		JTAG_SET_TMS();				// last one with tms
			if(tapstate==SHIFT_IR){
				tapstate = EXIT1_IR;
			} else {
				tapstate = EXIT1_DR;
			}
		}
		if(buf[bits/8] >> (bits & 7) & 1)
			JTAG_SET_TDI();
		else
			JTAG_CLEAR_TDI();

		JTAG_CLK();

		if(JTAG_IS_TDO_SET())
			readbuf[bits/8] |= (1 << (bits & 7));
		else
			readbuf[bits/8] &= ~(1 << (bits & 7));


	  bits++;
	}
	return bits;
}


void jtag_goto_state(TAP_STATE state)
{
  /* If 'state' is invalid, simply ignore it */
  if( state > UPDATE_IR ) return;
	//SendHex(tapstate);
	//SendHex(state);
  while( tapstate != state ) {
  	//if (debug_verbose != 0)
			//SendHex(tapstate);
		switch( tapstate ) {
			case TEST_LOGIC_RESET:
			  JTAG_CLEAR_TMS();
	  		JTAG_CLK();
	  		tapstate = RUN_TEST_IDLE;
	  	break;

			case RUN_TEST_IDLE:
	  		JTAG_SET_TMS();
	  		JTAG_CLK();
	  		tapstate = SELECT_DR_SCAN;
	  	break;

			case SELECT_DR_SCAN:
	  		if( state < TEST_LOGIC_RESET ){
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = CAPTURE_DR;
	    	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = SELECT_IR_SCAN;
	    	}
	  	break;

			case CAPTURE_DR:
	  		if( state == SHIFT_DR ){
      		JTAG_CLEAR_TMS();
      		JTAG_CLK();
      		tapstate = SHIFT_DR;
    		} else {
	    		JTAG_SET_TMS();
	    		JTAG_CLK();
	    		tapstate = EXIT1_DR;
	  		}
			break;

			case SHIFT_DR:
	  		JTAG_SET_TMS();
	  		JTAG_CLK();
	  		tapstate = EXIT1_DR;
	  	break;

			case EXIT1_DR:
	  		if( state == PAUSE_DR || state == EXIT2_DR ) {
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = PAUSE_DR;
	   	 	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = UPDATE_DR;
	    	}
	  	break;

			case PAUSE_DR:
	  		JTAG_SET_TMS();
	  		JTAG_CLK();
	  		tapstate = EXIT2_DR;
	  	break;

			case EXIT2_DR:
	  		if( state == SHIFT_DR || state == EXIT1_DR || state == PAUSE_DR ){
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = SHIFT_DR;
	    	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = UPDATE_DR;
	    	}
	  	break;

			case UPDATE_DR:
	  		if( state == RUN_TEST_IDLE ) {
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = RUN_TEST_IDLE;
	    	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = SELECT_DR_SCAN;
	    	}
	  	break;

			case SELECT_IR_SCAN:
	  		if( state != TEST_LOGIC_RESET ){
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = CAPTURE_IR;
	    	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = TEST_LOGIC_RESET;
	    	}
	  	break;

			case CAPTURE_IR:
	  		if( state == SHIFT_IR ){
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = SHIFT_IR;
	    	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = EXIT1_IR;
	    	}
	  	break;

			case SHIFT_IR:
	  		JTAG_SET_TMS();
	  		JTAG_CLK();
	  		tapstate = EXIT1_IR;
	  	break;

			case EXIT1_IR:
	  		if( state == PAUSE_IR || state == EXIT2_IR ) {
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = PAUSE_IR;
	    	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = UPDATE_IR;
	    	}
	  	break;

			case PAUSE_IR:
	  		JTAG_SET_TMS();
	  		JTAG_CLK();
	  		tapstate = EXIT2_IR;
	  	break;

			case EXIT2_IR:
	  		if( state == SHIFT_IR  || state == EXIT1_IR  || state == PAUSE_IR ) {
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = SHIFT_IR;
	    	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = UPDATE_IR;
	    	}
	  	break;

			case UPDATE_IR:
	  		if( state == RUN_TEST_IDLE ){
	      	JTAG_CLEAR_TMS();
	      	JTAG_CLK();
	      	tapstate = RUN_TEST_IDLE;
	    	} else {
	      	JTAG_SET_TMS();
	      	JTAG_CLK();
	      	tapstate = SELECT_DR_SCAN;
	    	}
	  	break;
			default:
				return;
		}
  }
	return;
}

void jtag_clock_cycles(uint8_t num)
{
	JTAG_CLEAR_TMS();

	while (num--) {
		JTAG_CLK();
	}
}
