/*-------------------------------------------------------------------------
 * JTAG_AVR_OCD.C
 * Copyright (C) 2003 Armand ten Doesschate <a.doesschate@hccnet.nl>
 * Copyright (C) 2007 Benedikt Sauter <sauter@ixbat.de>
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

#include "jtag_avr_prg.h"
#include "jtag.h"
#include "jtag_avr.h"
#include "jtag_avr_defines.h"

/*----------------------------------------------------------------------*
 * This function reads or writes the peripherals, internal memory, CPU  *
 * registers of the AVR target by executing AVR target instructions.    *
 *                                                                      *
 * Note :                                                               *
 * The jtag interface cannot access the peripherals or memory directly. *
 * Instead it sends and receives commands through use of the ocdr       *
 * register which is mapped in the AVR core. The cpu core will access   *
 * the peripheral and memory by executing the "normal" avr instructions.*
 * The ocdr register isnot enabled by default and must be enabled when  *
 * the avr target is stopped succefully.                                *
 *                                                                      *
 * A special case is for access of the program counter : this program   *
 * counter can be accessed by a 32 bit data in which the first 16 bits  *
 * are 0xffff.                                                          *
 *                                                                      *
 * Every avr instruction transmitted to the jtag data register must end *
 * with an run-test/idle stage, while the access of the ocd register    *
 * set is not required                                                  *
 *----------------------------------------------------------------------*/
static unsigned char
exec_instr_avr (unsigned char *out, unsigned char *in, unsigned char flg,
                unsigned char delay)
{
    unsigned char tdi [4], tdo [4], stat;

    stat      = avr_jtag_instr (AVR_INSTR, delay);
    if (stat == JTAG_OK) {
        if (flg == CHK_PC) {
            tdi [0] = 0;
            tdi [1] = 0;
            tdi [2] = 0;
            tdi [3] = 0;
            stat = jtag_write_and_read (32,tdo, tdi);
            if (stat == JTAG_OK) {
                out [0] = tdo [0];
                out [1] = tdo [1];
                out [2] = tdo [2];
                out [3] = tdo [3];
            }
        }
        else { 
            /*
             * an avr instruction 
             */
            tdi [0] = in [0];
            tdi [1] = in [1];
            stat = jtag_write_and_read (16,tdo, tdi);
        }
    }
    return stat;
}

/*----------------------------------------------------------------------*
 * write to on chip debugging registers                                 *
 *----------------------------------------------------------------------*/
static unsigned char
wr_dbg_ocd (unsigned char reg, unsigned char *buf, unsigned delay)
{
    unsigned char stat, tdi [3], tdo [3];

    stat = avr_jtag_instr (AVR_OCD, delay);
    if (stat == JTAG_OK) {
        tdi [0] = buf [0];
        tdi [1] = buf [1];
        tdi [2] = reg | AVR_WR_OCDR;
        stat    = jtag_write_and_read (21,tdo, tdi);
    };
    return stat;
}

/*----------------------------------------------------------------------*
 * read from on chip debugging registers                                *
 * prepare to read the register : in the second run you can get the data*
 * acts just like a shift register : when you must read you must clock  *
 * in redundant data in order to read the shift register                *
 *----------------------------------------------------------------------*/
static unsigned char
rd_dbg_ocd (unsigned char reg, unsigned char *buf_out, unsigned char delay)
{
    unsigned char stat, tdo [3], tdi [3];

    stat = avr_jtag_instr (AVR_OCD, delay);
    if (stat == JTAG_OK) {
        tdi [0] = reg;
        stat    = jtag_write_and_read (5,&tdo [0], &tdi [0]);
    }
    if (stat == JTAG_OK) {
        tdi [0]     = 0;
        tdi [1]     = 0;
        tdi [2]     = reg;
        stat        = jtag_write_and_read (21,tdo, tdi);
        buf_out [0] = tdo [0];
        buf_out [1] = tdo [1];
    }
    return stat;
}

/*----------------------------------------------------------------------*
 *----------------------------------------------------------------------*/
