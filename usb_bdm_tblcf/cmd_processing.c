/*
    Turbo BDM Light ColdFire - USB command processing
    Copyright (C) 2005  Daniel Malik

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "MC68HC908JB16.h"
#include "hidef.h"
#include "options.h"
#include "commands.h"
#include "cmd_processing.h"
#include "version.h"
#include "usb.h"
#include "led.h"
#include "boot.h"
#include "bdmcf.h"

#pragma DATA_SEG Z_RAM
cable_status_t near cable_status;

#pragma DATA_SEG DEFAULT

/* processes all commands received over USB */
/* the command is expected to be in command_buffer+1  */
/* returns number of bytes left in the buffer (at position command_buffer+0) to be sent back as response */
unsigned char command_exec(void) {
  led_state = LED_BLINK;                          /* blink the LED to indicate a command */
  if (command_buffer[1]==CMD_GET_LAST_STATUS) {   /* need to process this special command before status of the last command is lost */
    return(1);      
  }
  command_buffer[0] = command_buffer[1];      /* assume the command will execute OK */
  switch (command_buffer[1]) {                /* commands which execute the same way irrespective of selected target type */
    case CMD_GET_VER:                         /* get HW & SW version */
      *((unsigned int *)(command_buffer+1)) = VERSION;
      return(3);                              /* return cmd + 2 bytes of version */
    case CMD_SET_TARGET:                      /* set target type */
      cable_status.target_type = command_buffer[2];
      if (cable_status.target_type==CF_BDM) {
        bdmcf_init();                         /* initialise the BDM interface */
        bdmcf_resync();                       /* synchronize with the target */
        return(1);
      } 
      if (cable_status.target_type==JTAG) {
				jtag_init();                          /* initialise JTAG */
        return(1);
      }
      break;                                  /* unknown target type */
    case CMD_SET_BOOT:                        /* request bootloader action on next power-up */
      if ((command_buffer[2]=='B')&&(command_buffer[3]=='O')&&(command_buffer[4]=='O')&&(command_buffer[5]=='T')) {
        force_bootloader();                   /* program the flash */    
        return(1);
      }
      break;
    case CMD_RESET:                           /* reset; one 8-bit parameter: ==0 reset to BDM mode, !=0 reset to normal mode */
      bdmcf_reset(command_buffer[2]);
      return(1);
    case CMD_GET_STATUS:                      /* returns 16-bit status of the cable; bit0 - target reset detected, bit1 - current state of the RSTO pin */
      command_buffer[1]=0;									  /* the PCB has a pull-down on the input, so only trust that RSTO is low if an edge was detected */
      command_buffer[2]=0;									  /* cannot put a pull-up on the pin as the single layer PCB is too tight to allow it */
      if (cable_status.reset==RESET_DETECTED) {
        command_buffer[2]|=RESET_DETECTED_MASK;   /* reset detected */
        cable_status.reset=NO_RESET_ACTIVITY;		  /* clear the flag */
      }
      #ifdef INVERT         
        if (RSTO_IN==0) command_buffer[2]|=RSTO_STATE_MASK;  /* the RSTO pin is currently high */
      #else
        if (RSTO_IN==1) command_buffer[2]|=RSTO_STATE_MASK;  /* the RSTO pin is currently high */
      #endif
      return(3);
    #ifdef STACK_SIZE_EVALUATION
      case CMD_GET_STACK_SIZE:                /* parameters: none, returns 16-bit stack size required by the application so far */
		    {
		      unsigned char *ptr;
		      ptr = (unsigned char *)__SEG_START_SSTACK;
          while ((*ptr)==0x55) ptr++;
          *((unsigned int *)(command_buffer+1)) = (unsigned char *)__SEG_END_SSTACK - ptr;
		    }
        return(2);
    #endif  
    default:
      if (cable_status.target_type==CF_BDM) {   /* commands which execute depending on the selected target type */
        switch (command_buffer[1]) {
          case CMD_HALT:                        /* stop execution of user code by asserting the BKPT line; no parameters */
            bdmcf_halt();
            return(1);
          case CMD_GO:                          /* start code execution from current PC address; no parameters */
            bdmcf_tx_msg(BDMCF_CMD_RDMREG);     /* get CSR */
            if (bdmcf_rx(2,command_buffer+12)) break; /* CSR is received into command_data+12,+13,+14,+15 */
            *(command_buffer+15)&=~0x10;        /* clear the SSM bit */
            *((unsigned int *)(command_buffer+10))=BDMCF_CMD_WDMREG;
            bdmcf_tx(3,command_buffer+10);			/* write the CSR back */
					  if (bdmcf_complete_chk(BDMCF_CMD_GO)) break; /* GO */
						#ifdef CMD_COMPLETE_CHECK
						  if (bdmcf_complete_chk_rx()) break;
						#endif
            return(1);
          case CMD_STEP:                        /* step over a single instruction; no parameters */
            bdmcf_tx_msg(BDMCF_CMD_RDMREG);     /* get CSR */
            if (bdmcf_rx(2,command_buffer+12)) break; /* CSR is received into command_data+12,+13,+14,+15 */
            *(command_buffer+15)|=0x10;         /* set the SSM bit - Single Step Mode */
            *((unsigned int *)(command_buffer+10))=BDMCF_CMD_WDMREG;
            bdmcf_tx(3,command_buffer+10);			/* write the CSR back */
					  if (bdmcf_complete_chk(BDMCF_CMD_GO)) break; /* GO */
						#ifdef CMD_COMPLETE_CHECK
						  if (bdmcf_complete_chk_rx()) break;
						#endif
            return(1);
          case CMD_READ_CREG:                   /* read control register; parameter 16-bit register address, returns 32-bit control register contents */
            bdmcf_tx_msg(BDMCF_CMD_RCREG);      /* send the command */
            bdmcf_tx_msg(0);                    /* and the register address */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+2)));
            if (bdmcf_rx(2,command_buffer+1)) break; /* the 4 bytes of the register contents are received into command_buffer+1,+2,+3,+4 */
            return(5);          
          case CMD_WRITE_CREG:									/* write control register; parameter 16-bit register address & the 32-bit control register contents to be written */
            bdmcf_tx_msg(BDMCF_CMD_WCREG);      /* send the command */
            bdmcf_tx_msg(0);                    /* the register address */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+2)));
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+4))); /* and the register value */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+6)));
						#ifdef CMD_COMPLETE_CHECK
						  if (bdmcf_complete_chk_rx()) break;
						#endif
            return(1);          
          case CMD_READ_DREG:										/* read debug register; parameter 8-bit register number to read, returns 32-bit debug module register contents */
            bdmcf_tx_msg(BDMCF_CMD_RDMREG+command_buffer[2]);   /* send the command */
            if (bdmcf_rx(2,command_buffer+1)) break;            /* the 4 bytes of the register contents are received into command_buffer+1,+2,+3,+4 */
            return(5);
          case CMD_WRITE_DREG:                  /* write debug register; parameter 8-bit register number to write & the 32-bit debug module register contents to be written */
            bdmcf_tx_msg(BDMCF_CMD_WDMREG+command_buffer[2]);   /* send the command */
            if (bdmcf_tx_msg_half_rx(*((unsigned int *)(command_buffer+3)))) break;	 /* and the register value */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+5))); 
						#ifdef CMD_COMPLETE_CHECK
						  if (bdmcf_complete_chk_rx()) break;
						#endif
            return(1);          
          case CMD_READ_REG:                    /* read address/data register; parameter 8-bit register number to read, returns 32-bit register contents */
            bdmcf_tx_msg(BDMCF_CMD_RAREG+command_buffer[2]);   /* send the command */
            if (bdmcf_rx(2,command_buffer+1)) break;           /* the 4 bytes of the register contents are received into command_buffer+1,+2,+3,+4 */
            return(5);
          case CMD_WRITE_REG:                   /* write address/data register; parameter 8-bit register number to write & the 32-bit register contents to be written */
            bdmcf_tx_msg(BDMCF_CMD_WAREG+command_buffer[2]);    /* send the command */
            if (bdmcf_tx_msg_half_rx(*((unsigned int *)(command_buffer+3)))) break;	 /* and the register value */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+5))); 
						#ifdef CMD_COMPLETE_CHECK
						  if (bdmcf_complete_chk_rx()) break;
						#endif
            return(1);          
          case CMD_READ_MEM8:                   /* read a byte from memory; parameter 32bit address, returns 8bit value read from address */
            bdmcf_tx_msg(BDMCF_CMD_READ8);      /* send the command */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+2))); /* and the address */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+4))); 
            if (bdmcf_rx(1,command_buffer+1)) break; /* read the result into command_buffer+1 */
            *(command_buffer+1)=*(command_buffer+2); /* the byte is LSB of the received word, copy it to the right place */
            return(2);
          case CMD_READ_MEM16:                  /* read a word from memory; parameter 32bit address, returns 16bit value read from address */
            bdmcf_tx_msg(BDMCF_CMD_READ16);     /* send the command */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+2))); /* and the address */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+4))); 
            if (bdmcf_rx(1,command_buffer+1)) break; /* read the result into command_buffer+1,+2 */
            return(3);
          case CMD_READ_MEM32:                  /* read a double-word from memory; parameter 32bit address, returns 32bit value read from address */
            bdmcf_tx_msg(BDMCF_CMD_READ32);     /* send the command */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+2))); /* and the address */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+4))); 
            if (bdmcf_rx(2,command_buffer+1)) break; /* read the result into command_buffer+1,+2,+3,+4 */
            return(5);
          case CMD_WRITE_MEM8:                  /* write a byte to memory; parameter 32bit address & an 8-bit value to be written to the address */ 
            bdmcf_tx_msg(BDMCF_CMD_WRITE8);     /* send the command */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+2))); /* the address */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+4))); 
            bdmcf_tx_msg(*(command_buffer+6));  /* and the data to be written */
						#ifdef CMD_COMPLETE_CHECK
						  if (bdmcf_complete_chk_rx()) break;
						#endif
					  return(1);
          case CMD_WRITE_MEM16:                 /* write a word to memory; parameter 32bit address & a 16-bit value to be written to the address */ 
            bdmcf_tx_msg(BDMCF_CMD_WRITE16);    /* send the command */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+2))); /* the address */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+4))); 
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+6))); /* and the data to be written */
						#ifdef CMD_COMPLETE_CHECK
						  if (bdmcf_complete_chk_rx()) break;
						#endif
					  return(1);
          case CMD_WRITE_MEM32:                 /* write a double-word to memory; parameter 32bit address & a 32-bit value to be written to the address */ 
            bdmcf_tx_msg(BDMCF_CMD_WRITE32);    /* send the command */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+2))); /* the address */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+4))); 
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+6))); /* and the data to be written */
            bdmcf_tx_msg(*((unsigned int *)(command_buffer+8)));
						#ifdef CMD_COMPLETE_CHECK
						  if (bdmcf_complete_chk_rx()) break;
						#endif
					  return(1);
          case CMD_READ_MEMBLOCK8:                /* reads a block of bytes; parameter 32bit address; the number of bytes to read is given by command_size (the number of bytes requested by the host -1) */
            {
              unsigned char i;
              unsigned char *ptr;
              bdmcf_tx_msg(BDMCF_CMD_READ8);      /* send read byte command */
              bdmcf_tx_msg(*((unsigned int *)(command_buffer+2))); /* and the address */
              bdmcf_tx_msg(*((unsigned int *)(command_buffer+4))); 
              i=command_size;
              ptr=command_buffer+1;               /* where first result should go */
							do {
							  i--;                              /* decrement the number of bytes to read */
                if (i) {
                  if (bdmcf_rxtx(1,ptr,BDMCF_CMD_DUMP8)) break;  /* get the result & send in new DUMP command */
                } else {
                  if (bdmcf_rx(1,ptr)) {          /* read the result (and send NOP) */
                    i=1;                          /* make i non-zero */
                    break;     
                  }
                }
                *(ptr)=*(ptr+1);                  /* the byte is LSB of the received word, copy it to the right place */
                ptr++;                                
							} while(i);
              if (i) break;                       /* an error has occured */
              return(command_size+1);
            }
          case CMD_READ_MEMBLOCK16:               /* reads a block of words; parameter 32bit address; the number of bytes to read is given by command_size (the number of bytes requested by the host -1) */
            {
              unsigned char i;
              unsigned char *ptr;
              bdmcf_tx_msg(BDMCF_CMD_READ16);     /* send read byte command */
              bdmcf_tx_msg(*((unsigned int *)(command_buffer+2))); /* and the address */
              bdmcf_tx_msg(*((unsigned int *)(command_buffer+4))); 
              i=(command_size>>1);                /* the number of words is the bytecount/2 */
              ptr=command_buffer+1;               /* where first result should go */
							do {
							  i--;                              /* decrement the number of bytes to read */
                if (i) {
                  if (bdmcf_rxtx(1,ptr,BDMCF_CMD_DUMP16)) break;  /* get the result & send in new DUMP command */
                } else {
                  if (bdmcf_rx(1,ptr)) {          /* read the result (and send NOP) */
                    i=1;                          /* make i non-zero */
                    break;     
                  }
                }
                ptr+=2;                                
							} while(i);
              if (i) break;                       /* an error has occured */
              return(command_size+1);
            }
          case CMD_READ_MEMBLOCK32:               /* reads a block of dwords; parameter 32bit address; the number of bytes to read is given by command_size (the number of bytes requested by the host -1) */
            {
              unsigned char i;
              unsigned char *ptr;
              bdmcf_tx_msg(BDMCF_CMD_READ32);     /* send read byte command */
              bdmcf_tx_msg(*((unsigned int *)(command_buffer+2))); /* and the address */
              bdmcf_tx_msg(*((unsigned int *)(command_buffer+4))); 
              i=(command_size>>2);                /* the number of dwords is the bytecount/4 */
              ptr=command_buffer+1;               /* where first result should go */
							do {
							  i--;                              /* decrement the number of bytes to read */
                if (i) {
                  if (bdmcf_rxtx(2,ptr,BDMCF_CMD_DUMP32)) break;  /* get the result & send in new DUMP command */
                } else {
                  if (bdmcf_rx(2,ptr)) {          /* read the result (and send NOP) */
                    i=1;                          /* make i non-zero */
                    break;     
                  }
                }
                ptr+=4;
							} while(i);
              if (i) break;                       /* an error has occured */
              return(command_size+1);
            }
          case CMD_WRITE_MEMBLOCK8:               /* writes a block of words; parameters 32bit address & data to write; the number of bytes to write is given by command_size */
            {
              unsigned char i;
              unsigned char *ptr;
              bdmcf_tx_msg(BDMCF_CMD_WRITE8);     /* send write byte command */
              bdmcf_tx_msg(*((unsigned int *)(command_buffer+2))); /* the address */
              bdmcf_tx_msg(*((unsigned int *)(command_buffer+4))); 
              bdmcf_tx_msg(*(command_buffer+6));  /* and the data */
              i=command_size-4-1;                 /* the address has 4 bytes & done 1 byte already */
              ptr=command_buffer+7;
              while(i) {
      				  if (bdmcf_complete_chk(BDMCF_CMD_FILL8)) break; /* send write byte command */
                bdmcf_tx_msg(*ptr);               /* and the data */
                ptr++;                                
                i--;
              }
              if (i) break;                       /* an error has occured */
      				#ifdef CMD_COMPLETE_CHECK
  						  if (bdmcf_complete_chk_rx()) break;
      				#endif
              return(1);
            }
          case CMD_WRITE_MEMBLOCK16:              /* writes a block of words; parameters 32bit address & data to write; the number of bytes to write is given by command_size */
            {
              unsigned char i;
              unsigned char *ptr;
              bdmcf_tx_msg(BDMCF_CMD_WRITE16);    /* send write byte command */
              bdmcf_tx_msg(*((unsigned int *)(command_buffer+2))); /* the address */
              bdmcf_tx_msg(*((unsigned int *)(command_buffer+4))); 
              bdmcf_tx_msg(*((unsigned int *)(command_buffer+6))); /* and the data */
              i=(command_size-4-2)>>1;            /* the address has 4 bytes & done 1 word already, every word has 2 bytes */
              ptr=command_buffer+8;
              while(i) {
      				  if (bdmcf_complete_chk(BDMCF_CMD_FILL16)) break; /* send write word command */
                bdmcf_tx_msg(*(unsigned int *)ptr); /* and the data */
                ptr+=2;                                
                i--;
              }
              if (i) break;                       /* an error has occured */
      				#ifdef CMD_COMPLETE_CHECK
  						  if (bdmcf_complete_chk_rx()) break;
      				#endif
              return(1);
            }
          case CMD_WRITE_MEMBLOCK32:              /* writes a block of dwords; parameters 32bit address & data to write; the number of bytes to write is given by command_size */
            {
              unsigned char i;
              unsigned char *ptr;
              bdmcf_tx_msg(BDMCF_CMD_WRITE32);    /* send write byte command */
              bdmcf_tx_msg(*((unsigned int *)(command_buffer+2))); /* the address */
              bdmcf_tx_msg(*((unsigned int *)(command_buffer+4))); 
              bdmcf_tx_msg(*((unsigned int *)(command_buffer+6))); /* and the data */
              bdmcf_tx_msg(*((unsigned int *)(command_buffer+8)));
              i=(command_size-4-4)>>2;            /* the address has 4 bytes & done 1 dword already, every dword has 4 bytes */
              ptr=command_buffer+10;
              while(i) {
      				  if (bdmcf_complete_chk(BDMCF_CMD_FILL32)) break; /* send write dword command */
                bdmcf_tx_msg(*(unsigned int *)(ptr+0)); /* and the data */
                bdmcf_tx_msg(*(unsigned int *)(ptr+2));
                ptr+=4;                                
                i--;
              }
              if (i) break;                       /* an error has occured */
      				#ifdef CMD_COMPLETE_CHECK
  						  if (bdmcf_complete_chk_rx()) break;
      				#endif
              return(1);
            }
          case CMD_RESYNCHRONIZE:		              /* resync communication with the target MCU */
            if (bdmcf_resync()) break;            /* try to resynchronize */
            return(1);
          case CMD_ASSERT_TA:                     /* assert the TA signal, parameter: 8-bit number of 10us ticks - duration of the TA assertion */
            bdmcf_ta(command_buffer[2]);
            return(1);
          default:                                /* unknown command */
            command_buffer[0] = CMD_UNKNOWN;
            return(1);    
        }
				bdmcf_complete_chk_rx();                  /* send at least 2 nops to purge the BDM of the offending command */
				bdmcf_complete_chk_rx();
      } else if (cable_status.target_type==JTAG) {
        switch (command_buffer[1]) {
          case CMD_JTAG_GOTORESET:								/* no parameters, takes the TAP to TEST-LOGIC-RESET state, re-select the JTAG target to take TAP back to RUN-TEST/IDLE */
					  jtag_transition_reset();
					  return(1);
					case CMD_JTAG_GOTOSHIFT:								/* parameters 8-bit path option; path option ==0 : go to SHIFT-DR, !=0 : go to SHIFT-IR (requires the tap to be in RUN-TEST/IDLE) */
					  jtag_transition_shift(command_buffer[2]);
					  return(1);
					case CMD_JTAG_WRITE:										/* parameters 8-bit exit option, 8-bit count of bits to shift in, and the data to be shifted in (shifted in LSB (last byte) first, unused bits (if any) are in the MSB (first) byte; exit option ==0 : stay in SHIFT-xx, !=0 : go to RUN-TEST/IDLE when finished */
					  jtag_write(command_buffer[2], command_buffer[3], command_buffer+4);
            return(1);
					case CMD_JTAG_READ:											/* parameters 8-bit exit option, 8-bit count of bits to shift out; exit option ==0 : stay in SHIFT-xx, !=0 : go to RUN-TEST/IDLE when finished, returns the data read out of the device (first bit in LSB of the last byte in the buffer) */
					  {
					    unsigned char i;
					    i=command_buffer[3]>>3;             /* calculate the number of bytes to return */
					    if ((command_buffer[3]&0x07)==0) i++;
					    jtag_read(command_buffer[2], command_buffer[3], command_buffer+1);
              return(i+1);
					  }
          default:                                /* unknown command */
            command_buffer[0] = CMD_UNKNOWN;
            return(1);    
        }
      } else {
        command_buffer[0] = CMD_UNKNOWN;
        return(1);    
      }
  }
  command_buffer[0] = CMD_FAILED;                 /* if any of the case statements falls through, the command has failed */
  return(1);  
}

