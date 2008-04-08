/*-------------------------------------------------------------------------
 * JTAG.H
 * Copyright (C) 2003 Armand ten Doesschate <a.doesschate@hccnet.nl>
 * Copyright (C) 2008 Martin Lang <Martin.Lang@rwth-aachen.de>
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
 *
 *----------------------------------------------------------------------*/

#ifndef JTAG_AVR_DEFINES_H
#define JTAG_AVR_DEFINES_H

#include "constant.h"

/*
 * JTAG reset lasts 5 times
 */
#define RESET_CNT                       5

/*
 * JTAG instructions for AVR
 */
#define EXTEST                          0
#define AVR_IDCODE                      1
#define AVR_PRG_ENABLE                  4
#define AVR_PRG_CMDS                    5
#define AVR_PRG_PAGE_LOAD               6
#define AVR_PRG_PAGE_READ               7
#define AVR_FORCE_BRK                   8
#define AVR_RUN                         9
#define AVR_INSTR                       10
#define AVR_OCD                         11
#define AVR_RESET                       12
#define BYPASS                          15

/*
 * the AVR debugging register addresses :
 * all registers are 16 bits
 */
#define AVR_PSB0                        0
#define AVR_PSB1                        1
#define AVR_PDMSB                       2
#define AVR_PDSB                        3
#define AVR_BCR                         8  /* break control register  */
#define AVR_BSR                         9  /* break control and status
                                              register */
#define AVR_DBG_COMM_DATA               12 /* communication data OCDR
                                              register */
#define AVR_DBG_COMM_CTL                13 /* control and status of OCDR
                                              register */

/*
 * BCR bits in high byte of break control register
 * please note that breakpoint modes of pdsb and pdmsb
 * are like the mode parameter in SET_BREAK command
 */

#define AVR_PDSB_MODE0                  0x0008
#define AVR_PDSB_MODE1                  0x0010
#define AVR_PDMSB_MODE0                 0x0020
#define AVR_PDMSB_MODE1                 0x0040
#define AVR_EN_PDSB                     0x0080
#define AVR_EN_PDMSB                    0x0100
#define AVR_MASK_BREAK                  0x0200
#define AVR_EN_PSB1                     0x0400
#define AVR_EN_PSB0                     0x0800
#define AVR_BRK_ON_FLOW                 0x1000
#define AVR_BRK_STEP                    0x2000
#define AVR_PC_MOD                      0x4000
#define AVR_TMR_RUN_ON_BRK              0x8000

/* Bits in SPMCR */
#define AVR_SPMCR_SPMEN 0x01
#define AVR_SPMCR_PGERS 0x02
#define AVR_SPMCR_PGWRT 0x04
#define AVR_SPMCR_BLBSET 0x08
#define AVR_SPMCR_RWWSRE 0x10
#define AVR_SPMCR_RWWSB 0x40

/*
 * BSR bits
 */
#define AVR_BRK_BRK_INSTR               1
#define AVR_BRK_FORCED                  2
#define AVR_BRK_STAT                    4
#define AVR_BRK_PDSB                    8
#define AVR_BRK_PDMSB                   0x10
#define AVR_BRK_PSB1                    0x20
#define AVR_BRK_PSB2                    0x40
#define AVR_BRK_FLOW                    0x80

/*
 * OCDR status bits
 */
#define AVR_SET_OCDR                    4
#define AVR_RES_1                       8
#define AVR_WR_OCDR                     0x10
/*
 * this is in high byte
 */
#define AVR_EN_OCDR                     0x8000

enum jtag_data_state {
    CHK_BRK = 1,
    IDLE = 2
};


#define JTAG_OK  1
#define AVR  1


enum {
  CHK_PC = 1, /* get program counter +2 or +4 depending on the AVR
		                   instruction */
	LONG        /* 32 bits instruction when data or I/O is queried  */
};

#define DEBUG_ON	//Debugmode


#endif