static unsigned char
rd_dbg_channel (unsigned char *buf_out, unsigned char delay)
{
    unsigned char stat, tdo [2], tdi [2];

    stat = avr_jtag_instr (AVR_OCD, delay);
    if (stat == JTAG_OK) {
        tdi [0] = AVR_DBG_COMM_DATA;
        stat    = jtag_write_and_read (5,&tdo [0], &tdi [0]);
    }
    if (stat == JTAG_OK) {
        tdi [0]     = 0;
        tdi [1]     = 0;
        stat        = jtag_write_and_read (16,tdo, tdi);
        buf_out [0] = tdo [0];
        buf_out [1] = tdo [1];
    }
    return stat;
}

/*----------------------------------------------------------------------*
 *----------------------------------------------------------------------*/
unsigned char
force_avr_stop (unsigned char delay)
{
    unsigned char stat;

    stat = avr_jtag_instr (AVR_FORCE_BRK, delay);
    return stat;
}

/*----------------------------------------------------------------------*
 *----------------------------------------------------------------------*/
unsigned char
init_avr_jtag (struct avr_reg *reg, unsigned char delay)
{
    unsigned char stat, buf_in [4], buf_out [4], *p, *q;
    char          cnt;

    stat = jtag_reset();
    if (stat == JTAG_OK) stat = avr_reset (0);
    if (stat == JTAG_OK) stat = rd_dbg_ocd (AVR_DBG_COMM_CTL, buf_out, delay);
    if (stat == JTAG_OK) stat = avr_reset(0);
    if (stat == JTAG_OK) stat = avr_jtag_instr (AVR_FORCE_BRK, delay);
    if (stat == JTAG_OK) stat = rd_dbg_ocd (AVR_DBG_COMM_CTL, buf_out, delay);
    if (stat == JTAG_OK) stat = avr_reset (1);

    /*
     * set break point at reset vector : 
     * write (= 1 bit) breakpoint register PSB0 (4 bits) : address 0 (= 16
     * bits).  makes total 21 bits -> 3 bytes
     */
    buf_in [0] = 0;
    buf_in [1] = 0;
    if (stat == JTAG_OK) wr_dbg_ocd (AVR_PSB0, buf_in, delay);
    /*
     * prepare to read control and status register
     */
    if (stat == JTAG_OK) stat = rd_dbg_ocd (AVR_BCR, buf_out, delay);
    /*
     * write control and status register with PSB1 activated
     */
    buf_in [0] = 0;
    buf_in [1] = AVR_EN_PSB1;
    if (stat == JTAG_OK) wr_dbg_ocd (AVR_BCR, buf_in, delay);

    /*
     * and let the target run
     */
    if (stat == JTAG_OK) stat = avr_reset (0);
    if (stat == JTAG_OK) stat = rd_dbg_ocd (AVR_DBG_COMM_CTL, buf_out, delay);

    /*
     * prepare to read control and status register again
     */
    if (stat == JTAG_OK) stat = rd_dbg_ocd (AVR_BCR, buf_out, delay);

    /*
     * write control and status register with PSB0 deactivated
     * prepare to read the pc 
     */
    buf_in [0] = 0;
    buf_in [1] = 0;
    if (stat == JTAG_OK) wr_dbg_ocd (AVR_BCR, buf_in, delay);

    /*
     * prepare to read the pc
     */
    if (stat == JTAG_OK) exec_instr_avr (buf_out, buf_in, CHK_PC, delay);
    if (stat == JTAG_OK) stat = rd_dbg_ocd (AVR_DBG_COMM_CTL, buf_out, delay);

    buf_in [0] = buf_out [0];
    buf_in [1] = buf_out [1] | AVR_EN_OCDR;
    if (stat == JTAG_OK) wr_dbg_ocd (AVR_DBG_COMM_CTL, buf_in, delay);

    if (stat == JTAG_OK) {  
        buf_in [0] = 0xE1;
        buf_in [1] = 0xBF;
        buf_in [2] = 0xF1;
        buf_in [3] = 0xBF;
        q          = (unsigned char *) &(reg->pc);
        p          = buf_in;
        cnt        = 1;
        do {
            stat = exec_instr_avr (buf_out, p, 0, delay);
            p += 2;
            if (stat == JTAG_OK) stat = rd_dbg_channel (buf_out, delay);
            if (stat == JTAG_OK) *q++ = buf_out [1];
        } while ((!(--cnt & 0x80)) && (stat == JTAG_OK));
    }

    if (stat == JTAG_OK) stat = rd_dbg_ocd (AVR_DBG_COMM_CTL, buf_out, delay);
    buf_in [0] = BIT3 | BIT2;
    buf_in [1] = AVR_EN_OCDR;
    if (stat == JTAG_OK) stat = wr_dbg_ocd (AVR_DBG_COMM_CTL, buf_in, delay);

    return stat;
}

