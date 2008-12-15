
/*-------------------------------------------------------------------------
 * AVR_ASM.H
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
 */


// This file contains some convienience declarations of ASM Commands often used by OCD

#ifndef __AVR_ASM_H__
#define __AVR_ASM_H__

#define AVR_LDI(reg,val) (uint16_t)(0xE000 | (((val) & 0xF0) << 4) | (((reg) & 0x0F)<<4) | ((val) & 0x0F))
#define AVR_OUT(iomem,reg) (uint16_t)(0xB800 | (((iomem) & 0x30) << 5) | (((reg) & 0x1F) << 4) | ((iomem) & 0x0F))
#define AVR_IN(reg,iomem) (uint16_t)(0xB000 | (((iomem) & 0x30) << 5) | (((reg) & 0x1F) << 4) | ((iomem) & 0x0F))
#define AVR_ORI(reg,mask) (uint16_t)(0x6000 | (((mask) & 0xF0) << 4) | (((reg) & 0x0F) << 4) | ((mask) & 0xF))

#define AVR_LDZ_PostInc(reg) (uint16_t)(0x9001 | (((reg) & 0x1F) << 4))
#define AVR_LDZ(reg) (uint16_t)(0x8000 | (((reg) & 0x1F) << 4))

#define AVR_STZ_PostInc(reg) (uint16_t)(0x9201 | (((reg) & 0x1F) << 4))

#define AVR_STY(reg) (uint16_t)(0x8208 | (((reg) & 0x1F) << 4))

#define AVR_LDY(reg) (uint16_t)(0x8008 | (((reg) & 0x1F) << 4))

#define AVR_ADIW(rid,val) (uint16_t)(0x9600 | (((rid) & 0x3) << 4) | (((val) & 0x30) << 2) | ((val) & 0x0F))

/* LPM seems not to work over Instruction interface. Maybe there is a starting of the processor needed like
 * using SPM. But there exists special instructions to access the flash over the JTAG interface
 * for LPM is not needed
#define AVR_LPM_PostInc(reg) (uint16_t)(0x9005 | (((reg) & 0x1F) << 4))
#define AVR_LPM(reg) (uint16_t)(0x9004 | (((reg) & 0x1F) << 4))
#define AVR_LPM() (uint16_t)0x95C8
*/

#define AVR_SPM() (uint16_t)0x95E8

#define AVR_NOP() (uint16_t)0

#define AVR_IJMP() (uint16_t)0x9409


#endif
