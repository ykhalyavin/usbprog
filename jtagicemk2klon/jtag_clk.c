/*-------------------------------------------------------------------------
 * JTAG_CLK.C
 * Copyright (C) 2003 Armand ten Doesschate <a.doesschate@hccnet.nl>
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
 
 ************************************************************************
 ************************************************************************
 ************************************************************************
 *
 * Note :
 *
 * The real hardware driving jtag port is done through clk_jtag. This
 * function is called with the software jtag port and is microcontroller
 * dependent.
 * The jtag port is checked on its powersupply. If there is no power
 * supply then return an error.
 *
 ************************************************************************
 ************************************************************************
 ************************************************************************/

#include "jtag.h"
#include "jtag_clk.h"
#include "constant.h"
#include "uc_call.h"
#include "var.h"
#include "lib.h"

/*----------------------------------------------------------------------*
 *----------------------------------------------------------------------*/
unsigned char
reset_jtag (char delay)
{
    char          res_cnt;
    unsigned char stat;

    stat = JTAG_OK;
    if (((tgt_pwr - MIN_SUPPLY) & 0x80) == 0x80) stat = PWR_SUPPLY_ERR;
    else {
        /*
         * initial startup is in test logic reset state
         */
        clk_jtag (0, delay);
        res_cnt = RESET_CNT;
        do {
            clk_jtag (TMS, delay);
       } while (!(--res_cnt & 0x80));
   }
   clk_jtag (0, delay);
   return stat;
}

/*----------------------------------------------------------------------*
 *----------------------------------------------------------------------*/
static void
goto_shift_dr (unsigned char delay)
{
    clk_jtag (TMS, delay);
    clk_jtag (0, delay);
    clk_jtag (0, delay);
}

/*----------------------------------------------------------------------*
 *----------------------------------------------------------------------*/
static void
goto_shift_ir (unsigned char delay)
{
    clk_jtag (TMS, delay);
    goto_shift_dr (delay);
}

/*----------------------------------------------------------------------*
 * The jtag instructions are 4 bits in length. Before the last bit is   *
 * to be shifted the TMS pin must be activated in order to exit the     *
 * (ir) shift state                                                     *
 *----------------------------------------------------------------------*/
unsigned char
feed_jtag_instr (unsigned char jtag_instr, unsigned char delay)
{
   char          num_bits;
   unsigned char jtag_prt, ret_val;

   ret_val = JTAG_OK;

   if ((tgt_pwr - MIN_SUPPLY) & 0x80) ret_val = PWR_SUPPLY_ERR;
   else {
       num_bits = 3;
       jtag_prt = 0;

       goto_shift_ir (delay);
       do {
           /*
            * go to exit state if end reached
            */
           if (num_bits == 0)  jtag_prt |= TMS;
           jtag_prt &= ~TDI;
           if (jtag_instr & 1) jtag_prt |= TDI;

           clk_jtag (jtag_prt, delay);
           jtag_instr >>= 1;
       } while (!(--num_bits & 0x80));
       /*
        * go to update state
        */
       clk_jtag (TMS, delay);
    }

    return ret_val;
}

/*----------------------------------------------------------------------*
 *                                                                      *
 * The state is only necessary in the ARM7 and can be set to CHK_BRK.   *
 *                                                                      *
 * The total bits to be shifted in depends how many data the jtag       *
 * interface must contain. The default here is defined as total bits in *
 * 1 byte.                                                              *
 *----------------------------------------------------------------------*/
