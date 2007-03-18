/* Copyright (C) 2002-2004  Mark Andrew Aikens <marka@desert.cx>
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
 * $Id: Pic18Device.cxx,v 1.3 2006/01/02 17:43:24 gaufille Exp $
 */
using namespace std;

#include <stdexcept>
#include "PicDevice.h"
#include "Util.h"

#define PANEL_SHIFT 13
#define PANELSIZE (1 << PANEL_SHIFT)	/* bytes */

/* ID locations: 0x200000 - 0x200007 (byte address)
 * Config words: 0x300000 - 0x30000D (byte address)
 * Device ID:    0x3ffffe - 0x3fffff (byte address)
 * Data EEPROM:  0xf00000 (byte address)
 */

Pic18Device::Pic18Device(char *name) : PicDevice(name) {
	char buf[16];
	long value;
	int i;

	/* Read in config bits */
	for(i=0; i<7; i++) {
		sprintf(buf, "configmask%d", i);
		if(pic_config->get_integer(name, buf, &value)) {
			config_masks[i] = value;
		} else {
			config_masks[i] = 0xffff;
		}
	}

	/* Create the memory map for this device. Note that these are 16-bit word
	 * offsets and lengths which are 1/2 of their byte equivalents */
	this->memmap.push_back(IntPair (0, this->codesize));
	this->memmap.push_back(IntPair (0x200000/2, 4));	/* ID locations */
	this->memmap.push_back(IntPair (0x300000/2, 7));	/* Config words */
	if(this->flags & PIC_FEATURE_EEPROM)
		this->memmap.push_back(IntPair (0xf00000/2, this->eesize/2));
}


Pic18Device::~Pic18Device() {
}


uint32_t Pic18Device::read_deviceid(void) {
	unsigned int d1, d2;

	/* Read and check the device ID */
	set_tblptr(0x3ffffe);
	d1 = write_command_read_data(COMMAND_TABLE_READ_POSTINC);
	d2 = write_command_read_data(COMMAND_TABLE_READ_POSTINC);

	return d1 | d2<<8;
}


void Pic18Device::erase(void) {
	if(this->memtype != MEMTYPE_FLASH)
		throw runtime_error("Operation not supported by device");

	try {
		set_program_mode();

		set_tblptr(0x3c0004);
		write_command(COMMAND_TABLE_WRITE, 0x0080);	/* "Chip Erase" */
		write_command(COMMAND_CORE_INSTRUCTION, 0x0000); /* nop */
		write_command(COMMAND_CORE_INSTRUCTION, 0x0000); /* nop */
		this->io->usleep(this->erase_time);

		pic_off();
	} catch(std::exception& e) {
		pic_off();
		throw;
	}
}


void Pic18Device::program(DataBuffer& buf) {
	switch(this->memtype) {
	case MEMTYPE_EPROM:
	case MEMTYPE_FLASH:
		break;
	default:
		throw runtime_error("Unsupported memory type in device");
	}

	/* Progress_total is x2 because we write and verify every location */
	this->progress_total = 2 * (this->codesize + 4 + 7 + this->eesize) - 1;
	this->progress_count = 0;

	try {
		set_program_mode();

		write_program_memory(buf, true);
		write_id_memory(buf, 0x200000, true);
		if(flags & PIC_FEATURE_EEPROM)
			write_data_memory(buf, 0xf00000, true);
		write_config_memory(buf, 0x300000, true);

		pic_off();
	} catch(std::exception& e) {
		pic_off();
		throw;
	}
}


void Pic18Device::read(DataBuffer& buf, bool verify) {
	this->progress_total = this->codesize + 4 + 7 + this->eesize - 1;
	this->progress_count = 0;

	try {
		set_program_mode();

		read_memory(buf, 0, this->codesize, verify);	/* Program memory */
		read_memory(buf, 0x200000, 4, verify);			/* ID memory */
		read_config_memory(buf, 0x300000, 7, verify);	/* Config words */
		if(flags & PIC_FEATURE_EEPROM)
		{
			read_data_memory(buf, 0xf00000, verify);
		}

		pic_off();
	} catch(std::exception& e) {
		pic_off();
		throw;
	}
}


