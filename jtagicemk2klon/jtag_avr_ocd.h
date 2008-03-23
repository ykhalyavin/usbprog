/*-------------------------------------------------------------------------
 * JTAG_AVR_OCD.H
 * Copyright (C) 2003 Armand ten Doesschate <a.doesschate@hccnet.nl>
 * Copyright (C) 2007 Benedikt Sauter <satrer@sistecs.de>
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
	uint8_t r16;
	uint8_t r30;
	uint8_t r31;

	uint8_t registerDirty;
};

unsigned char ocd_save_context();
unsigned char ocd_restore_context();

unsigned char ocd_execute_avr_instruction(uint16_t instr);
uint8_t ocd_read_ocdr();

uint16_t ocd_read_pc();

uint8_t ocd_rd_sram(uint16_t startaddr, uint16_t len, uint8_t *buf);
uint8_t ocd_wr_sram(uint16_t startaddr, uint16_t len, uint8_t *buf);

uint8_t ocd_rd_flash(uint16_t startaddr, uint16_t len, uint8_t *buf);
uint8_t ocd_wr_flash(uint16_t startaddr, uint16_t len, uint8_t *buf);

uint8_t ocd_rd_eeprom(uint16_t startaddr, uint16_t len, uint8_t *buf);
uint8_t ocd_wr_eeprom(uint16_t startaddr, uint16_t len, uint8_t *buf);



unsigned char rd_flash_ocd_avr (unsigned long addr, unsigned char *buf,
                       short size, unsigned char delay);
unsigned char rd_sram_ocd_avr (unsigned short addr, unsigned char *buf,
                               short size, unsigned char delay);
unsigned char rd_io_ocd_avr (unsigned char addr, unsigned char *buf,
                             short size, unsigned char delay);
unsigned char rd_e2_ocd_avr (unsigned short addr, unsigned char *buf,
                             short size, unsigned char delay);
unsigned char wr_avr_flash_ocd (unsigned long addr, unsigned char *buf,
                                short size, unsigned char delay);
unsigned char wr_sram_ocd_avr (unsigned short addr, unsigned char *buf,
                               short size, unsigned char delay);
unsigned char wr_io_ocd_avr (unsigned char addr, unsigned char *buf,
                             short size, unsigned char delay);


unsigned char wr_dbg_ocd (unsigned char reg, unsigned char *buf, unsigned delay);
unsigned char rd_dbg_ocd (unsigned char reg, unsigned char *buf_out, unsigned char delay);
unsigned char rd_dbg_channel (unsigned char *buf_out, unsigned char delay);


void step_avr (unsigned char delay);
unsigned char force_avr_stop (unsigned char delay);
unsigned char run_avr (unsigned char mode, unsigned char go_flg,
                              unsigned long addr, unsigned char delay);

unsigned char init_avr_jtag (struct avr_reg *reg, unsigned char delay);

void init_all_regs_avr (void);
void get_all_regs_avr (unsigned char delay);
void set_all_regs_avr (unsigned char *buf, unsigned char delay);

unsigned char activate_ocd (unsigned char delay);

unsigned char
exec_instr_avr (unsigned char *out, unsigned char *in, unsigned char flg,
                unsigned char delay);

#define write_flash		0x01


#endif
