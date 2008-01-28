/*****************************************************************************
* file: scripts.c
* description: Scripting functions
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
#include "../../pk_comm.h"
#include "scripts.h"
#include "../icsp/icsp.h"

/******************************************************************************
 * Function:        void SendScriptChecksums(void)
 * Overview:        Calculates and writes with checksums of the Script Buffer in the USB outbuf.
 *
 * Input:           None
 * Output:          Transmits HID Tx report with 4 bytes.
 *                      First 2 bytes are a 16-bit sum of the lengths in the 
 *                           ScriptTable
 *                      Second 2 are 16-bit sum of all used script bytes.
 *****************************************************************************/
extern void 
SendScriptChecksums(void)
{
    int length_checksum = 0;
    int buffer_checksum = 0;
    int i;
    
    for (i = 0; i < SCRIPT_ENTRIES; i++)
    {
        length_checksum += ScriptTable[i].Length;
    }

    for (i = 0; i < length_checksum; i++)
    {
        buffer_checksum += *(uc_ScriptBuf_ptr + i);
    }

    outbuf[0] = (length_checksum & 0xFF);
    outbuf[1] = (length_checksum >> 8);
	outbuf[2] = (buffer_checksum & 0xFF);;
	outbuf[3] = (buffer_checksum >> 8);;
}


/******************************************************************************
 * Function:        void RunScript(byte scriptnumber, byte repeat)
 * Overview:        Runs a given script# from the Script Buffer "repeat" number of
 *                  times.
 *
 * Input:           scriptnumber = # of script to run
 *                  repeat = how many times to run the script
 * Output:          usbprogPICstatus.EmptyScript set if no script at given script#
 *****************************************************************************/
extern void 
RunScript(byte scriptnumber, byte repeat)
{
    // check for valid script #
    if ((scriptnumber >= SCRIPT_ENTRIES) || (ScriptTable[scriptnumber].Length == 0))
    {
        usbprogPICstatus.EmptyScript = 1;  // set error
        return;
    }

    do
    {
        ScriptEngine((uc_ScriptBuf_ptr + ScriptTable[scriptnumber].StartIndex) , ScriptTable[scriptnumber].Length);
        repeat--;
    } while (repeat > 0);

}


/******************************************************************************
 * Function:        void ClearScriptTable(void)
 * Overview:        Clears Script buffer by setting all Script Table length entries to zero.
 *
 * Input:           None.
 * Output:          ScriptTable[x].Length = 0 for all valid x.
 *****************************************************************************/
extern void 
ClearScriptTable(void)
{
    byte i;

	for (i=0; i < SCRIPT_ENTRIES; i++) // init script table to empty.
	{
		ScriptTable[i].Length = 0;
	}
}

/******************************************************************************
 * Function:        void StoreScriptInBuffer(byte *usbindex)
 * Overview:        Stores the script from USB buffer into Script Buffer & updates
 *                  the Script Table.
 *                  Prior script at the given script # is deleted and all following
 *                  scripts are moved up.  New script is appended at end.
 *
 * Input:           *usbindex - index to script # byte in USB buffer
 * Output:          uc_script_buffer[] - updated
 *                  ScriptTable[] - updated
 *                  usbprogPICstatus.ScriptBufOvrFlow - set if script length > remaining buffer
 *****************************************************************************/