void Pic18Device::write_program_memory(DataBuffer& buf, bool verify) {
	unsigned int npanels, panel, offset;
 
	panel = 0;
	offset = 0;
	npanels = this->codesize/(PANELSIZE/2);
	try {
		/* Step 1: Enable multi-panel writes */
		set_tblptr(0x3c0006);
		write_command(COMMAND_TABLE_WRITE, 0x0040);

		/* Step 2: Direct access to code memory */
		/* BSF EECON1, EEPGD */
		write_command(COMMAND_CORE_INSTRUCTION, 0x8ea6);
		/* BCF EECON1, CFGS */
		write_command(COMMAND_CORE_INSTRUCTION, 0x9ca6);

		for(offset=0; offset < PANELSIZE; offset+=8) {
			/* Step 3,4,5,6: Load write buffer for Panel 1,2,3,4 */
			for(panel=0; panel<npanels-1; panel++) {
				/* Give byte addresses to progress() to match datasheet. */
				progress((panel << PANEL_SHIFT) + offset);
				load_write_buffer(buf, panel, offset, false);
			}
			load_write_buffer(buf, panel, offset, true);

			program_wait();

			if(verify) {
				/* Verify the memory just written */
				for(panel=0; panel<npanels; panel++) {
					progress((panel << PANEL_SHIFT) + offset);
					/* Verify the 4 words per panel */
					read_memory(buf, (panel << PANEL_SHIFT) + offset, 4, true);
				}
			}
		}
	} catch(std::exception& e) {
		THROW_ERROR(runtime_error,
		  "Couldn't write program memory at address 0x%06lx: %s",
		  (unsigned long)(panel << PANEL_SHIFT) + offset, e.what());
	}
}


void Pic18Device::write_id_memory(DataBuffer& buf, unsigned long addr,
  bool verify) {
	progress(addr);
	try {
		/* Step 1: Direct access to config memory */
		/* BSF EECON1, EEPGD */
		write_command(COMMAND_CORE_INSTRUCTION, 0x8ea6);
		/* BSF EECON1, CFGS */
		write_command(COMMAND_CORE_INSTRUCTION, 0x8ca6);

		/* Step 2: Configure device for single panel writes */
		set_tblptr(0x3c0006);
		write_command(COMMAND_TABLE_WRITE, 0x0000);

		/* Step 3: Direct access to code memory */
		/* BSF EECON1, EEPGD */
		write_command(COMMAND_CORE_INSTRUCTION, 0x8ea6);
		/* BCF EECON1, CFGS */
		write_command(COMMAND_CORE_INSTRUCTION, 0x9ca6);

		/* Step 4: Load write buffer. Panel will be automatically determined
		 * by address. */
		load_write_buffer(buf, addr/PANELSIZE, addr%PANELSIZE, true);
		program_wait();

		if(verify) {
			/* Verify the memory just written */
			read_memory(buf, addr, 4, true);
		}
	} catch(std::exception& e) {
		THROW_ERROR(runtime_error,
		  "Couldn't write ID memory at address 0x%06lx", addr);
	}
}


