#ifndef __COMMANDS_H__
#define __COMMANDS_H__


#include "basic.h"

#define CMD_ANSWER_BUF_LENGTH				(320)

#define CMD_GROUP_MASK						(0xF0)
#define CMD_SUB_MASK						(0x0F)

#define CMD_GROUP_COMM						(0x00)
#define CMD_COMM_FIRMWARE_VERSION			(0x00)
#define CMD_COMM_INIT						(0x01)
#define CMD_COMM_LED_RED					(0x02)


#define CMD_GROUP_TAP						(0x10)
#define CMD_TAP_SET_SRST					(0x10)
#define CMD_TAP_RESET						(0x11)
#define CMD_TAP_STATE						(0x12)
#define CMD_TAP_TRANS						(0x13)
#define CMD_TAP_SHIFT						(0x14)

#define CMD_GROUP_JTAG						(0x20)
#define CMD_JTAG_INSTRUCTION				(0x20)
#define CMD_JTAG_DATA						(0x21)
#define CMD_JTAG_DETECT_IR					(0x22)
#define CMD_JTAG_DETECT_DR					(0x23)
#define CMD_JTAG_DETECT						(0x24)

#define CMD_GROUP_AVR32_JTAG1				(0x30)
#define CMD_AVR32_JTAG_BYPASS				(0x30)
#define CMD_AVR32_JTAG_CLAMP				(0x31)
#define CMD_AVR32_JTAG_EXTEST				(0x32)
#define CMD_AVR32_JTAG_IDCODE				(0x33)
#define CMD_AVR32_JTAG_INTEST				(0x34)
#define CMD_AVR32_JTAG_SAMPLE				(0x35)
#define CMD_AVR32_JTAG_AVR_RESET			(0x36)
#define CMD_AVR32_JTAG_CANCEL_ACCESS		(0x37)
#define CMD_AVR32_JTAG_MEM_WORD_ACCESS		(0x38)
#define CMD_AVR32_JTAG_MEM_BLOCK_ACCESS		(0x39)
#define CMD_AVR32_JTAG_NEXUS_ACCESS			(0x3A)
#define CMD_AVR32_JTAG_SYNC					(0x3B)
#define CMD_AVR32_JTAG_PRIVATE2				(0x3C)
#define CMD_AVR32_JTAG_PRIVATE3				(0x3D)
#define CMD_AVR32_JTAG_PRIVATE4				(0x3E)
#define CMD_AVR32_JTAG_PRIVATE9				(0x3F)

#define CMD_GROUP_AVR32_JTAG2				(0x40)

#define CMD_GROUP_AVR32_BSR					(0x50)
#define CMD_AVR32_BSR						(0x50)

#define CMD_GROUP_AVR32_EBI					(0x60)
#define CMD_AVR32_EBI_INIT					(0x60)
#define CMD_AVR32_EBI_READ16				(0x61)
#define CMD_AVR32_EBI_BLOCKREAD16			(0x62)
#define CMD_AVR32_EBI_WRITE16				(0x63)
#define CMD_AVR32_EBI_LED					(0x64)
#define CMD_AVR32_EBI_J15					(0x65)

#define CMD_GROUP_FLASH						(0x70)
#define CMD_FLASH_INIT						(0x70)
#define CMD_FLASH_ID						(0x71)
#define CMD_FLASH_ERASE_CHIP				(0x72)
#define CMD_FLASH_ERASE_SECTOR				(0x73)
#define CMD_FLASH_ERASE_VERIFY				(0x74)
#define CMD_FLASH_UNLOCK_SECTOR				(0x75)
#define CMD_FLASH_PROGRAM					(0x76)
#define CMD_FLASH_READ						(0x77)


#define CMD_STATUS_OK						((char)0)
#define CMD_STATUS_UNKOWN_COMMAND			((char)1)
#define CMD_STATUS_NOT_IMPLEMENTED			((char)2)
#define CMD_STATUS_SIZE_ERROR				((char)3)
#define CMD_STATUS_INVALID_PARAM			((char)4)
#define CMD_STATUS_ERROR					((char)5)


/*
Command/answer struct in buffer:
	byte[0]         U8  command code
	byte[1]         U8  0x00 / command executed status
	byte[3:2]       U16 rest data size of the package
	byte[4..size+3]     data/parameters
Note: size, data and parameters are all in little-endian format
*/
#define CMD_HEAD_SIZE 						(4)		// sizeof command, status and size
typedef struct {
	unsigned char  command;
	unsigned char  status;
	unsigned short size;
	unsigned char  data[CMD_ANSWER_BUF_LENGTH - CMD_HEAD_SIZE];
} CMD_STR;


#define CMD_GET_WORD(cmd, index)										\
	(*((unsigned short*)&((cmd)->data[index])))

#define CMD_GET_DWORD(cmd, index)										\
	(*((unsigned long*)&((cmd)->data[index])))

#define CMD_SET_WORD(cmd, index, word)									\
	{ *((unsigned short*)&((cmd)->data[index])) = (word); }

#define CMD_SET_DWORD(cmd, index, dword)								\
	{ *((unsigned long*)&((cmd)->data[index])) = (dword); }


#endif  /* __COMMANDS_H__ */
