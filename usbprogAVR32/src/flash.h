#ifndef __FLASH_H__
#define __FLASH_H__

#include <stdint.h>
#include "../include/basic.h"

#define FLASH_DATA_BIT_WIDTH			16
#define FLASH_BASE						0x00000000
#define FLASH_SIZE						0x00800000
#define FLASH_MAX_BANKS					1
#define FLASH_MAX_SECT					135

/* Manufacturer ID */
#define MANUFACTURER_AMD				0x0001
#define MANUFACTURER_ATMEL				0x001F
#define MANUFACTURER_SST				0x00BF
#define MANUFACTURER_INTEL				0x0089
#define MANUFACTURER_INTEL_ALT			0x00B0
#define MANUFACTURER_MICRON_ALT			0x002C


/* Device ID */
#define FLASH_DEVICE_AT49BV642D			0x001F01D6
#define FLASH_DEVICE_AT49BV642DT		0x001F01D2
#define FLASH_DEVICE_AT49BV6416			0x001F00D6
#define FLASH_DEVICE_AT49BV6416T		0x001F00D2
#define FLASH_DEVICE_AM29LV64XD			0x000122D7		// X = 0, 1, 2

#define SECTOR_EREASE_TIME				150				// ms, 0.15s
#define SECTOR_EREASE_TIME_OUT			6000			// ms, 6s


typedef struct {
	U8 count;							// number of sectors
	U32	size;							// sector size in bytes
} FLASH_SECT_T;

typedef struct {
	U32	id;								// combined device & manufacturer code
	U32	size;							// total bank size in bytes
	FLASH_SECT_T s1;					// setcor type 1
	FLASH_SECT_T s2;					// setcor type 2
} FLASH_INFO_T;


/* Status codes */
#define FLASH_STATUS_OK					0x00
#define FLASH_STATUS_INVALID_PARAM		0x01
#define FLASH_STATUS_SUBROUTINE_ERROR	0x02
#define FLASH_STATUS_NOT_SUPPORT		0x03
#define FLASH_STATUS_TIME_OUT			0x04
#define FLASH_STATUS_PROGRAM_ERROR		0x04
#define FLASH_STATUS_ERROR				0x05


void flash_init(void);
STATUS_T flash_null_read(uint8_t add_on);
STATUS_T flash_get_id(uint32_t *id);
STATUS_T flash_unlock_sector(uint32_t address);
STATUS_T flash_erase_sector(uint32_t address);
STATUS_T flash_program(uint32_t address, uint16_t data);

#endif  /* __FLASH_H__ */
