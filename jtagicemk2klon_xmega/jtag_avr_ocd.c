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
#include "jtagice2.h"
#include "avr_asm.h"
#include "uart.h"

static const uint8_t OCDR_Addr = 0x31;
static const uint8_t SPMCR_Addr = 0x57;
static const uint8_t EECR_Addr = 0x1C;
static const uint8_t SREG_Addr = 0x3F;



/* This struct holds essential chip data for debugging.
 * Please use the according accessor functions to modify it
 */
struct AVR_Context_Type avrContext;

/*----------------------------------------------------------------------*
 * write to on chip debugging registers                                 *
 *----------------------------------------------------------------------*/
unsigned char
wr_dbg_ocd (unsigned char reg, unsigned char *buf, unsigned delay)
{
//	unsigned char stat, tdi [3], tdo [3];
	unsigned char tdi [3];

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
rd_dbg_ocd (unsigned char reg, unsigned char *buf_out, unsigned char delay)
{
//	unsigned char stat, tdo [3], tdi [3];
	unsigned char tdo [3], tdi [3];
//	jtag_reset();
	//UARTWrite("PT1\r\n");
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


unsigned char ocd_enshure_ocdr_enable(void)
{
	uint16_t csr;
	rd_dbg_ocd(AVR_DBG_COMM_CTL, (unsigned char *)&csr, 0);

	if (!(csr & AVR_EN_OCDR)) {
		csr |= AVR_EN_OCDR;
		wr_dbg_ocd(AVR_DBG_COMM_CTL, (unsigned char *)&csr, 0);
	}
	return '\0';	// TODO
}

uint8_t ocd_read_ocdr(void)
{
	unsigned char tdo[2];

	rd_dbg_ocd(AVR_DBG_COMM_DATA,tdo,0);

	return tdo[1];
}

/* Saves the avr registers which get used by ocd system */
unsigned char ocd_save_context(void)
{
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
void ocd_dump_debug_registers(void)
{
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
unsigned char ocd_restore_context(void)
{
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

		uint16_t OCDSCR;
		rd_dbg_ocd(AVR_DBG_COMM_CTL, (unsigned char *)&OCDSCR, 0);
		OCDSCR &= ~AVR_EN_OCDR;
		wr_dbg_ocd(AVR_DBG_COMM_CTL, (unsigned char *)&OCDSCR, 0);

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

	wr_dbg_ocd(AVR_PSB0, (unsigned char *)&avrContext.PSB0, 0);
	wr_dbg_ocd(AVR_PSB1, (unsigned char *)&avrContext.PSB1, 0);
	wr_dbg_ocd(AVR_PDMSB, (unsigned char *)&avrContext.PDMSB, 0);
	wr_dbg_ocd(AVR_PDSB, (unsigned char *)&avrContext.PDSB, 0);

	wr_dbg_ocd(AVR_BCR, (unsigned char *)&avrContext.break_config, 0);


	return 1;
}


uint16_t ocd_read_pc(void)
{
	avr_jtag_instr(AVR_INSTR,0);

	unsigned char tdi[4] = { 0, 0 , 0xFF, 0xFF };
	unsigned char tdo[4];

	jtag_write_and_read(32,tdi,tdo);
	jtag_goto_state(RUN_TEST_IDLE);

	uint16_t res = (tdo[1] << 8) | tdo[0];

	return res;
}

unsigned char ocd_execute_avr_instruction(uint16_t instr)
{
	avr_jtag_instr(AVR_INSTR, 0);

	(void)jtag_write(16, (unsigned char *)&instr);
	jtag_goto_state(RUN_TEST_IDLE); // some documentation said that its neccesary to go to IDLE state after executing the instr

	return '\0';		// TODO
}

// TODO: in the following 2 functions pay attention that the debugging registers are not in memory range!
uint8_t ocd_rd_sram(uint16_t startaddr, uint16_t len, uint8_t *buf)
{
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

uint8_t ocd_wr_sram(uint16_t startaddr, uint16_t len, uint8_t *buf)
{
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

uint8_t ocd_rd_flash(uint16_t startaddr, uint16_t len, uint8_t *buf)
{
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

uint8_t ocd_erase_flash_page(uint16_t pageaddr)
{
	// first we have to save some more registers which are involved in this operation
	// there are for page erasing the addressing registers for the Y pointer
	// we have to use the Z pointer to indicate the page, so we must use another one for
	// writing to SPMCR
	uint8_t r28, r29;
	ocd_execute_avr_instruction(AVR_OUT(OCDR_Addr,28));
	r28 = ocd_read_ocdr();
	ocd_execute_avr_instruction(AVR_OUT(OCDR_Addr,29));
	r29 = ocd_read_ocdr();
#ifdef DEBUG_VERBOSE
	UARTWrite("Starting Page Erase...\r\n");
#endif

	ocd_spm_sequence(AVR_SPMCR_SPMEN | AVR_SPMCR_PGERS, pageaddr & 0xFF, pageaddr >> 8);

	// wait for action to complete
#ifdef DEBUG_VERBOSE
	UARTWrite("Wait for completion...\r\n");
#endif
	while (ocd_read_spmcr() & AVR_SPMCR_SPMEN);

#ifdef DEBUG_VERBOSE
	UARTWrite("Page Erased\r\n");
#endif

	// restore registers
	ocd_execute_avr_instruction(AVR_LDI(28,r28));
	ocd_execute_avr_instruction(AVR_LDI(29,r29));
	return 1;
}

uint8_t ocd_spm_sequence(uint8_t spmcr, uint8_t zlow, uint8_t zhigh)
{
	// now we place the address of the SPMCR to the adressing register
	ocd_execute_avr_instruction(AVR_LDI(28,SPMCR_Addr & 0xFF));
	ocd_execute_avr_instruction(AVR_LDI(29,SPMCR_Addr>>8));

	// load the value to be written to r16
	ocd_execute_avr_instruction(AVR_LDI(16,spmcr));

	// then set the pc to beginning of the bootloader space
	ocd_execute_avr_instruction(AVR_LDI(30,deviceDescriptor.ulBootAddress & 0xFF));
	ocd_execute_avr_instruction(AVR_LDI(31,(deviceDescriptor.ulBootAddress >> 8) & 0xFF));
	ocd_execute_avr_instruction(AVR_IJMP());

	// then set the Z register to correct location
	ocd_execute_avr_instruction(AVR_LDI(30,zlow));
	ocd_execute_avr_instruction(AVR_LDI(31,zhigh));

	// and write control character to SPMCR
	ocd_execute_avr_instruction(AVR_STY(16));

	// set BCR to Stepping mode
	uint16_t bcr = AVR_BRK_STEP;
	wr_dbg_ocd(AVR_BCR, (unsigned char *)&bcr, 0);

	// execute SPM and run device
	ocd_execute_avr_instruction(AVR_SPM());
	avr_jtag_instr(AVR_RUN,0);

	// now wait for avr to stop again
	uint16_t ocdcr;
	do {
		rd_dbg_ocd(AVR_DBG_COMM_CTL, (unsigned char *)&ocdcr, 0);
	} while (!(ocdcr & 0xC));

	return 1; // when avr has stopped again the sequence has finished
}

uint8_t ocd_read_spmcr(void)
{
	// load address of the register to Z pointer
	ocd_execute_avr_instruction(AVR_LDI(30,SPMCR_Addr & 0xFF));
	ocd_execute_avr_instruction(AVR_LDI(31,SPMCR_Addr>>8));

	// now load value to r16 and put it to ocdr
	ocd_execute_avr_instruction(AVR_LDZ(16));
	ocd_execute_avr_instruction(AVR_OUT(OCDR_Addr,16));

	return ocd_read_ocdr();
}

/* Ja die Funktion sieht sehr komisch aus - das seh ich ein, aber es geht! */
uint8_t ocd_rd_eeprom(uint16_t startaddr, uint16_t len, uint8_t *buf)
{
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
uint8_t ocd_wr_eeprom(uint16_t startaddr, uint16_t len, uint8_t *buf)
{
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

uint8_t ocd_set_psb0(uint16_t addr)
{
	// write adress to psb0 register in context type
	// the cpu frags this values when running, so we must refresh it every time
	avrContext.PSB0 = addr & jtagice.pcmask;

	avrContext.break_config |= AVR_EN_PSB0;

	return 0;		// TODO
}

uint8_t ocd_set_psb1(uint16_t addr)
{
	// write adress to psb1 register

	avrContext.PSB1 = addr & jtagice.pcmask;

	avrContext.break_config |= AVR_EN_PSB1;

	return 0;		// TODO
}

uint8_t ocd_set_pdmsb(uint16_t addr, uint8_t mode)
{
	// write adress to psmsb register

	if (mode == break_program)
		avrContext.PDMSB = addr & jtagice.pcmask;
	else
		avrContext.PDMSB = addr;

	if (mode != 4) { // 4 is break_mask
		avrContext.break_config |= AVR_EN_PDMSB;
		avrContext.break_config = (avrContext.break_config & ~(AVR_PDMSB_MODE0|AVR_PDMSB_MODE1)) | ((mode & 0x3) << 5);
	}
	else {
		avrContext.break_config |= AVR_EN_PDMSB | AVR_MASK_BREAK;
	}

	return 0;		// TODO
}

uint8_t ocd_set_pdsb(uint16_t addr, uint8_t mode)
{
	// write adress to pdsb register

	if (mode == break_program)
		avrContext.PDSB = addr & jtagice.pcmask;
	else
		avrContext.PDSB = addr;
	avrContext.break_config |= AVR_EN_PDSB;
	avrContext.break_config = (avrContext.break_config & ~(AVR_PDSB_MODE0|AVR_PDSB_MODE1)) | ((mode & 0x3) << 3);

	return 0;		// TODO
}


uint8_t ocd_clr_psb0(void)  {
	avrContext.PSB0 = 0;

	avrContext.break_config &= ~AVR_EN_PSB0;

	return 0;		// TODO
}

uint8_t ocd_clr_psb1(void)  {
	avrContext.PSB1 = 0;

	avrContext.break_config &= ~AVR_EN_PSB1;

	return 0;		// TODO
}

uint8_t ocd_clr_pdsb(void)  {
	avrContext.PDSB = 0;

	avrContext.break_config &= ~AVR_EN_PDSB;

	return 0;		// TODO
}

uint8_t ocd_clr_pdmsb(void)  {
	avrContext.PDMSB = 0;

	avrContext.break_config &= ~(AVR_EN_PDMSB | AVR_MASK_BREAK);

	return 0;		// TODO
}
