#ifndef __CMD_FLASH_H__
#define __CMD_FLASH_H__

#include "../include/basic.h"

/* Flash chip IDs */
#define FLASH_DEVICE_AT49BV642D			0x001F01D6
#define FLASH_DEVICE_AT49BV642DT		0x001F01D2
#define FLASH_DEVICE_AT49BV6416			0x001F00D6
#define FLASH_DEVICE_AT49BV6416T		0x001F00D2
#define FLASH_DEVICE_AM29LV64XD			0x000122D7		// X = 0, 1, 2

/* Sector Group is a collection of the sectors which have the same structure
 * and in continuous addresses
 */
typedef struct {
	int size;			// sector size in bytes
	int num;			// number of sector
	int addr;			// start address of sectors' group
} SECTOR_GROUP_STR;

#define MAX_SECTOR_GROUPS				4
typedef struct {
	const char* type;							// flash chip type
	U32 id;										// idcode of the flash chip
	int size;									// size in bytes
	int sect_num;								// number of sectors
	int group_num;								// number of sectors groups, max is MAX_SECTOR_GROUPS
	SECTOR_GROUP_STR group[MAX_SECTOR_GROUPS];	// infomation of each sector group
} FLASH_STR;

int cmd_flash_init(void);
FLASH_STR *cmd_flash_index(U32 id);

int cmd_flash_id(U32 *id);
int cmd_flash_erase_chip(void);
int cmd_flash_unlock_sector(const U32 *sect_addrs, U32 num);
int cmd_flash_erase_sector(const U32 *sect_addrs, U32 num);
int cmd_flash_erase_verify(U32 start_addr, U32 end_addr);
int cmd_flash_program(U32 start_addr, U32 num, const U16 *data);
int cmd_flash_read(U32 start_addr, U32 num, U16 *data);

#endif /* __CMD_FLASH_H__ */

