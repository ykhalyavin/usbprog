/*-------------------------------------------------------------------------
 * JTAG_AVR_OCD.C
 * Copyright (C) 2003 Armand ten Doesschate <a.doesschate@hccnet.nl>
 * Copyright (C) 2007 Benedikt Sauter <sauter@ixbat.de>
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

#include "jtag.h"
#include "jtag_avr.h"
#include "jtag_avr_defines.h"
#include "jtag_avr_ocd.h"
#include "avr_asm.h"
#include "uart.h"

static const uint8_t OCDR_Addr = 0x31;
static const uint8_t SPMCR_Addr = 0x37;
static const uint8_t EECR_Addr = 0x1C;
static const uint8_t SREG_Addr = 0x3F;


/* This struct holds essential chip data for debugging.
 * Please use the according accessor functions to modify it
 */
struct AVR_Context_Type avrContext;

unsigned char ocd_enshure_ocdr_enable() {
	uint16_t csr;
	rd_dbg_ocd(AVR_DBG_COMM_CTL,&csr,0);

	if (!(csr & AVR_EN_OCDR)) {
		csr |= AVR_EN_OCDR;
		wr_dbg_ocd(AVR_DBG_COMM_CTL,&csr,0);
	}
}

uint8_t ocd_read_ocdr() {
	unsigned char tdo[2];

	rd_dbg_ocd(AVR_DBG_COMM_DATA,tdo,0);

	return tdo[1];
}

/* Saves the avr registers which get used by ocd system */
unsigned char ocd_save_context() {
	ocd_enshure_ocdr_enable();

	avrContext.PC = ocd_read_pc();
	// save register 16
	ocd_execute_avr_instruction(AVR_OUT(OCDR_Addr,16)); // out 0x31, r16
	avrContext.r16 = ocd_read_ocdr();

	ocd_execute_avr_instruction(AVR_IN(16,SREG_Addr));
	ocd_execute_avr_instruction(AVR_OUT(OCDR_Addr,16));
	avrContext.STATUS = ocd_read_ocdr();

	ocd_execute_avr_instruction(AVR_OUT(OCDR_Addr,30)); // out 0x31, r30
	avrContext.r30 = ocd_read_ocdr();

	ocd_execute_avr_instruction(AVR_OUT(OCDR_Addr,31)); // out 0x31, r33
	avrContext.r31 = ocd_read_ocdr();

	avrContext.registerDirty = 1; // reading the above register dirties the context
#ifdef DEBUG_VERBOSE
	UARTWrite("OCDSave\r\nr0:");
	SendHex(avrContext.r16);
	UARTWrite("\r\nr30:");
	SendHex(avrContext.r30);
	UARTWrite("\r\nr31:");
	SendHex(avrContext.r31);
	UARTWrite("\r\n");
#endif
	return 1;
}

#ifdef DEBUG_ON
void ocd_dump_debug_registers() {
	UARTWrite("r16:");
	ocd_execute_avr_instruction(AVR_OUT(OCDR_Addr,16)); // out 0x31, r16
	SendHex(ocd_read_ocdr());

	UARTWrite("\r\nr30:");
	ocd_execute_avr_instruction(AVR_OUT(OCDR_Addr,30)); // out 0x31, r16
	SendHex(ocd_read_ocdr());

	UARTWrite("\r\nr31:");
	ocd_execute_avr_instruction(AVR_OUT(OCDR_Addr,31)); // out 0x31, r16
	SendHex(ocd_read_ocdr());
	UARTWrite("\r\n");

}
#endif