/*----------------------------------------------------------------------*
 * this one really gets the register values right after a break, stop,  *
 * or initial condition                                                 *
 *----------------------------------------------------------------------*/
#if 0
static void
get_avr_regs (unsigned char delay)
{
    unsigned char buf_in [2], buf_out [2], stat, *p, *q, xsum, byte_buf [2];
    char          cnt;
  
#ifdef AVR
    unsigned char code_buf [64];
#endif

    stat = rd_dbg_ocd (AVR_DBG_COMM_CTL, buf_out, delay);
    if (stat == JTAG_OK) {
        buf_in [0] = buf_out [0] | BIT4 | BIT3 | BIT2;
        buf_in [1] = buf_out [1] | AVR_EN_OCDR;
        stat = wr_dbg_ocd (AVR_DBG_COMM_CTL, buf_in, delay);
    }
    if (stat == JTAG_OK) {
        /*
         * read register set
         */
#ifdef AVR
        memcpy_P (code_buf, avr_code_get_reg, 64);
        p = code_buf;
#else
        p = (unsigned char *) &avr_code_get_reg;
#endif
        q = (unsigned char *) &(reg.avr);
        cnt = 31;
        do {
            stat = exec_instr_avr (buf_out, p, 0, delay);
            p += 2;
            if (stat == JTAG_OK) stat = rd_dbg_channel (buf_out, delay);
            if (stat == JTAG_OK) *q++ = buf_out [1];
        } while (!((--cnt & 0x80)) && (stat == JTAG_OK));
    }
    p    = (unsigned char *) &(reg.avr);
    cnt  = 0;
    xsum = 0;
    outch ('$');
    while (cnt < (sizeof (struct avr_reg))) {
        byte2ascii (byte_buf, *p);
        xsum += byte_buf [0] + byte_buf [1];
        outbyte (*p++);
        cnt++;
    }
    outch ('#');
    outbyte (xsum);

}
#endif

/*----------------------------------------------------------------------*
 * enable on chip debugging                                             * 
 *----------------------------------------------------------------------*/
static unsigned char
activate_ocd (unsigned char delay)
{
    unsigned char stat, buf_in [2];
 
    do {
        jtag_reset ();
        stat = avr_jtag_instr (AVR_FORCE_BRK, delay);
        buf_in [0] = BIT2;
        buf_in [1] = AVR_EN_OCDR;
        if (stat == JTAG_OK)
            stat = wr_dbg_ocd (AVR_DBG_COMM_CTL, buf_in, delay);
        if (stat == JTAG_OK)
            stat = rd_dbg_ocd (AVR_DBG_COMM_CTL, buf_in, delay);
    } while (((buf_in [0] & (BIT2 | BIT3)) != (BIT2 | BIT3))
             && (stat == JTAG_OK));
    return stat;
}

/*----------------------------------------------------------------------*
 * reading flash bytes :                                                *
 *                                                                      *
 * 0xe<lo_adr_hi_nib><e><lo_adr_lo_nib> ldi     r30,lo8(addr)           *
 * 0xe<hi_adr_hi_nib><f><hi_adr_lo_nib> ldi     r31,hi8(addr)           *
 * 0x95c8                               lpm                             *
 * 0xbe01                               out     0x31,r0                 *
 *                                      read ocdr                       *
 * 0x9631                               adiw    r30,1                   *
 *                                                                      *
 * low byte is first byte in jtag chain                                 *
 *----------------------------------------------------------------------*/