unsigned char
feed_jtag_data (unsigned char *tdo, unsigned char *tdi, short num_bits,
                unsigned char state, unsigned char delay)
{
    unsigned char ret_val, jtag_prt, rd_msk, stat;

    stat = JTAG_OK;
    if ((tgt_pwr - MIN_SUPPLY) & 0x80) stat = PWR_SUPPLY_ERR; 
    else {
        /*
         * reset TMS, TDO and TDI
         */
        ret_val = jtag_prt = 0;
        goto_shift_dr (delay);

        /*
         * default TMS = 0 so that state willnot be changed
         */
        jtag_prt = 0;

        /*
         * The state of the ARM7 : if system speed / breakpoint bit is set
         * then check the return value of TDO. When the ARM7 is in debug
         * mode (not running an application) then the ARM7 is always in
         * 32 bits condition even if the ARM7 is running the application
         * in thumb mode.
         */
        if (state & CHK_BRK) {
            if (tdi [4] & 1) jtag_prt |= TDI;
            ret_val = clk_jtag (jtag_prt, delay);
            tdo [4] = (ret_val & TDO);
            num_bits--;
        }

        /*
         * The jtag expects the lsb to be shifted in first and out. So 
         * a bit mask is following from low to high when the TDO pin is
         * sampled and the sample is put into the tdo array 
         */

        rd_msk = 1;
        *tdo   = 0;
        while (num_bits > 0) {
            /*
             * if the last bit is clocked in then also set TMS high in order
             * to exit the (dr) state into the exit state.
             */
            if (num_bits == 1) jtag_prt |= TMS;

            jtag_prt &= ~TDI;
            if (*tdi & 1) jtag_prt |= TDI;

            ret_val = clk_jtag (jtag_prt, delay);

            /*
             * rd_msk goes along with the bits to be shifted out
             * point to the next *bit* of tdi to be shifted in 
             */
            if (ret_val & TDO) *tdo |= rd_msk;
            rd_msk <<= 1;
            *tdi   >>= 1;
            num_bits--;
            /*
             * The AVR can only hold an unsigned char to shift in and
             * and out. Point the next byte in the tdo array to shift out
             * and initialise it to "0"
             * point also to the next byte in the tdi array
             */
            if (rd_msk == 0) {
                tdi++;
                if (num_bits > 0) *++tdo = 0; 
                rd_msk = 1;
            }
        } 
        /*
         * When ready out of the exit stage then go the update stage
         */
        clk_jtag (TMS, delay);
    }
    /*
     * go to idle / test state, generate internal clock sequence
     * if neccesary
     */
    if (state & IDLE) clk_jtag (0, delay);

    return stat;
}

/*----------------------------------------------------------------------*
 * after fetching the id code the jtag interface must be set into ready *
 * state                                                                *
 *----------------------------------------------------------------------*/
unsigned char
get_jtag_idcode (unsigned char instr_id, unsigned long *jtag_id,
                 unsigned char delay)
{
    unsigned long dont_care, id_code;
    unsigned char stat;

    *jtag_id = id_code = dont_care = 0;

    stat = reset_jtag (delay);
    if (stat == JTAG_OK) stat = feed_jtag_instr (instr_id, delay);
    if (stat == JTAG_OK) stat = feed_jtag_data ((unsigned char *)&id_code,
                                                (unsigned char *)&dont_care,
                                                32, IDLE, delay);
    if (stat == JTAG_OK) *jtag_id = id_code;

    return stat;
}

/*----------------------------------------------------------------------*
 *----------------------------------------------------------------------*/
void
bypass (unsigned char delay)
{
    unsigned char dont_care1, dont_care2;

    feed_jtag_instr (BYPASS, delay);
    feed_jtag_data (&dont_care1, &dont_care2, 1, IDLE, delay);
}

/*----------------------------------------------------------------------*
 * This one is used in the ARM7 to select the debug chain or the ice    *
 * breaker chain. The instruction register must be told to receive the  *
 * scan instruction and then the channel to be selected is going into   *
 * data register.                                                       *
 * When the scan register is set then the jtag interface is not put     *
 * into idle state                                                      *
 *----------------------------------------------------------------------*/
static unsigned char
select_scan_chain (unsigned char scan_nr, unsigned char delay)
{
    unsigned char dont_care, stat;

    stat = dont_care = 0;

    stat = feed_jtag_instr (ARM_SCAN_N, delay);
    if (stat == JTAG_OK) feed_jtag_data (&dont_care, &scan_nr, 4, 0, delay);
    return stat;
}


