/* Copyright (C) 2002-2004  Mark Andrew Aikens <marka@desert.cx>
 * Copyright (C) 2005       Pierre Gaufillet   <pierre.gaufillet@magic.fr>
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
 * $Id: Pic18f2xx0Device.cxx,v 1.4 2006/12/02 22:19:54 gaufille Exp $
 */
using namespace std;

#include <stdexcept>
#include "PicDevice.h"
#include "Util.h"
#include <iostream>

/* ID locations: 0x200000 - 0x200007 (byte address)
 * Config words: 0x300000 - 0x30000D (byte address)
 * Device ID:    0x3ffffe - 0x3fffff (byte address)
 * Data EEPROM:  0xf00000 (byte address)
 */

Pic18f2xx0Device::Pic18f2xx0Device(char *name) : Pic18Device(name) {
	long value;

	if(! pic_config->get_integer(name, "writebuffersize", &value))
		throw runtime_error(string(name) + " should have a writebuffersize entry");
	this->write_buffer_size = value;

	if(! pic_config->get_integer(name, "erasebuffersize", &value))
		throw runtime_error(string(name) + " should have an erasebuffersize entry");
	this->erase_buffer_size = value;
}


Pic18f2xx0Device::~Pic18f2xx0Device() {
}



void Pic18f2xx0Device::erase(void)  {
	if(this->memtype != MEMTYPE_FLASH)
		throw runtime_error("Operation not supported by device");

	try {
		set_program_mode();

		set_tblptr(0x3c0005);
		write_command(COMMAND_TABLE_WRITE, 0x0f0f);	/* "Chip Erase" */
		set_tblptr(0x3c0004);
		write_command(COMMAND_TABLE_WRITE, 0x8787);	/* "Chip Erase" */
		write_command(COMMAND_CORE_INSTRUCTION, 0x0000); /* nop */
		write_command(COMMAND_CORE_INSTRUCTION, 0x0000); /* nop */
		this->io->usleep(this->erase_time);

		pic_off();
	} catch(std::exception& e) {
		pic_off();
		throw;
	}
}


void Pic18f2xx0Device::write_program_memory(DataBuffer& buf, bool verify) {
	unsigned long address;
    unsigned long offset;
    offset=0;
	address = 0;

	try {
		/* Step 1: Direct access to code memory */
		/* BSF EECON1, EEPGD */
		write_command(COMMAND_CORE_INSTRUCTION, 0x8ea6);
		/* BCF EECON1, CFGS */
		write_command(COMMAND_CORE_INSTRUCTION, 0x9ca6);

        while(address < this->codesize)
        {
			set_tblptr(2*address);
			for(offset=0; offset < this->write_buffer_size - 2; offset+=2) {
				/* Give byte addresses to progress() to match datasheet. */
				progress(2*address);
				/* Step 2, 3: Load write buffer */
				load_write_buffer(buf[address], false);
                address++;
			}

			progress(2*address);
            /* Step 4: Load write buffer */
			load_write_buffer(buf[address], true);
            address++;
            program_wait();
        }
        
		if(verify) {
			cout << endl << "Verifying the memory written..." << endl;
			/* Verify the memory just written */
			read_memory(buf, 0x00000, this->codesize, true);
			}
	} catch(std::exception& e) {
		THROW_ERROR(runtime_error,
		  "Couldn't write program memory at address 0x%06lx: %s",
		  address, e.what());
	}
}


void Pic18f2xx0Device::write_id_memory(DataBuffer& buf, unsigned long addr,
  bool verify) {
	progress(addr);
	try {
		/* Step 1: Direct access to code memory */
		/* BSF EECON1, EEPGD */
		write_command(COMMAND_CORE_INSTRUCTION, 0x8ea6);
		/* BCF EECON1, CFGS */
		write_command(COMMAND_CORE_INSTRUCTION, 0x9ca6);

		/* Step 2: Load write buffer with 8 bytes and write. */
		set_tblptr(addr);
		load_write_buffer(buf[(addr>>1)], false);
		load_write_buffer(buf[(addr>>1)+1], false);
		load_write_buffer(buf[(addr>>1)+2], false);
		load_write_buffer(buf[(addr>>1)+3], true);

		/* and wait the device been programmed */
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


void Pic18f2xx0Device::write_data_memory(DataBuffer& buf, unsigned long addr,
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

			/* Step 5: Initiate write */
			/* BSF EECON1, WR */
			write_command(COMMAND_CORE_INSTRUCTION, 0x82a6);

			/* Step 6: Poll WR bit, repeat until the bit is clear */
			do {
				/* MOVF EECON1, W, 0 */
				write_command(COMMAND_CORE_INSTRUCTION, 0x50a6);
				/* MOVWF TABLAT */
				write_command(COMMAND_CORE_INSTRUCTION, 0x6ef5);
				/* required NOP */
				write_command(COMMAND_CORE_INSTRUCTION, 0x0000);

				ins = write_command_read_data(COMMAND_SHIFT_OUT_TABLAT);
			} while(ins & 0x02);

			/* Step 7: Hold PGC low for time P10 */
			this->io->usleep(100);	/* High-voltage discharge time P10 */
			
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
				/* required NOP */
				write_command(COMMAND_CORE_INSTRUCTION, 0x0000);

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


void Pic18f2xx0Device::write_config_memory(DataBuffer& buf, unsigned long addr,
  bool verify) {
	int i;

	i = 0;
	try {
		/* Step 1: Direct access to config memory */
		/* BSF EECON1, EEPGD */
		write_command(COMMAND_CORE_INSTRUCTION, 0x8ea6);
		/* BSF EECON1, CFGS */
		write_command(COMMAND_CORE_INSTRUCTION, 0x8ca6);

		for(i=0; i<7; i++) {
			/* Give byte addresses to the callback to match datasheet. */
			progress(addr);

			/* Step 2: Set Table Pointer for config byte to be written. Write
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


void Pic18f2xx0Device::read_data_memory(DataBuffer& buf, unsigned long addr,
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
			/* a required nop */
			write_command(COMMAND_CORE_INSTRUCTION, 0x0000);

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


void Pic18f2xx0Device::load_write_buffer(unsigned int word, bool last) {
	if(last) {
		/* Do the final word write which also starts the programming */
		write_command(COMMAND_TABLE_WRITE_START, word);
	} else {
		/* Just another load */
		write_command(COMMAND_TABLE_WRITE_POSTINC, word);
	}
	this->progress_count++;
}


void Pic18f2xx0Device::program_wait(void) {
	this->io->shift_bits_out(0x00, 3);
	this->io->set_data(false);
	this->io->set_clk(true);	/* Hold clk high */

	this->io->usleep(program_time); /* P9 */

	this->io->set_clk(false);
	this->io->usleep(100);	/* High-voltage discharge time P10 */
	this->io->shift_bits_out(0x0000, 16);/* 16-bit payload (NOP) */
}