unsigned char
rd_flash_ocd_avr (unsigned long addr, unsigned char *buf, short size,
                  unsigned char delay)
{
    unsigned char stat, *p, buf_in [2], buf_out [2];
    short         num;
  
    num  = 0;
    stat = activate_ocd (delay);
    p    = (unsigned char *) &addr;
    p   += 2;
    /*
     * ldi      r31,hi8(addr)
     */
    if (stat == JTAG_OK) {
        buf_in [0] = (*p & 0xF) + 0xF0;
        buf_in [1] = ((*p & 0xF0) >> 4) + 0xE0;
        stat       = exec_instr_avr (buf_out, buf_in, 0, delay);
    }
    /*
     * ldi      r30,lo8(addr)
     */
    if (stat == JTAG_OK) {
        buf_in [0] = (*++p & 0xF) + 0xE0;
        buf_in [1] = ((*p & 0xF0) >> 4) + 0xE0;
        stat       = exec_instr_avr (buf_out, buf_in, 0, delay);
    }
    while ((stat == JTAG_OK) && (num < size)) {
        /*
         * lpm  r0,z+
         */
        buf_in [0] = 5;
        buf_in [1] = 0x90;
        stat       = exec_instr_avr (buf_out, buf_in, 0, delay);
        /*
         * out  ocdr,r0
         */
        if (stat == JTAG_OK) {
            buf_in [0] = 1;
            buf_in [1] = 0xBE;
            stat       = exec_instr_avr (buf_out, buf_in, 0, delay);
        }
        if (stat == JTAG_OK) stat   = rd_dbg_channel (buf_out, delay);
        if (stat == JTAG_OK) *buf++ = buf_out [1];
        num++;
    }

    return stat;
}

/*----------------------------------------------------------------------*
 * read data space bytes                                                *
 *                                                                      *
 * 0xe<lo_adr_hi_nib><e><lo_adr_lo_nib> ldi     r30,lo8(addr)           *
 * 0xe<hi_adr_hi_nib><f><hi_adr_lo_nib> ldi     r31,hi8(addr)           *
 * 0x9000                               ld      r0,z                    *
 * 0xbe01                               out     0x31,r0                 *
 *                                      read ocdr                       *
 *----------------------------------------------------------------------*/
unsigned char
rd_sram_ocd_avr (unsigned short addr, unsigned char *buf, short size,
                 unsigned char delay)
{
    unsigned char stat, buf_in [4], buf_out [4], *p;
    short         num;

    num  = 0;
    stat = activate_ocd (delay);
    p    = (unsigned char *) &addr;
    p   += 2;
    /*
     * ldi      r31,hi8(addr)
     */
    if (stat == JTAG_OK) {
        buf_in [0] = (unsigned char) (*p & 0xF) + 0xF0;
        buf_in [1] = ((*p & 0xF0) >> 4) + 0xE0;
        stat       = exec_instr_avr (buf_out, buf_in, 0, delay);
    }
    /*
     * ldi      r30,lo8(addr)
     */
    if (stat == JTAG_OK) {
        buf_in [0] = (*++p & 0xF) + 0xE0;
        buf_in [1] = ((*p & 0xF0) >> 4) + 0xE0;
        stat       = exec_instr_avr (buf_out, buf_in, 0, delay);
    }
    while ((stat == JTAG_OK) && (num < size)) {
        /*
         * ld   r0,z+
         */
        if (stat == JTAG_OK) {
            buf_in [0] = 1; 
            buf_in [1] = 0x90;
            stat       = exec_instr_avr (buf_out, buf_in, 0, delay);
        }
        /*
         * out  ocdr,r0
         */
        if (stat == JTAG_OK) {
            buf_in [0] = 1;
            buf_in [1] = 0xBE;
            stat       = exec_instr_avr (buf_out, buf_in, 0, delay);
        }
        if (stat == JTAG_OK) stat   = rd_dbg_channel (buf_out, delay);
        if (stat == JTAG_OK) *buf++ = buf_out [1];
        num++;
    }

    return stat;
}

/*----------------------------------------------------------------------*
 *                                                                      *
 * 0xb<(addr & 0x30) >> 3)>0<addr & 0xF)        in      r0,<addr>       *
 * 0xbe01                                       out     0x31,r0         *
 *                                      read ocdr                       *
 *----------------------------------------------------------------------*/