/* Restores before saved registers after completing the debug modifications */
unsigned char ocd_restore_context() {
	if (avrContext.registerDirty == 1) {
		ocd_enshure_ocdr_enable();

		// restore SREG
		ocd_execute_avr_instruction(AVR_LDI(16,avrContext.STATUS));
		ocd_execute_avr_instruction(AVR_OUT(SREG_Addr,16));

		// restore PC
		avrContext.PC -= 3; // this is needed because of the 3 LDI Instructions after the IJMP
		ocd_execute_avr_instruction(AVR_LDI(30,avrContext.PC));
		ocd_execute_avr_instruction(AVR_LDI(31,avrContext.PC>>8));
		ocd_execute_avr_instruction(AVR_IJMP());
		jtag_clock_cycles(1);

		// restore registers
		ocd_execute_avr_instruction(AVR_LDI(16,avrContext.r16));

		ocd_execute_avr_instruction(AVR_LDI(30,avrContext.r30));

		ocd_execute_avr_instruction(AVR_LDI(31,avrContext.r31));
	#ifdef DEBUG_VERBOSE
		UARTWrite("Registers restored\r\n");
	#endif

		avrContext.registerDirty = 0;
	}
	// restore all BP data

#ifdef DEBUG_ON
	UARTWrite("PSB0:");
	SendHex((char)(avrContext.PSB0>>8));
	SendHex((char)avrContext.PSB0);
	UARTWrite("\r\nPSB1:");
	SendHex((char)(avrContext.PSB1>>8));
	SendHex((avrContext.PSB1));
	UARTWrite("\r\nPDMSB:");
	SendHex((char)(avrContext.PDMSB>>8));
	SendHex((avrContext.PDMSB));
	UARTWrite("\r\nPDSB:");
	SendHex((char)(avrContext.PDSB>>8));
	SendHex((char)avrContext.PDSB);
	UARTWrite("\r\n");
#endif

	wr_dbg_ocd(AVR_PSB0,&avrContext.PSB0,0);
	wr_dbg_ocd(AVR_PSB1,&avrContext.PSB1,0);
	wr_dbg_ocd(AVR_PDMSB,&avrContext.PDMSB,0);
	wr_dbg_ocd(AVR_PDSB,&avrContext.PDSB,0);

	wr_dbg_ocd(AVR_BCR,&avrContext.break_config,0);


	return 1;
}

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
unsigned char
exec_instr_avr (unsigned char *out, unsigned char *in, unsigned char flg,
				unsigned char delay) {
	unsigned char tdi [4], tdo [4], stat;

	//jtag_reset();
	avr_jtag_instr(AVR_INSTR, delay);
	if (flg == CHK_PC) {
			tdi [0] = 0;
			tdi [1] = 0;
			tdi [2] = 0xFF;
			tdi [3] = 0xFF;
			jtag_write_and_read (32,tdi, tdo);
			out [0] = tdo [0];
			out [1] = tdo [1];
			out [2] = tdo [2];
			out [3] = tdo [3];
		} else {
			/*
			* an avr instruction
			*/
			tdi [0] = in [0];
			tdi [1] = in [1];
			UARTWrite("AVR_Instr:");
			SendHex(tdi[1]);
			SendHex(tdi[0]);
			UARTWrite("\r\n");

			jtag_write_and_read(16,tdi,tdo);
			jtag_goto_state(RUN_TEST_IDLE);

			UARTWrite("Readback:");
			SendHex(tdo[1]);
			SendHex(tdo[0]);
			UARTWrite("\r\n");

		}
	return 1;
}

uint16_t ocd_read_pc() {
	avr_jtag_instr(AVR_INSTR,0);

	unsigned char tdi[4] = { 0, 0 , 0xFF, 0xFF };
	unsigned char tdo[4];

	jtag_write_and_read(32,tdi,tdo);
	jtag_goto_state(RUN_TEST_IDLE);

	uint16_t res = (tdo[1] << 8) | tdo[0];

	return res;
}

unsigned char ocd_execute_avr_instruction(uint16_t instr) {
	avr_jtag_instr(AVR_INSTR, 0);

	jtag_write(16,&instr);
	jtag_goto_state(RUN_TEST_IDLE); // some documentation said that its neccesary to go to IDLE state after executing the instr

}

