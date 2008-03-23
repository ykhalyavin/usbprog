// This file contains some convienience declarations of ASM Commands often used by OCD

#ifndef __AVR_ASM_H__
#define __AVR_ASM_H__

#define AVR_LDI(reg,val) (uint16_t)(0xE000 | (((val) & 0xF0) << 4) | (((reg) & 0x0F)<<4) | ((val) & 0x0F))
#define AVR_OUT(iomem,reg) (uint16_t)(0xB800 | (((iomem) & 0x30) << 5) | (((reg) & 0x1F) << 4) | ((iomem) & 0x0F))
#define AVR_IN(reg,iomem) (uint16_t)(0xB000 | (((iomem) & 0x30) << 5) | (((reg) & 0x1F) << 4) | ((iomem) & 0x0F))
#define AVR_ORI(reg,mask) (uint16_t)(0x6000 | (((mask) & 0xF0) << 4) | (((reg) & 0x0F) << 4) | ((mask) & 0xF))

#define AVR_LDZ_PostInc(reg) (uint16_t)(0x9001 | (((reg) & 0x1F) << 4))

#define AVR_STZ_PostInc(reg) (uint16_t)(0x9201 | (((reg) & 0x1F) << 4))

#define AVR_ADIW(rid,val) (uint16_t)(0x9600 | (((rid) & 0x3) << 4) | (((val) & 0x30) << 2) | ((val) & 0x0F))

#define AVR_LPM_PostInc(reg) (uint16_t)(0x9005 | (((reg) & 0x1F) << 4))

#define AVR_NOP() (uint16_t)0


#endif