void Pic18Device::write_data_memory(DataBuffer& buf, unsigned long addr,
  bool verify)
{
	uint32_t ins;
	uint8_t data;
	unsigned int offset;

	offset = 0;
	try {
		/* Step 1: Direct access to data EEPROM */
		/* BCF EECON1, EEPGD */
		write_command(COMMAND_CORE_INSTRUCTION, 0x9ea6);
		/* BCF EECON1, CFGS */
		write_command(COMMAND_CORE_INSTRUCTION, 0x9ca6);

		for(offset=0; offset < this->eesize; offset++) {
			progress(addr+(2*offset));

			/* Step 2: Set the data EEPROM address pointer */
			ins = 0x0e00 | (offset & 0xff);			/* movlw <addr lo> */
			write_command(COMMAND_CORE_INSTRUCTION, ins);
			write_command(COMMAND_CORE_INSTRUCTION, 0x6ea9);/* movwf EEADR */
			ins = 0x0e00 | ((offset >> 8) & 0xff);		/* movlw <addr hi> */
			write_command(COMMAND_CORE_INSTRUCTION, ins);
			write_command(COMMAND_CORE_INSTRUCTION, 0x6eaa);/* movwf EEADRH */

			/* Step 3: Load the data to be written */
			if((offset & 1) == 0) {
				data = buf[(addr+offset)/2] & 0xff;
			} else {
				data = (buf[(addr+offset)/2] >> 8) & 0xff;
			}
			ins = 0x0e00 | data;	/* movlw <data> */
			write_command(COMMAND_CORE_INSTRUCTION, ins);
			write_command(COMMAND_CORE_INSTRUCTION, 0x6ea8);/* movwf EEDATA */

			/* Step 4: Enable memory writes */
			/* BSF EECON1, WREN */
			write_command(COMMAND_CORE_INSTRUCTION, 0x84a6);

			/* Step 5: Perform required sequence */
			/* MOVLW 0x55 */
			write_command(COMMAND_CORE_INSTRUCTION, 0x0e55);
			/* MOVWF EECON2 */
			write_command(COMMAND_CORE_INSTRUCTION, 0x6ea7);
			/* MOVLW 0xAA */
			write_command(COMMAND_CORE_INSTRUCTION, 0x0eaa);
			/* MOVWF EECON2 */
			write_command(COMMAND_CORE_INSTRUCTION, 0x6ea7);

			/* Step 6: Initiate write */
			/* BSF EECON1, WR */
			write_command(COMMAND_CORE_INSTRUCTION, 0x82a6);

			/* Step 7: Poll WR bit, repeat until the bit is clear */
			do {
				/* MOVF EECON1, W, 0 */
				write_command(COMMAND_CORE_INSTRUCTION, 0x50a6);
				/* MOVWF TABLAT */
				write_command(COMMAND_CORE_INSTRUCTION, 0x6ef5);

				ins = write_command_read_data(COMMAND_SHIFT_OUT_TABLAT);
			} while(ins & 0x02);

			/* Step 8: Disable writes */
			/* BCF EECON1, WREN */
			write_command(COMMAND_CORE_INSTRUCTION, 0x94a6);

			this->progress_count++;

			if(verify) {
				/* Initiate a memory read */
				/* BSF EECON1, RD */
				write_command(COMMAND_CORE_INSTRUCTION, 0x80a6);

				/* Load data into the serial data holding register */
				/* MOVF EEDATA, W, 0 */
				write_command(COMMAND_CORE_INSTRUCTION, 0x50a8);
				/* MOVWF TABLAT */
				write_command(COMMAND_CORE_INSTRUCTION, 0x6ef5);

				/* Shift out data and check */
				ins = write_command_read_data(COMMAND_SHIFT_OUT_TABLAT);
				if(ins != data)
					throw runtime_error("");
				this->progress_count++;
			}
		}
	} catch(std::exception& e) {
		THROW_ERROR(runtime_error,
		  "Couldn't write data memory at location %d", offset);
	}
}


void Pic18Device::write_config_memory(DataBuffer& buf, unsigned long addr,
  bool verify) {
	int i;

	i = 0;
	try {
		/* Step 1: Direct access to config memory */
		/* BSF EECON1, EEPGD */
		write_command(COMMAND_CORE_INSTRUCTION, 0x8ea6);
		/* BSF EECON1, CFGS */
		write_command(COMMAND_CORE_INSTRUCTION, 0x8ca6);

		/* Step 2: Position the program counter */
		/* GOTO 100000h */
		write_command(COMMAND_CORE_INSTRUCTION, 0xef00);
		write_command(COMMAND_CORE_INSTRUCTION, 0xf800);

		for(i=0; i<7; i++) {
			/* Give byte addresses to the callback to match datasheet. */
			progress(addr);

			/* Step 3: Set Table Pointer for config byte to be written. Write
			 * even/odd addresses */
			set_tblptr(addr);
			write_command(COMMAND_TABLE_WRITE_START, buf[(addr/2)] & 0xff);
			program_wait();

			write_command(COMMAND_CORE_INSTRUCTION, 0x2af6); /* INCF TBLPTR */
			write_command(COMMAND_TABLE_WRITE_START, buf[(addr/2)] & 0xff00);
			program_wait();

			this->progress_count++;
			if(verify) {
				/* Verify the config word just written */
				read_config_memory(buf, addr, 1, true);
			}
			addr += 2;
		}
	} catch(std::exception& e) {
		THROW_ERROR(runtime_error,
		  "Couldn't write configuration word #%d", i);
	}
}