// TODO: in the following 2 functions pay attention that the debugging registers are not in memory range!
uint8_t ocd_rd_sram(uint16_t startaddr, uint16_t len, uint8_t *buf) {
	avrContext.registerDirty = 1;
	ocd_enshure_ocdr_enable();


#ifdef DEBUG_VERBOSE
		ocd_dump_debug_registers();

		UARTWrite("SRR:");

		SendHex((char)(startaddr >> 8));
		SendHex((char)startaddr);
		UARTWrite(":");
		SendHex((char)(len >> 8));
		SendHex((char)len);
		UARTWrite("\r\n");
#endif

	// load starting address into Z register
	ocd_execute_avr_instruction(AVR_LDI(30,startaddr & 0xFF));
	ocd_execute_avr_instruction(AVR_LDI(31,startaddr >> 8));

#ifdef DEBUG_VERBOSE
	ocd_dump_debug_registers();

	UARTWrite("-----\r\n");
#endif

	// now read len bytes from avr
	while (len--) {
		uint8_t databuf;

		if (startaddr > 31 || (startaddr != 16 && startaddr != 30 && startaddr != 31)) {
			ocd_execute_avr_instruction(AVR_LDZ_PostInc(16)); // load value with post increment
			jtag_clock_cycles(1); // this may be needed because LDZ is 2 cycle instruction
			ocd_execute_avr_instruction(AVR_OUT(OCDR_Addr,16));

			databuf = ocd_read_ocdr();
		}
		else {
				if (startaddr == 16)
					databuf = avrContext.r16;
				else if (startaddr == 30)
					databuf = avrContext.r30;
				else if (startaddr == 31)
					databuf = avrContext.r31;

				// signalize AVR that this address gets skipped
				ocd_execute_avr_instruction(AVR_ADIW(3, 1));
				jtag_clock_cycles(1); // the above is two cycle instruction
		}
		startaddr++;



#ifdef DEBUG_VERBOSE
		//ocd_dump_debug_registers();

		SendHex(databuf);
		UARTWrite("\r\n");
#endif

		*buf++ = databuf;
	}
#ifdef DEBUG_VERBOSE
		UARTWrite("\r\n");
		ocd_dump_debug_registers();
#endif
	return 1;
}

uint8_t ocd_wr_sram(uint16_t startaddr, uint16_t len, uint8_t *buf) {
	avrContext.registerDirty = 1;
	ocd_enshure_ocdr_enable();


#ifdef DEBUG_VERBOSE
		ocd_dump_debug_registers();

		UARTWrite("SWR:");

		SendHex((char)(startaddr >> 8));
		SendHex((char)startaddr);
		UARTWrite(":");
		SendHex((char)(len >> 8));
		SendHex((char)len);
		UARTWrite("\r\n");
#endif

	// load starting address into Z register
	ocd_execute_avr_instruction(AVR_LDI(30,startaddr & 0xFF));
	ocd_execute_avr_instruction(AVR_LDI(31,startaddr >> 8));

#ifdef DEBUG_VERBOSE
	ocd_dump_debug_registers();

	UARTWrite("-----\r\n");
#endif

	// now write len bytes from avr
	while (len--) {

		if (startaddr > 31 || (startaddr != 16 && startaddr != 30 && startaddr != 31)) {
			ocd_execute_avr_instruction(AVR_LDI(16,*buf));
			buf++; // DO NOT WRITE IT INTO THE MACRO BECAUSE THE PARAMETER GETS REFERENCED MORE THAN ONCE
			ocd_execute_avr_instruction(AVR_STZ_PostInc(16)); // store value with post increment
			jtag_clock_cycles(1); // this may be needed because STZ is 2 cycle instruction
		}
		else { // its important to save values which get written to the working registers
			if (startaddr == 16)
					avrContext.r16 = *buf++;
				else if (startaddr == 30)
					avrContext.r30 = *buf++;
				else if (startaddr == 31)
					avrContext.r31 = *buf++;

				// signalize AVR that this address gets skipped
				ocd_execute_avr_instruction(AVR_ADIW(3, 1));
				jtag_clock_cycles(1); // the above is two cycle instruction
		}


		startaddr++;
	}
#ifdef DEBUG_VERBOSE
		UARTWrite("\r\n");
		ocd_dump_debug_registers();
#endif

	return 1;
}

