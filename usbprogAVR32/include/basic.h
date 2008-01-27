#ifndef __BASIC_H__
#define __BASIC_H__

#define CHECK_PARAM				1

typedef unsigned char  U8;
typedef unsigned short U16;
typedef unsigned long  U32;

#ifndef FALSE
typedef unsigned char BOOLEAN;
#define FALSE					(BOOLEAN)(0)
#define TRUE					(BOOLEAN)(1)
#endif

typedef char STATUS_T;
typedef unsigned char BUF_T;
#define BUF_BIT_WIDTH			(sizeof(BUF_T)*8)


#define MAX_PARTS_NUM			(16)
#define MAX_PART_IR_SIZE		(32)
#define MAX_IR_CHAIN_SIZE		(MAX_PARTS_NUM * MAX_PART_IR_SIZE)

#define LO						0
#define HI						1

#define IO_OUT					0
#define IO_IN					1

#define NO						0
#define YES						1

#define OFF						0
#define ON						1

#ifndef NULL
#define NULL						((void*)0)
#endif


/* TAP's states */
#define NUM_OF_STATES				0x10		//16 TAP States
#define TAPSTATE_UNDEFINED			0xF0
#define TAPSTATE_MASK				0x0F

#define TAPSTATE_TSTLOG_RST			0xF
#define TAPSTATE_RUNTEST_IDLE		0xC
#define TAPSTATE_SELECT_DR_SCAN		0x7
#define TAPSTATE_CAPTURE_DR			0x6
#define TAPSTATE_SHIFT_DR			0x2
#define TAPSTATE_EXIT1_DR			0x1
#define TAPSTATE_PAUSE_DR			0x3
#define TAPSTATE_EXIT2_DR			0x0
#define TAPSTATE_UPDATE_DR			0x5
#define TAPSTATE_SELECT_IR_SCAN		0x4
#define TAPSTATE_CAPTURE_IR			0xE
#define TAPSTATE_SHIFT_IR			0xA
#define TAPSTATE_EXIT1_IR			0x9
#define TAPSTATE_PAUSE_IR			0xB
#define TAPSTATE_EXIT2_IR			0x8
#define TAPSTATE_UPDATE_IR			0xD


/* IDCODE information */
#define IDCODE_LSB_MASK				0x00000001
#define IDCODE_MANUFACTUER_MASK 	0x00000FFE
#define IDCODE_PARTNUMBER_MASK   	0x0FFFF000
#define IDCODE_REVISION_MASK 		0xF0000000

#define IDCODE_LSB(id)				((id) & IDCODE_LSB_MASK)
#define IDCODE_MANUFACTUER(id)	 	(((id) & IDCODE_MANUFACTUER_MASK) >> 1)
#define IDCODE_PARTNUMBER(id)	   	(((id) & IDCODE_PARTNUMBER_MASK) >> 12)
#define IDCODE_REVISION(id)	 		(((id) & IDCODE_REVISION_MASK) >> 28)

#define ID_AP7000					0x01E8203E	// 0000_0001_1110_1000_0010_0000_0011_1110
#define ID_AP7001					0x01E8203E	// 0000_0001_1110_1000_0010_0000_0011_1110


/* Returned status code of routines */
#define STATUS_OK				(STATUS_T)(0)
#define STATUS_INVALID_PARAM	(STATUS_T)(1)
#define STATUS_SUBROUTINE_ERROR	(STATUS_T)(2)
#define STATUS_ERROR			(STATUS_T)(3)


/* Converting binary in number format into byte
 * Examples: BIN(01010101) -> 0x55; BIN(10101010) -> 0xAA
 */
#define LONG_TO_BIN(n)				\
(									\
	(((n) >> 21) & 0x80) |			\
	(((n) >> 18) & 0x40) |			\
	(((n) >> 15) & 0x20) |			\
	(((n) >> 12) & 0x10) |			\
	(((n) >> 9)  & 0x08) |			\
	(((n) >> 6)  & 0x04) |			\
	(((n) >> 3)  & 0x02) |			\
	((n) & 0x01)					\
)
#define BIN(n)						LONG_TO_BIN(0x##n##l)


#define BIT(n)					(1 << (n))

#endif  /* __BASIC_H__ */
