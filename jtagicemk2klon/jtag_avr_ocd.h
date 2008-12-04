/*-------------------------------------------------------------------------
 * JTAG_AVR_OCD.H
 * Copyright (C) 2003 Armand ten Doesschate <a.doesschate@hccnet.nl>
 * Copyright (C) 2007 Benedikt Sauter <satrer@sistecs.de>
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

#ifndef JTAG_AVR_OCD_H
#define JTAG_AVR_OCD_H

struct AVR_Context_Type {
	uint8_t STATUS;
	uint8_t r16;
	uint8_t r30;
	uint8_t r31;

	uint16_t PC;

	uint16_t PSB0;
	uint16_t PSB1;
	uint16_t PDMSB;
	uint16_t PDSB;

	/* This Value contains the current breakpoint config
	 * It saves which breakpoints are currently enabled, and
	 * in which mode we use the 2 data/program/mask breakpoints
	 * therefore it holds the values like in OCD BCR Register
	 *
	 * According to JTAGICE Specification it get's cleared on break
	 */
	uint16_t break_config;


	uint8_t registerDirty;
};


extern struct AVR_Context_Type avrContext;

unsigned char ocd_save_context(void);
unsigned char ocd_restore_context(void);

unsigned char ocd_execute_avr_instruction(uint16_t instr);
uint8_t ocd_read_ocdr(void);

uint16_t ocd_read_pc(void);

uint8_t ocd_rd_sram(uint16_t startaddr, uint16_t len, uint8_t *buf);
uint8_t ocd_wr_sram(uint16_t startaddr, uint16_t len, uint8_t *buf);

uint8_t ocd_rd_flash(uint16_t startaddr, uint16_t len, uint8_t *buf);
uint8_t ocd_erase_flash_page(uint16_t pageaddr);
uint8_t ocd_spm_sequence(uint8_t spmcr, uint8_t zlow, uint8_t zhigh);
uint8_t ocd_read_spmcr(void);


uint8_t ocd_rd_eeprom(uint16_t startaddr, uint16_t len, uint8_t *buf);
uint8_t ocd_wr_eeprom(uint16_t startaddr, uint16_t len, uint8_t *buf);

// breakpoint support
uint8_t ocd_set_psb0(uint16_t addr);
uint8_t ocd_set_psb1(uint16_t addr);
uint8_t ocd_set_pdmsb(uint16_t addr, uint8_t mode);
uint8_t ocd_set_pdsb(uint16_t addr, uint8_t mode);

uint8_t ocd_clr_psb0(void);
uint8_t ocd_clr_psb1(void);
uint8_t ocd_clr_pdmsb(void);
uint8_t ocd_clr_pdsb(void);


unsigned char wr_dbg_ocd (unsigned char reg, unsigned char *buf, unsigned delay);
unsigned char rd_dbg_ocd (unsigned char reg, unsigned char *buf_out, unsigned char delay);

#define write_flash		0x01


#endif
