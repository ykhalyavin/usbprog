#ifndef __AVR32_H__
#define __AVR32_H__

#include <stdint.h>
#include "../include/basic.h"

/* Instruction Register info */
#define AVR32_IR_SIZE				(5)				// bits
#define AVR32_IR_MASK   			(0x1F)

/* Instructions */
#define AVR32_JTAG_BYPASS			(0x1F)
#define AVR32_JTAG_CLAMP			(0x06)
#define AVR32_JTAG_EXTEST			(0x03)
#define AVR32_JTAG_IDCODE			(0x01)
#define AVR32_JTAG_INTEST			(0x04)
#define AVR32_JTAG_SAMPLE			(0x02)

#define AVR32_JTAG_AVR_RESET		(0x0C)
#define AVR32_JTAG_CANCEL_ACCESS	(0x13)
#define AVR32_JTAG_MEM_WORD_ACCESS	(0x11)
#define AVR32_JTAG_MEM_BLOCK_ACCESS	(0x12)
#define AVR32_JTAG_NEXUS_ACCESS		(0x10)
#define AVR32_JTAG_SYNC				(0x17)
#define AVR32_JTAG_PRIVATE2			(0x19)
#define AVR32_JTAG_PRIVATE3			(0x1A)
#define AVR32_JTAG_PRIVATE4			(0x1B)
#define AVR32_JTAG_PRIVATE9			(0x18)

/* Instruction Register shift out value and bits */
#define AVR32_INSTRUCTION_CAPTURE	(0x01)			// 0_0001
#define AVR32_IR_PROTECT_BIT		(0x10)			// 1_0000
#define AVR32_IR_ERROR_BIT			(0x08)			// 0_1000
#define AVR32_IR_BUSY_BIT			(0x04)			// 0_0100
#define AVR32_IR_CAPTURE_BIT		(0x03)			// 0_0011

/* Reset instruction data bits */
#define AVR32_JTAG_AVR_RESET_CPU	(0x01)
#define AVR32_JTAG_AVR_RESET_ICACHE	(0x02)
#define AVR32_JTAG_AVR_RESET_DCACHE	(0x04)
#define AVR32_JTAG_AVR_RESET_APP	(0x08)
#define AVR32_JTAG_AVR_RESET_OCD	(0x10)
#define AVR32_JTAG_AVR_RESET_ALL	(0x1F)
#define AVR32_JTAG_AVR_RESET_NONE	(0x00)
    
/* AVR32 CPU types */
#define AVR32_UNKNOWN				(0xFF)
#define AVR32_AP7000				(0x01)
#define AVR32_AP7001				(0x02)

/* SAB slaves */
#define AVR32_SAB_OCD				(0x01)
#define AVR32_SAB_HSB1				(0x04)
#define AVR32_SAB_HSB2				(0x05)

/* OCD registers' addresses */
#define AVR32_OCD_DID				(0x00000000)	// Device ID Register
#define AVR32_OCD_DC				(0x00000008)	// Development Control Register
#define AVR32_OCD_DS				(0x00000010)	// Development Status register
#define AVR32_OCD_RWCS				(0x0000001C)	// Read/Write Control/Status
#define AVR32_OCD_RWA				(0x00000024)	// Read/Write Access Address
#define AVR32_OCD_RWD				(0x00000028)	// Read/Write Access Data
#define AVR32_OCD_NXCFG				(0x00000100)	// Nexus Configuration Register
#define AVR32_OCD_DINST				(0x00000104)	// Debug Instruction Register
#define AVR32_OCD_DPC				(0x00000108)	// Debug Program Counter
#define AVR32_OCD_					(0x00000000)	// 

/* Status codes */
#define AVR32_STATUS_OK						0x00
#define AVR32_STATUS_INVALID_PARAM			0x01
#define AVR32_STATUS_INVALID_TAP_STATE		0x02
#define AVR32_STATUS_SHIFT_IR_ERROR			0x03
#define AVR32_STATUS_SHIFT_DR_ERROR			0x04
#define AVR32_STATUS_INVALID_IR_CAPTURE		0x05
#define AVR32_STATUS_IR_BUSY				0x06
#define AVR32_STATUS_IR_ERROR				0x07
#define AVR32_STATUS_IR_PROTECT				0x08
#define AVR32_STATUS_INVALID_IDCODE			0x09
#define AVR32_STATUS_ERROR					0x0A


void avr32_init(void);
STATUS_T avr32_send_ins(uint8_t ins);
STATUS_T avr32_get_idcode(uint32_t* id);
uint8_t  avr32_cpu_type(uint32_t id);
STATUS_T avr32_mem_access(uint8_t read, uint8_t sab_slave, uint32_t address, uint32_t *data, uint8_t *eb);
//uint8_t avr32_detect(uint8_t* cpu_type);
STATUS_T avr32_avr_reset(uint8_t domain);
STATUS_T avr32_extest(const BUF_T *bsr_out, BUF_T *bsr_in, uint8_t add_on);

#endif  /* __AVR32_H__ */