uint8_t ocd_rd_flash(uint16_t startaddr, uint16_t len, uint8_t *buf) {
	// initialize the ocd system
	avrContext.registerDirty = 1;
	//ocd_enshure_ocdr_enable();
	// adjust pc so that it does not collide with read access

	unsigned char obuf[4] = { 0, 0, 0, 0 };
	unsigned char ibuf[4];

	uint16_t curraddr = startaddr>>1; // this is word address of avr memory
	uint8_t skipbyte = (startaddr & 1);

	while (1) {
		avr_jtag_instr(AVR_INSTR,0);
		obuf[0] = curraddr & 0xFF;
		obuf[1] = (curraddr>>8) & 0xFF;

		// you have to latch first the address of the word
		// when recieving the second one you have the complete program word ready
		jtag_write(32,obuf);
		jtag_goto_state(SHIFT_DR);
		jtag_write_and_read(32,obuf,ibuf);

#ifdef DEBUG_VERBOSE
		UARTWrite("JTAG Data:");
		SendHex(ibuf[0]);
		UARTWrite(" ");
		SendHex(ibuf[1]);
		UARTWrite(" ");
		SendHex(ibuf[2]);
		UARTWrite(" ");
		SendHex(ibuf[3]);
		UARTWrite("\r\n");
#endif

		// now save used bytes
		if (!skipbyte) {
			*buf++ = ibuf[3]; // please notice that default byte order is little endian
		}
		else {
			skipbyte = 0;
		}

		--len;

		if (!len)
			break;

		*buf++ = ibuf[2];

		--len;

		if (!len)
			break;

		++curraddr; // increase word address

	}

	return 1;
}

uint8_t ocd_wr_flash(uint16_t startaddr, uint16_t len, uint8_t *buf) {


}

/* Ja die Funktion sieht sehr komisch aus - das seh ich ein, aber es geht! */
uint8_t ocd_rd_eeprom(uint16_t startaddr, uint16_t len, uint8_t *buf) {
		// initialize the ocd system
	avrContext.registerDirty = 1;
	ocd_enshure_ocdr_enable();

	// load the start adress to the Z register
	ocd_execute_avr_instruction(AVR_LDI(30,startaddr & 0xFF));
	ocd_execute_avr_instruction(AVR_LDI(31,startaddr >> 8));


	// then clock out bytewise the data from program memory
	while (len--) {
		uint8_t databuf;

		// write the adress to the eeprom adressing registers
		ocd_execute_avr_instruction(AVR_OUT(EECR_Addr+2,30));
		ocd_execute_avr_instruction(AVR_OUT(EECR_Addr+3,31));
		ocd_execute_avr_instruction(AVR_IN(16,EECR_Addr));
		ocd_execute_avr_instruction(AVR_ORI(16,1));
		ocd_execute_avr_instruction(AVR_OUT(EECR_Addr,16));
		ocd_execute_avr_instruction(AVR_NOP());
		ocd_execute_avr_instruction(AVR_NOP());
		ocd_execute_avr_instruction(AVR_NOP());
		ocd_execute_avr_instruction(AVR_NOP());
		ocd_execute_avr_instruction(AVR_NOP());
		ocd_execute_avr_instruction(AVR_NOP()); // normally the cpu gets halted for 5 clock cycles when reading the eeprom. it seems that we should occupy this also

		//jtag_clock_cycles(7);
		// read from eeprom data register
		ocd_execute_avr_instruction(AVR_IN(16,EECR_Addr+1));
		ocd_execute_avr_instruction(AVR_OUT(OCDR_Addr,16));

		ocd_execute_avr_instruction(AVR_ADIW(3, 1));
		ocd_execute_avr_instruction(AVR_NOP()); // the above is two cycle instruction

		databuf = ocd_read_ocdr();

#ifdef DEBUG_ON
		SendHex(databuf);
		UARTWrite("\r\n");
		ocd_dump_debug_registers();
#endif
		*buf++ = databuf;
	}

#ifdef DEBUG_VERBOSE
	UARTWrite("\r\n");
#endif

	return 1;
}