void Pic18Device::read_memory(DataBuffer& buf, unsigned long addr,
  unsigned long len, bool verify) {
	unsigned int data;

	try {
		addr >>= 1;			/* Shift to word addresses */
		set_tblptr(2*addr);
		while(len > 0) {
			/* Give byte addresses to progress() to match datasheet. */
			progress(addr*2);

			/* Read memory a byte at a time (little endian format) */
			data = write_command_read_data(COMMAND_TABLE_READ_POSTINC);
			data |= (write_command_read_data(COMMAND_TABLE_READ_POSTINC) << 8);
			if(verify) {
				if(data != buf[addr])
					throw runtime_error("");
			} else {
				buf[addr] = data;
			}
			addr++;
			len--;
			this->progress_count++;
		}
	} catch(std::exception& e) {
		THROW_ERROR(runtime_error, "%s at address 0x%06lx",
		  verify ? "Verification failed" : "Couldn't read memory",
 		  2*addr);
	}
}


void Pic18Device::read_config_memory(DataBuffer& buf, unsigned long addr,
  unsigned long len, bool verify) {
	unsigned int data, cword_num;
	unsigned long i;

	cword_num = 0;
	try {
		addr >>= 1;			/* Shift to word addresses */
		set_tblptr(2*addr);
		for(i=0; i<len; i++) {
			/* Give byte addresses to progress() to match datasheet. */
			progress(addr*2);

			/* Read memory a byte at a time (little endian format) */
			cword_num = addr - (0x300000/2);
			data = write_command_read_data(COMMAND_TABLE_READ_POSTINC);
			data |= (write_command_read_data(COMMAND_TABLE_READ_POSTINC) << 8);
			if(verify) {
				if((data & this->config_masks[cword_num]) !=
				  (buf[addr] & this->config_masks[cword_num]))
					throw runtime_error("");
			} else {
				buf[addr] = data;
			}
			addr++;
			this->progress_count++;
		}
	} catch(std::exception& e) {
		THROW_ERROR(runtime_error, "%s of configuration word #%u failed",
		  verify ? "Verification" : "Read", cword_num);
	}
}


void Pic18Device::read_data_memory(DataBuffer& buf, unsigned long addr,
  bool verify) {
	uint32_t ins;
	unsigned int offset, data;

	offset = 0;
	try {
		/* Direct address to data EEPROM */
		write_command(COMMAND_CORE_INSTRUCTION, 0x9ea6);/* bcf EECON1, EEPGD */
		write_command(COMMAND_CORE_INSTRUCTION, 0x9ca6);/* bcf EECON1, CPGS */

		for(offset=0; offset<this->eesize; offset++) {
			/* Give byte addresses to progress() to match datasheet. */
			progress(addr+(2*offset));

			/* Set the data EEPROM address pointer */
			ins = 0x0e00 | (offset & 0xff);				/* movlw <addr> */
			write_command(COMMAND_CORE_INSTRUCTION, ins);
			write_command(COMMAND_CORE_INSTRUCTION, 0x6ea9);/* movwf EEADR */

			/* Initiate a memory read */
			/* bsf EECON1, RD */
			write_command(COMMAND_CORE_INSTRUCTION, 0x80a6);

			/* Load data into the serial data holding register */
			/* movf EEDATA, W, 0 */
			write_command(COMMAND_CORE_INSTRUCTION, 0x50a8);
			/* movwf TABLAT */
			write_command(COMMAND_CORE_INSTRUCTION, 0x6ef5);

			/* Shift out data */
			data = write_command_read_data(COMMAND_SHIFT_OUT_TABLAT);
			if(verify) {
				/* The data is packed 2 bytes per word, little endian */
				if((offset & 1) == 0) {
					if(data != (buf[(addr + offset)/2] & 0xff))
						throw runtime_error("");
				} else {
					if(data != ((buf[(addr + offset)/2] >> 8) & 0xff))
						throw runtime_error("");
				}
			} else {
				/* The data is packed 2 bytes per word, little endian */
				if((offset & 1) == 0) {
					buf[(addr + offset)/2] &= ~0x00ff;
					buf[(addr + offset)/2] |= data;
				} else {
					buf[(addr + offset)/2] &= ~0xff00;
					buf[(addr + offset)/2] |= (data << 8);
				}
			}
			this->progress_count++;
		}
	} catch(std::exception& e) {
		THROW_ERROR(runtime_error, "%s at location %d",
		  verify ? "Data memory verification failed" :
		  "Couldn't read data memory", offset);
	}
}