unsigned char
rd_io_ocd_avr (unsigned char addr, unsigned char *buf, short size,
               unsigned char delay)
{
    unsigned char stat, buf_in [2], buf_out [2];
    short         num;

    num  = 0;
    stat = activate_ocd (delay);
    while ((stat == JTAG_OK) && (num < size)) {
        buf_in [0] = addr & 0xF;
        buf_in [1] = ((addr & 0x30) >> 3) + 0xB0;
        stat = exec_instr_avr (buf_out, buf_in, 0, delay);

        if (stat == JTAG_OK) {
            buf_in [0] = 1;
            buf_in [1] = 0xBE;
            stat = exec_instr_avr (buf_out, buf_in, 0, delay);
        }
        if (stat == JTAG_OK) stat = rd_dbg_channel (buf_out, delay);
        if (stat == JTAG_OK) *buf++ = buf_out [1];
        addr++;
        num++;
    }

    return stat;
}

/*----------------------------------------------------------------------*
 *                                                                      * 
 * 0xe<lo_adr_hi_nib><e><lo_adr_lo_nib> ldi     r30,lo8(addr)           *
 * 0xe<hi_adr_hi_nib><f><hi_adr_lo_nib> ldi     r31,hi8(addr)           *
 * 0xbbff                               out     eearh,r31               *
 * 0xbbee                               out     eearl,r30               *
 * 0x9ae1                               sbi     eecr,1                  *
 * 0xb20d                               in      r0,eedr                 *
 * 0xbe01                               out     0x31,r0                 *
 *                                      read ocdr                       *
 * 0x9631                               adiw    r30,1                   *
 *----------------------------------------------------------------------*/
unsigned char
rd_e2_ocd_avr (unsigned short addr, unsigned char *buf, short size,
               unsigned char delay)
{
    unsigned char stat, buf_in [2], buf_out [2], *p;
    short         num;

    num  = 0;
    stat = activate_ocd (delay);
    p    = (unsigned char *) &addr;
    /*
     * ldi      r31,hi8(addr)
     */
    if (stat == JTAG_OK) {
        buf_in [0] = (unsigned char) (*p & 0xF) + 0xF0;
        buf_in [1] = ((*p & 0xF0) >> 4) + 0xE0;
        stat       = exec_instr_avr (buf_out, buf_in, 0, delay);
    }
    /*
     * ldi      r30,lo8(addr)
     */
    if (stat == JTAG_OK) {
        buf_in [0] = (*++p & 0xF) + 0xE0;
        buf_in [1] = ((*p & 0xF0) >> 4) + 0xE0;
        stat       = exec_instr_avr (buf_out, buf_in, 0, delay);
    }
    while ((stat == JTAG_OK) && (num < size)) {
        /*
         * out  eearh,r31
         */
        if (stat == JTAG_OK) {
            buf_in [0] = 0xFF;
            buf_in [1] = 0xBB;
            stat = exec_instr_avr (buf_out, buf_in, 0, delay);
        }
        /*
         * out  eearl,r30
         */
        if (stat == JTAG_OK) {
            buf_in [0] = 0xEE;
            buf_in [1] = 0xBB;
            stat = exec_instr_avr (buf_out, buf_in, 0, delay);
        }
        /*
         * sbi  eecr,1
         */
        if (stat == JTAG_OK) {
            buf_in [0] = 0xE1;
            buf_in [1] = 0x9A;
            stat = exec_instr_avr (buf_out, buf_in, 0, delay);
        }
        /*
         * in   r0,eedr
         */
        if (stat == JTAG_OK) {
            buf_in [0] = 0xD;
            buf_in [1] = 0xB2;
            stat = exec_instr_avr (buf_out, buf_in, 0, delay);
        }
        /*
         * out  ocdr,r0
         */
        if (stat == JTAG_OK) {
            buf_in [0] = 1;
            buf_in [1] = 0xBE;
            stat       = exec_instr_avr (buf_out, buf_in, 0, delay);
        }
        if (stat == JTAG_OK) stat = rd_dbg_channel (buf_out, delay);
        if (stat == JTAG_OK) *buf++ = buf_out [1];
        /*
         * adiw r30,1
         */
        if (stat == JTAG_OK) {
            buf_in [0] = 0x31;
            buf_in [1] = 0x96;
            stat       = exec_instr_avr (buf_out, buf_in, 0, delay);
        }
        addr++;
        num++;
    }

    return stat;
}