/* Ja die Funktion sieht sehr komisch aus - das seh ich ein, aber es geht! */
uint8_t ocd_wr_eeprom(uint16_t startaddr, uint16_t len, uint8_t *buf) {
			// initialize the ocd system
	avrContext.registerDirty = 1;
	ocd_enshure_ocdr_enable();

	// load the start adress to the Z register
	ocd_execute_avr_instruction(AVR_LDI(30,startaddr & 0xFF));
	ocd_execute_avr_instruction(AVR_LDI(31,startaddr >> 8));


	// then clock out bytewise the data from program memory
	while (len--) {

		// write the adress to the eeprom adressing registers
		ocd_execute_avr_instruction(AVR_OUT(EECR_Addr+2,30));
		ocd_execute_avr_instruction(AVR_OUT(EECR_Addr+3,31));
		// write data to eeprom data register
		ocd_execute_avr_instruction(AVR_LDI(16,*buf));
		buf++;
		ocd_execute_avr_instruction(AVR_OUT(EECR_Addr+1,16));

		ocd_execute_avr_instruction(AVR_IN(16,EECR_Addr));
		ocd_execute_avr_instruction(AVR_ORI(16,0x04)); // set EEMWE
		ocd_execute_avr_instruction(AVR_OUT(EECR_Addr,16));
		ocd_execute_avr_instruction(AVR_ORI(16,0x02)); // set EEWE
		ocd_execute_avr_instruction(AVR_OUT(EECR_Addr,16));
		ocd_execute_avr_instruction(AVR_NOP());
		ocd_execute_avr_instruction(AVR_NOP());// normally the cpu gets halted for 2 clock cycles when reading the eeprom. it seems that we should occupy this also

		// wait for EEWE to become zero
		do {
				ocd_execute_avr_instruction(AVR_IN(16,EECR_Addr));
				ocd_execute_avr_instruction(AVR_OUT(OCDR_Addr,16));
		} while (ocd_read_ocdr() & 0x2); // as long as EEWE is one

		// read from eeprom data register
		ocd_execute_avr_instruction(AVR_IN(16,EECR_Addr+1));
		ocd_execute_avr_instruction(AVR_OUT(OCDR_Addr,16));

		ocd_execute_avr_instruction(AVR_ADIW(3, 1));
		ocd_execute_avr_instruction(AVR_NOP()); // the above is two cycle instruction

#ifdef DEBUG_VERBOSE
		UARTWrite("\r\n");
		ocd_dump_debug_registers();
#endif
	}

#ifdef DEBUG_VERBOSE
	UARTWrite("\r\n");
#endif

	return 1;
}

uint8_t ocd_set_psb0(uint16_t addr) {
	// write adress to psb0 register in context type
	// the cpu frags this values when running, so we must refresh it every time
	avrContext.PSB0 = addr;

	avrContext.break_config |= AVR_EN_PSB0;
}

uint8_t ocd_set_psb1(uint16_t addr) {
	// write adress to psb1 register

	avrContext.PSB1 = addr;

	avrContext.break_config |= AVR_EN_PSB1;
}

uint8_t ocd_set_pdmsb(uint16_t addr, uint8_t mode) {
	// write adress to psmsb register

	avrContext.PDMSB = addr;

	if (mode != 4) { // 4 is break_mask
		avrContext.break_config |= AVR_EN_PDMSB;
		avrContext.break_config = (avrContext.break_config & ~(AVR_PDMSB_MODE0|AVR_PDMSB_MODE1)) | ((mode & 0x3) << 5);
	}
	else {
		avrContext.break_config |= AVR_EN_PDMSB | AVR_MASK_BREAK;
	}
}

uint8_t ocd_set_pdsb(uint16_t addr, uint8_t mode) {
	// write adress to pdsb register

	avrContext.PDSB = addr;
	avrContext.break_config |= AVR_EN_PDSB;
	avrContext.break_config = (avrContext.break_config & ~(AVR_PDSB_MODE0|AVR_PDSB_MODE1)) | ((mode & 0x3) << 3);
}


uint8_t ocd_clr_psb0()  {
	avrContext.PSB0 = 0;

	avrContext.break_config &= ~AVR_EN_PSB0;
}

uint8_t ocd_clr_psb1()  {
	avrContext.PSB1 = 0;

	avrContext.break_config &= ~AVR_EN_PSB1;
}

uint8_t ocd_clr_pdsb()  {
	avrContext.PDSB = 0;

	avrContext.break_config &= ~AVR_EN_PDSB;
}