void Pic18Device::load_write_buffer(DataBuffer& buf, unsigned int panel,
  unsigned int offset, bool last) {
	unsigned long addr;
	int i;

	addr = (panel << PANEL_SHIFT) + offset;
	set_tblptr(addr);
	for(i=0; i<3; i++) {		/* Write 3 words */
		write_command(COMMAND_TABLE_WRITE_POSTINC, buf[(addr / 2) + i]);
		this->progress_count++;
	}
	if(last) {
		/* Do the final word write which also starts the programming */
		write_command(COMMAND_TABLE_WRITE_START, buf[(addr / 2) + 3]);
	} else {
		/* Just another load */
		write_command(COMMAND_TABLE_WRITE_POSTINC, buf[(addr / 2) + 3]);
	}
	this->progress_count++;
}


void Pic18Device::program_wait(void) {
	this->io->shift_bits_out(0x00, 3);
	this->io->set_data(false);
	this->io->set_clk(true);	/* Hold clk high */

	this->io->usleep(program_time);

	this->io->set_clk(false);
	this->io->usleep(5);	/* High-voltage discharge time */
	this->io->shift_bits_out(0x0000, 16);/* 16-bit payload (NOP) */
}


void Pic18Device::set_tblptr(unsigned long addr) {
	uint32_t ins;

	ins = 0x0e00 | ((addr >> 16) & 0x3f);	/* movlw addr[21:16] */
	write_command(COMMAND_CORE_INSTRUCTION, ins);
	write_command(COMMAND_CORE_INSTRUCTION, 0x6ef8);/* movwf TBLPTRU */
	ins = 0x0e00 | ((addr >> 8) & 0xff);	/* movlw addr[15:8] */
	write_command(COMMAND_CORE_INSTRUCTION, ins);
	write_command(COMMAND_CORE_INSTRUCTION, 0x6ef7);/* movwf TBLPTRH */
	ins = 0x0e00 | (addr & 0xff);			/* movlw addr[7:0] */
	write_command(COMMAND_CORE_INSTRUCTION, ins);
	write_command(COMMAND_CORE_INSTRUCTION, 0x6ef6);/* movwf TBLPTRL */
}


void Pic18Device::write_command(unsigned int command) {
	this->io->shift_bits_out(command, 4);
	this->io->usleep(1);
}


void Pic18Device::write_command(unsigned int command, unsigned int data) {
	write_command(command);
	this->io->shift_bits_out(data, 16);
	this->io->usleep(1);
}


unsigned int Pic18Device::write_command_read_data(unsigned int command) {
	write_command(command);
	this->io->shift_bits_out(0x00, 8);		/* 8 dummy bits */
	this->io->usleep(1);
	return (this->io->shift_bits_in(8) & 0xff);
}

void Pic18Device::set_config_default(DataBuffer& buf)
{
	/* On most PICs 18, the configuration memory is only = 0xffff */
	int i;
	char sbuf[32];
	long value;

	for(i=0; i<7; i++) {
		sprintf(sbuf, "configdefault%d", i);
		if(pic_config->get_integer(name, sbuf, &value)) {
			buf[(0x300000 >> 1) + i] = value;
		} else {
			buf[(0x300000 >> 1) + i] = 0xffff;
		}
	}
}