/*----------------------------------------------------------------------*
 * to be checked AMtD 14-06-2004                                        *
 *----------------------------------------------------------------------*/
unsigned char
wr_avr_flash_ocd (unsigned long addr, unsigned char *buf, short size,
                  unsigned char delay)
{
    unsigned char stat;
    short         num;

    num  = 0;
    stat = activate_ocd (delay);
    while ((stat == JTAG_OK) && (num < size)) {
        addr++;
        num++;
    }

    return stat;
}

/*----------------------------------------------------------------------*
 * to be checked AMtD 14-06-2004                                        *
 *----------------------------------------------------------------------*/
unsigned char
wr_sram_ocd_avr (unsigned short addr, unsigned char *buf, short size,
                 unsigned char delay)
{
    unsigned char stat;
    short         num;

    num  = 0;
    stat = activate_ocd (delay);
    while ((stat == JTAG_OK) && (num < size)) {
        addr++;
        num++;
    }
    return stat;
}

/*----------------------------------------------------------------------*
 * to be checked AMtD 14-06-2004                                        *
 *----------------------------------------------------------------------*/
unsigned char
wr_io_ocd_avr (unsigned char addr, unsigned char *buf, short size,
               unsigned char delay)
{
    unsigned char stat;
    short         num;

    num  = 0;
    stat = activate_ocd (delay);
    while ((stat == JTAG_OK) && (num < size)) {
        addr++;
        num++;
    }

    return stat;
}

/*----------------------------------------------------------------------*
 * mode : 1 (run) or 0 (stop). Only one bit is necessary                *
 *----------------------------------------------------------------------*/
unsigned char
run_avr (unsigned char mode, unsigned char go_flg, unsigned long addr,
         unsigned char delay)
{
    unsigned char stat, tdo, tdi;

    stat = avr_jtag_instr (AVR_FORCE_BRK, delay);
    tdi  = mode;
    stat = jtag_write_and_read (1,&tdo, &tdi);
    return stat;
}

/*----------------------------------------------------------------------*
 * to be checked AMtD 14-06-2004                                        *
 *----------------------------------------------------------------------*/
void
step_avr (unsigned char delay)
{
    unsigned char stat;

    stat = jtag_reset (); 
    if (stat == JTAG_OK) stat = activate_ocd (delay);
}

/*----------------------------------------------------------------------*
 *----------------------------------------------------------------------*/
void
init_all_regs_avr (void)
{
    unsigned char *p;
    char          i;

    p = (unsigned char *) &(reg.avr);
    i = sizeof (struct avr_reg) - 1;
    do {
        *p++ = 0;
    } while (!(--i & 0x80));
}

/*----------------------------------------------------------------------*
 *----------------------------------------------------------------------*/
void
get_all_regs_avr (unsigned char delay)
{
	#if 0
    unsigned char *p, xsum, byte_buf [2];
    char          i; 

    xsum = 0;
    i    = sizeof (struct avr_reg) - 1;
    p    = (unsigned char *) &(reg.avr);
    outch ('$');
    do {
        byte2ascii (byte_buf, *p);
        xsum += byte_buf [0] + byte_buf [1];
        outbyte (*p++);
    } while (!(--i & 0x80));
    outch ('#');
    outbyte (xsum);
	#endif
}


/*----------------------------------------------------------------------*
 *----------------------------------------------------------------------*/
void
set_all_regs_avr (unsigned char *buf, unsigned char delay)
{
	#if 0
    unsigned char *p, ok, val;
    char          i; 

    i  = 0;
    p  = (unsigned char *) &(reg.avr);
    ok = 1;
    while (ok && (i < (sizeof (struct avr_reg)))) {
        ok = in_byte (buf, &val);
        if (ok) {
            buf += 2;
            *p++ = val;
        }
        i++;
    }
    //if (ok) pstring (ok_str);
    //else pr_stat ('E', 19);
    //else pr_stat ('E', NO_NUMBER);
	#endif
}