uint8_t ocd_clr_pdmsb()  {
	avrContext.PDMSB = 0;

	avrContext.break_config &= ~(AVR_EN_PDMSB | AVR_MASK_BREAK);
}

/*----------------------------------------------------------------------*
 * write to on chip debugging registers                                 *
 *----------------------------------------------------------------------*/
unsigned char
wr_dbg_ocd (unsigned char reg, unsigned char *buf, unsigned delay) {
	unsigned char stat, tdi [3], tdo [3];

//	jtag_reset();
	avr_jtag_instr (AVR_OCD, delay);
	tdi [0] = buf [0];
	tdi [1] = buf [1];
	tdi [2] = reg | AVR_WR_OCDR; // start with writing
	jtag_write(21,tdi);
	jtag_goto_state(UPDATE_DR);

	return 1;
}

/*----------------------------------------------------------------------*
 * read from on chip debugging registers                                *
 * prepare to read the register : in the second run you can get the data*
 * acts just like a shift register : when you must read you must clock  *
 * in redundant data in order to read the shift register                *
 *----------------------------------------------------------------------*/
unsigned char
rd_dbg_ocd (unsigned char reg, unsigned char *buf_out, unsigned char delay) {
	unsigned char stat, tdo [3], tdi [3];
	//jtag_reset();
	avr_jtag_instr (AVR_OCD, delay);
	tdi [0] = reg;
	jtag_write(5,tdi); // write register adress to OCD - leading 0 indicated reading
	jtag_goto_state(UPDATE_DR);
	jtag_goto_state(SHIFT_DR);

	tdi [0]     = 0;
	tdi [1]     = 0;
	tdi [2]     = reg;
	jtag_read (16, tdo);
	buf_out [0] = tdo [0];
	buf_out [1] = tdo [1];
	return 1;
}

/*----------------------------------------------------------------------*
 *----------------------------------------------------------------------*/
unsigned char
rd_dbg_channel (unsigned char *buf_out, unsigned char delay) {
	unsigned char stat, tdo [2], tdi [2];

	jtag_reset();
	avr_jtag_instr (AVR_OCD, delay);
	tdi [0] = AVR_DBG_COMM_DATA;
	jtag_write_and_read (5,tdi, tdo);

	tdi [0]     = 0;
	tdi [1]     = 0;
	jtag_write_and_read (16,tdi, tdo);
	buf_out [0] = tdo [0];
	buf_out [1] = tdo [1];

	return 1;
}

/*----------------------------------------------------------------------*
 *----------------------------------------------------------------------*/
unsigned char
force_avr_stop (unsigned char delay) {
	jtag_reset();
	avr_jtag_instr (AVR_FORCE_BRK, delay);
	return 1;
}

/*----------------------------------------------------------------------*
 *----------------------------------------------------------------------*/