extern void 
StoreScriptInBuffer(byte *usbindex)
{
	int i;
	int LengthOfAllScripts;
	int Temp_1, Temp_2;

	Temp_1 = inbuf[(*usbindex)+1];  	// Length of new script

	// First, make sure script length is valid
	if (Temp_1 > SCRIPT_MAXLEN)
	{
		usbprogPICstatus.ScriptBufOvrFlow = 1;		// set error - script longer than max allowed
		return;
	}

	Temp_2 = inbuf[*usbindex];		// Script# of new script 

	// calculate length of all scripts.
	LengthOfAllScripts = 0;
	for (i=0; i < SCRIPT_ENTRIES; i++)
	{
		LengthOfAllScripts += ScriptTable[i].Length;
	}
	LengthOfAllScripts -= ScriptTable[Temp_2].Length;	// don't count length of script being replaced
	if (Temp_1 > (SCRIPTBUFSPACE-LengthOfAllScripts)) // if there isn't enough room
	{
		usbprogPICstatus.ScriptBufOvrFlow = 1;		// set error - not enough room in script buffer
		return;
	}	

	// Next, make sure script# is valid
	if (Temp_2 > (SCRIPT_ENTRIES-1))    // 0-31 valid
	{
		usbprogPICstatus.ScriptBufOvrFlow = 1;		// set error - script# invalid
		return;
	}


	if (ScriptTable[Temp_2].Length != 0)  // If a script exists in that location
	{
		// Move space created by deleting existing script to end of buffer.
		Temp_1 = (SCRIPTBUFSPACE - ScriptTable[Temp_2].Length) - 1;  // last copy location.
		for (i=ScriptTable[Temp_2].StartIndex; i < Temp_1; i++)
		{
			*(uc_ScriptBuf_ptr + i) = *(uc_ScriptBuf_ptr + ScriptTable[Temp_2].Length + i);
		}
		// update script table entries
		for (i=0; i < SCRIPT_ENTRIES; i++)
		{
			if (ScriptTable[i].StartIndex > ScriptTable[Temp_2].StartIndex) // if script is in moved section
			{
				ScriptTable[i].StartIndex -= ScriptTable[Temp_2].Length;  // adjust by amount moved
			} 
		}
	}

	// Store new script at end of buffer
	ScriptTable[Temp_2].Length = inbuf[(*usbindex)+1];	// update Script Table Entry with new length.
	ScriptTable[Temp_2].StartIndex = LengthOfAllScripts;    // update entry with new index at end of buffer.
	*usbindex += 2;	// point to first byte of new script in USB buffer.
	for (i = 0; i < ScriptTable[Temp_2].Length; i++)
	{
		*(uc_ScriptBuf_ptr + LengthOfAllScripts + i) = 	inbuf[(*usbindex)++];	
	}  

} 

/******************************************************************************
 * Function:        void ScriptEngine(byte *scriptstart_ptr, byte scriptlength) *
 * Overview:        Executes the script pointed to by scriptstart_ptr from the Script Buffer
 *					Aborts if a control byte attempts to use a byte from an empty Download buffer
 *                  or store a byte in a full Upload Buffer.  Will not execute if 
 *                  usbprogPICstatus.StatusHigh != 0 (a script error exists.)
 *
 * Input:           *scriptstart_ptr - Pointer to start of script
 *                  scriptlength - length of script
 * Output:          uc_downloadbuf_read - advanced by the number of bytes read.
 *                  uc_upload_buffer[] - new data may be stored
 *                  uc_uploadbuf_write - advance by number of bytes written.
 *                  usbprogPICstatus.StatusHigh - set if script error occurs
 *
 * Side Effects:    Uses Timer0.
 *****************************************************************************/