unsigned char init_avr_jtag (struct avr_reg *reg, unsigned char delay) {
	unsigned char stat, buf_in [4], buf_out [4], *p, *q;
	char          cnt;
	//jtag_reset();
	//avr_reset (0);
	rd_dbg_ocd (AVR_DBG_COMM_CTL, buf_out, delay);
	//avr_reset(0);
	avr_jtag_instr (AVR_FORCE_BRK, delay);
	rd_dbg_ocd (AVR_DBG_COMM_CTL, buf_out, delay);
	//avr_reset (1);

	/*
	 * set break point at reset vector :
	 * write (= 1 bit) breakpoint register PSB0 (4 bits) : address 0 (= 16
	 * bits).  makes total 21 bits -> 3 bytes
	 */
	buf_in [0] = 0;
	buf_in [1] = 0;
	wr_dbg_ocd (AVR_PSB0, buf_in, delay);
	/*
	 * prepare to read control and status register
	 */
	rd_dbg_ocd (AVR_BCR, buf_out, delay);
	/*
	 * write control and status register with PSB1 activated
	 */
	buf_in [0] = 0;
	buf_in [1] = AVR_EN_PSB1;
	wr_dbg_ocd (AVR_BCR, buf_in, delay);

	/*
	 * and let the target run
	 */
	//avr_reset (0);
	rd_dbg_ocd (AVR_DBG_COMM_CTL, buf_out, delay);

	/*
	 * prepare to read control and status register again
	 */
	rd_dbg_ocd (AVR_BCR, buf_out, delay);

	/*
	 * write control and status register with PSB0 deactivated
	 * prepare to read the pc
	 */
	buf_in [0] = 0;
	buf_in [1] = 0;
	wr_dbg_ocd (AVR_BCR, buf_in, delay);

	/*
	 * prepare to read the pc
	 */
	exec_instr_avr (buf_out, buf_in, CHK_PC, delay);
	rd_dbg_ocd (AVR_DBG_COMM_CTL, buf_out, delay);

	buf_in [0] = buf_out [0];
	buf_in [1] = buf_out [1] | AVR_EN_OCDR;
	wr_dbg_ocd (AVR_DBG_COMM_CTL, buf_in, delay);

	buf_in [0] = 0xE1;
	buf_in [1] = 0xBF;
	buf_in [2] = 0xF1;
	buf_in [3] = 0xBF;
	q          = (unsigned char *) &(reg->pc);
	p          = buf_in;
	cnt        = 1;
	do {
			exec_instr_avr (buf_out, p, 0, delay);
			p += 2;
			rd_dbg_channel (buf_out, delay);
			*q++ = buf_out [1];
		} while ((!(--cnt & 0x80)));


	rd_dbg_ocd (AVR_DBG_COMM_CTL, buf_out, delay);
	buf_in [0] = BIT3 | BIT2;
	buf_in [1] = AVR_EN_OCDR;
	wr_dbg_ocd (AVR_DBG_COMM_CTL, buf_in, delay);

	return 1;
}

/*----------------------------------------------------------------------*
 * this one really gets the register values right after a break, stop,  *
 * or initial condition                                                 *
 *----------------------------------------------------------------------*/
#if 0
void
get_avr_regs (unsigned char delay) {
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
unsigned char
activate_ocd (unsigned char delay) {
	unsigned char stat, buf_in [2];

	do {
			jtag_reset ();
			avr_jtag_instr (AVR_FORCE_BRK, delay);
			buf_in [0] = BIT2;
			buf_in [1] = AVR_EN_OCDR;
			wr_dbg_ocd (AVR_DBG_COMM_CTL, buf_in, delay);
			rd_dbg_ocd (AVR_DBG_COMM_CTL, buf_in, delay);
		} while (((buf_in [0] & (BIT2 | BIT3)) != (BIT2 | BIT3)));
#ifdef DEBUG
	SendHex(buf_in [0]);
#endif
	return 1;
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
				  unsigned char delay) {

	unsigned char stat, *p, buf_in [2], buf_out [2];
	short         num;

	num  = 0;
	//init_avr_jtag (&(reg.avr), 0);
	activate_ocd (delay);

	p    = (unsigned char *) &addr;
	p   += 2;
	/*
	 * ldi      r31,hi8(addr)
	 */
	buf_in [0] = (*p & 0xF) + 0xF0;
	buf_in [1] = ((*p & 0xF0) >> 4) + 0xE0;
	exec_instr_avr (buf_out, buf_in, 0, delay);

	/*
	 * ldi      r30,lo8(addr)
	 */

	buf_in [0] = (*++p & 0xF) + 0xE0;
	buf_in [1] = ((*p & 0xF0) >> 4) + 0xE0;
	exec_instr_avr (buf_out, buf_in, 0, delay);

	//SendHex(0x99);
	while (num < size) {
			/*
			 * lpm  r0,z+
			 */
			buf_in [0] = 5;
			buf_in [1] = 0x90;
			exec_instr_avr (buf_out, buf_in, 0, delay);
			/*
			 * out  ocdr,r0
			 */
			buf_in [0] = 1;
			buf_in [1] = 0xBE;
			exec_instr_avr (buf_out, buf_in, 0, delay);
			rd_dbg_channel (buf_out, delay);
			//SendHex(buf_out [0]);
			//SendHex(buf_out [1]);
			*buf++ = buf_out [1];
			num++;
		}

	return 1;
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
			   unsigned char delay) {
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