extern void 
ScriptEngine(byte *scriptstart_ptr, byte scriptlength)
{
	byte scriptindex = 0;
	byte temp_byte = 0;
	byte loopcount = 0;
	byte loopindex = 0;
	byte loopbufferindex = 0;
    unsigned int loopbuffercount = 0;
    byte *SFR_ptr;
	bool loopactive = 0;
    bool loopbufferactive = 0;

    TIMSK &= ~(1<<TOIE1);				// ensure Timer1 interrupt disabled.
    TCCR1B |= (1<<CS12);       			// 16-bit timer, 1:256 prescale.

	if ((scriptlength == 0) || (scriptlength > SCRIPT_MAXLEN))
	{
		usbprogPICstatus.EmptyScript = 1;		// set error - script length out of bounds
		return;		
	}

	while (((usbprogPICstatus.StatusHigh & STATUSHI_ERRMASK) == 0) && (scriptindex < scriptlength))
	{
		switch (*(scriptstart_ptr + scriptindex))
		{
            // NOTE : More frequently used cases are placed near the end of the switch-case
            // as they are compared first, to speed up execution.
			case VDD_ON:
				//Vdd_TGT_P = 0;
				scriptindex++;
				break;

			case VDD_OFF:
				//Vdd_TGT_P = 1;
				scriptindex++;
				break;

			case VDD_GND_ON:
				//Vdd_TGT_N = 1;
				scriptindex++;
				break;

			case VDD_GND_OFF:
				//Vdd_TGT_N = 0;
				scriptindex++;
				break;

			case VPP_ON:
				//Vpp_ON = 1;
				scriptindex++;
				break;

			case VPP_OFF:
				//Vpp_ON = 0;
				scriptindex++;
				break;

			case VPP_PWM_ON:
				scriptindex++;
				break;

			case VPP_PWM_OFF:
                //Vpp_PUMP = 0;               // pass VDD
				scriptindex++;
				break;

			case MCLR_GND_ON:
				//MCLR_TGT = 1;
				scriptindex++;
				break;

			case MCLR_GND_OFF:
				//MCLR_TGT = 0;
				scriptindex++;
				break;

			case BUSY_LED_ON:
				LED_on;
				scriptindex++;
				break;

			case BUSY_LED_OFF:
				LED_off;
				scriptindex++;
				break;

			case DELAY_LONG:
				scriptindex++;
				LongDelay(*(scriptstart_ptr + scriptindex));
				scriptindex++;
				break;

			case SET_ICSP_PINS:
				scriptindex++;
				icsp_pins = *(scriptstart_ptr + scriptindex);
				SetICSP_PinStates(icsp_pins);
				scriptindex++;
				break;

			case SET_ICSP_SPEED:
				scriptindex++;
                icsp_baud = *(scriptstart_ptr + scriptindex);
				scriptindex++;
				break;

			case LOOPBUFFER:
				if (loopbufferactive)
                {
                    loopbuffercount--;
                    if (loopbuffercount == 0)
                    {
                        loopbufferactive = 0;
                        scriptindex+=2;
                        break;    
                    }
                    scriptindex = loopbufferindex;
                    break;
                }
				loopbufferindex = scriptindex - *(scriptstart_ptr + scriptindex + 1);
                loopbuffercount = (unsigned int) ReadDownloadBuffer();  // low byte
                loopbuffercount += (256 * ReadDownloadBuffer());        // upper byte
                if (loopbuffercount == 0)
                { // value of "zero" 0x0000 means no loops.
                    scriptindex+=2;
                    break;
                }
                loopbufferactive = 1;
                scriptindex = loopbufferindex;
				break;

			case READ_BYTE:
				ShiftBitsInICSP(8);
				scriptindex++;
				break;

			case READ_BITS:
				scriptindex++;
				ShiftBitsInICSP(*(scriptstart_ptr + scriptindex));
				scriptindex++;
				break;

			case LOOP:
				if (loopactive)
                {
                    loopcount--;
                    if (loopcount == 0)
                    {
                        loopactive = 0;
                        scriptindex+=3;
                        break;    
                    }
                    scriptindex = loopindex;
                    break;
                }
                loopactive = 1;
				loopindex = scriptindex - *(scriptstart_ptr + scriptindex + 1);
                loopcount = *(scriptstart_ptr + scriptindex + 2);
                scriptindex = loopindex;
				break;

			case DELAY_SHORT:
				scriptindex++;
				ShortDelay(*(scriptstart_ptr + scriptindex));
				scriptindex++;
				break;

			case READ_BITS_BUFFER:
                scriptindex++;
				WriteUploadBuffer(ShiftBitsInICSP(*(scriptstart_ptr + scriptindex)));
				scriptindex++;
				break;

			case WRITE_BITS_BUFFER:
				scriptindex++;
                ShiftBitsOutICSP(ReadDownloadBuffer(), *(scriptstart_ptr + scriptindex));
				scriptindex++;
				break;

			case READ_BYTE_BUFFER:
				WriteUploadBuffer(ShiftBitsInICSP(8));
				scriptindex++;
				break;

			case WRITE_BYTE_LITERAL:
				scriptindex++;
                ShiftBitsOutICSP(*(scriptstart_ptr + scriptindex), 8);
				scriptindex++;
				break;

			case WRITE_BITS_LITERAL:
                scriptindex++;
                ShiftBitsOutICSP(*(scriptstart_ptr + scriptindex + 1), *(scriptstart_ptr + scriptindex));
				scriptindex+=2;
				break;

			case WRITE_BYTE_BUFFER:
                ShiftBitsOutICSP(ReadDownloadBuffer(), 8);
				scriptindex++;
				break;

			default:
                if (*(scriptstart_ptr + scriptindex) >= WRITE_BITS_BUF_HLD)
                {
    				switch (*(scriptstart_ptr + scriptindex))
                    {
                        case CONST_WRITE_DL:
                            scriptindex++;
                            WriteByteDownloadBuffer(*(scriptstart_ptr + scriptindex));
                            scriptindex++;
    				        break; 
    
                        case POP_DOWNLOAD:
                            ReadDownloadBuffer();
    				        scriptindex++;
    				        break;    
    
                        case RD2_BITS_BUFFER: 
                            scriptindex++;
            				WriteUploadBuffer(ShiftBitsInPIC24(*(scriptstart_ptr + scriptindex)));
            				scriptindex++;
            				break;   
    
    					case WRITE_BITS_LIT_HLD:
    		                scriptindex++;
    		                ShiftBitsOutICSPHold(*(scriptstart_ptr + scriptindex + 1), *(scriptstart_ptr + scriptindex));
    						scriptindex+=2;
    						break;
    		
    					case WRITE_BITS_BUF_HLD:
    						scriptindex++;
    		                ShiftBitsOutICSPHold(ReadDownloadBuffer(), *(scriptstart_ptr + scriptindex));
    						scriptindex++;
    						break;                
    
                        case ICSP_STATES_BUFFER:
                            WriteUploadBuffer(GetICSP_PinStates());
    				        scriptindex++;
    				        break;
                            
                        case IF_EQ_GOTO:
                            temp_byte = uc_upload_buffer[uploadbuf_mgmt.write_index - 1]; // last byte written
                            if (temp_byte == *(scriptstart_ptr + scriptindex + 1))
                            {
    				            scriptindex = scriptindex + (signed char)*(scriptstart_ptr + scriptindex + 2);                       
                            }
                            else
                            {
    				            scriptindex+=3;
                            }
    			        	break;
    
                        case IF_GT_GOTO:
                            temp_byte = uc_upload_buffer[uploadbuf_mgmt.write_index - 1]; // last byte written
                            if (temp_byte > *(scriptstart_ptr + scriptindex + 1))
                            {
    				            scriptindex = scriptindex + (signed char)*(scriptstart_ptr + scriptindex + 2);                       
                            }
                            else
                            {
    				            scriptindex+=3;
                            }
    			        	break;
    
                        case GOTO_INDEX:
    				        scriptindex = scriptindex + (signed char)*(scriptstart_ptr + scriptindex + 1);                       
    			        	break;
    
                        case POKE_SFR:
    				        scriptindex++;
                            SFR_ptr = (byte *)0x0F00 + *(scriptstart_ptr + scriptindex++);
                            *SFR_ptr = *(scriptstart_ptr + scriptindex++);
    			        	break;
    
                        case PEEK_SFR:
    				        scriptindex++;
                            SFR_ptr = (byte *)0x0F00 + *(scriptstart_ptr + scriptindex);
                            WriteUploadBuffer(*SFR_ptr);
    				        scriptindex++;
    			        	break;
    
                        case WRITE_BUFBYTE_W:
                            scriptindex++;
                            ShiftBitsOutICSP(0, 4); // six code
                            ShiftBitsOutICSP(*(scriptstart_ptr + scriptindex), 4); // W nibble
                            ShiftBitsOutICSP(ReadDownloadBuffer(), 8); // literal LSB
                            ShiftBitsOutICSP(0, 8); // literal MSB
                            ShiftBitsOutICSP(0x2, 4); // opcode
    				        scriptindex++;
    			        	break;
    
                        case WRITE_BUFWORD_W:
                            scriptindex++;
                            ShiftBitsOutICSP(0, 4); // six code
                            ShiftBitsOutICSP(*(scriptstart_ptr + scriptindex), 4); // W nibble
                            ShiftBitsOutICSP(ReadDownloadBuffer(), 8); // literal LSB
                            ShiftBitsOutICSP(ReadDownloadBuffer(), 8); // literal MSB
                            ShiftBitsOutICSP(0x2, 4); // opcode
    				        scriptindex++;
    			        	break;
    
    					case VISI24:
    						scriptindex++;
    						ShiftBitsOutICSP(1, 4);
    						ShiftBitsOutICSP(0, 8);
    						WriteUploadBuffer(ShiftBitsInPIC24(8));
    						WriteUploadBuffer(ShiftBitsInPIC24(8));
    						break;
    
    					case NOP24:
    						scriptindex++;
    						ShiftBitsOutICSP(0, 4);
    						ShiftBitsOutICSP(0, 8);
    						ShiftBitsOutICSP(0, 8);
    						ShiftBitsOutICSP(0, 8);
    						break;
    
    					case COREINST18:
    						scriptindex++;
    						ShiftBitsOutICSP(0, 4);
    						ShiftBitsOutICSP(*(scriptstart_ptr + scriptindex++), 8);
    						ShiftBitsOutICSP(*(scriptstart_ptr + scriptindex++), 8);
    						break;
    
    					case COREINST24:
    						scriptindex++;
    						ShiftBitsOutICSP(0, 4);
    						ShiftBitsOutICSP(*(scriptstart_ptr + scriptindex++), 8);
    						ShiftBitsOutICSP(*(scriptstart_ptr + scriptindex++), 8);
    						ShiftBitsOutICSP(*(scriptstart_ptr + scriptindex++), 8);
    						break;
    
                        case ICDSLAVE_RX:
                            scriptindex++;
                            break;
    
                        case ICDSLAVE_TX_LIT:
                            scriptindex++;
                            break;
    
                        case ICDSLAVE_TX_BUF:
                            scriptindex++;
                            break;
    
    					case RD2_BYTE_BUFFER:
    						scriptindex++;
    						WriteUploadBuffer(ShiftBitsInPIC24(8));
    						break;
        
                        case EXIT_SCRIPT:
                        default:
                            scriptindex = scriptlength;
                    } // end switch case
                }
                else
                {
                    switch (*(scriptstart_ptr + scriptindex))
                    {
            			case SET_AUX:
            				scriptindex++;
            				aux_pin = *(scriptstart_ptr + scriptindex);
            				//SetAUX_PinState(aux_pin);
            				scriptindex++;
            				break;

                        case AUX_STATE_BUFFER:
                            //WriteUploadBuffer(GetAUX_PinState());
    				        scriptindex++;
    				        break;

                        case I2C_START:
    				        scriptindex++;
    				        break;

                        case I2C_STOP:
    				        scriptindex++;
    				        break;

            			case I2C_WR_BYTE_LIT:
            				scriptindex++;
            				scriptindex++;
            				break;

            			case I2C_WR_BYTE_BUF:
            				scriptindex++;
            				break;

                        case I2C_RD_BYTE_ACK:
            				scriptindex++;
            				break;

                        case I2C_RD_BYTE_NACK:
            				scriptindex++;
            				break;

                        case SPI_WR_BYTE_LIT:
                            scriptindex++;
              				scriptindex++;
                            break;

                        case SPI_WR_BYTE_BUF:
                            scriptindex++;
               				break;

                        case SPI_RD_BYTE_BUF:
            				scriptindex++;
            				break;

                        case SPI_RDWR_BYTE_LIT:
                            scriptindex++;
            				scriptindex++;
                            break;

                        case SPI_RDWR_BYTE_BUF:
                            scriptindex++;
            				scriptindex++;
                            break;

                        default:
                            scriptindex = scriptlength;
                    } // end switch case
                } //end if else
		} // end switch-case
	} // end;

}

