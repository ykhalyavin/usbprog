#include "cmd_flash.h"

#include "avr32_ebi.h"
#include "flash.h"


static void unlock_erase_sector(const CMD_STR* cmd, CMD_STR* ans);
static void erase_verify(const CMD_STR* cmd, CMD_STR* ans);
static void program(const CMD_STR* cmd, CMD_STR* ans);
static void read(const CMD_STR* cmd, CMD_STR* ans);


void cmd_flash_init(void)
{
	avr32_ebi_init();
}

void cmd_flash(const CMD_STR* cmd, CMD_STR* ans)
{
	uint8_t command = cmd->command;
	uint16_t   size = cmd->size;
	uint32_t id;

	ans->status = CMD_STATUS_OK;
	switch (command) {
		case CMD_FLASH_INIT:
			if (size != 0) {
				ans->status = CMD_STATUS_SIZE_ERROR;
			} else {
				if (flash_null_read(0) != FLASH_STATUS_OK)
					ans->status = CMD_STATUS_ERROR;
			}
			ans->size = 0;
			break;

		case CMD_FLASH_ID:
			if (size != 0) {
				ans->status = CMD_STATUS_SIZE_ERROR;
				ans->size = 0;
			} else {
				if (flash_get_id(&id) == FLASH_STATUS_OK) {
					CMD_SET_DWORD(ans, 0, id);
					ans->size = sizeof(id);
				} else {
					ans->status = CMD_STATUS_ERROR;
					ans->size = 0;
				}
			}
			break;

		case CMD_FLASH_ERASE_CHIP:
			ans->status = CMD_STATUS_NOT_IMPLEMENTED;
			ans->size = 0;
			break;

		case CMD_FLASH_ERASE_SECTOR:
		case CMD_FLASH_UNLOCK_SECTOR:
			unlock_erase_sector(cmd, ans);
			break;

		case CMD_FLASH_ERASE_VERIFY:
			erase_verify(cmd, ans);
			break;

		case CMD_FLASH_PROGRAM:
			program(cmd, ans);
			break;

		case CMD_FLASH_READ:
			read(cmd, ans);
			break;

		default:
			cmd_answer_error(cmd->command, CMD_STATUS_UNKOWN_COMMAND, ans);
	}
}

static void unlock_erase_sector(const CMD_STR* cmd, CMD_STR* ans)
{
	uint16_t num, size, i;
	uint32_t sect_addr;
	STATUS_T status;

	/* Get parameter */
	num = CMD_GET_WORD(cmd, 0);

	/* Check */
	size = sizeof(num) + sizeof(sect_addr) * num;
	if (cmd->size != size) {
		ans->status = CMD_STATUS_SIZE_ERROR;
		ans->size = 0;
		return;
	}

	/* Run */
	ans->size = 0;
	for (i=0; i<num; i++) {
		sect_addr = CMD_GET_DWORD(cmd, 2 + i*sizeof(sect_addr));
		if (cmd->command == CMD_FLASH_ERASE_SECTOR)
			status = flash_erase_sector(sect_addr);
		else	// Unlock the sector
			status = flash_unlock_sector(sect_addr);
		if (status != FLASH_STATUS_OK) {
			ans->status = CMD_STATUS_ERROR;
			ans->size = sizeof(sect_addr);
			CMD_SET_DWORD(ans, 0, sect_addr);
			return;
		}
	}
}

static void erase_verify(const CMD_STR* cmd, CMD_STR* ans)
{
	uint32_t addr, end_addr;
	uint16_t data;
	uint8_t  addon;
	STATUS_T status;

	/* Check */
	if (cmd->size != (sizeof(addr) + sizeof(end_addr))) {
		ans->status = CMD_STATUS_SIZE_ERROR;
		ans->size = 0;
		return;
	}

	/* Get parameter */
	addr = CMD_GET_DWORD(cmd, 0);	// Get start address
	end_addr = CMD_GET_DWORD(cmd, sizeof(addr));	// Get read number

	/* Run */
//20080121O, 改为块读模式以改善速度?
	ans->size = 0;
	for (addon=0; addr<end_addr; addr+=2, addon=1) {
		status = avr32_ebi_read16(addr, &data, addon);
		if ((status != AVR32_EBI_STATUS_OK) || (data != 0xFFFF)) {
			ans->status = CMD_STATUS_ERROR;
			ans->size = 4;
			CMD_SET_DWORD(ans, 0, addr);
			return;
		}
	}
}

static void program(const CMD_STR* cmd, CMD_STR* ans)
{
	uint16_t num, size, i;
	uint32_t addr;
	STATUS_T status;

	/* Get parameter */
	num = CMD_GET_WORD(cmd, 0);
	addr = CMD_GET_DWORD(cmd, 2);		// start address

	/* Check */
	i = sizeof(num) + sizeof(addr);
	size = i + num*2;
	if (cmd->size != size) {
		ans->status = CMD_STATUS_SIZE_ERROR;
		ans->size = 0;
		return;
	}

	/* Run */
	ans->size = 0;
	flash_null_read(0);
	for (; i<size; i+=2, addr+=2) {
		status = flash_program(addr, CMD_GET_WORD(cmd, i));
		if (status != FLASH_STATUS_OK) {
			ans->status = CMD_STATUS_ERROR;
			ans->size = sizeof(addr);
			CMD_SET_DWORD(ans, 0, addr);
			return;
		}
	}
}

static void read(const CMD_STR* cmd, CMD_STR* ans)
{
	uint32_t addr;
	uint16_t num;
	STATUS_T status;

	/* Check */
	#define LSIZE		(sizeof(addr) + sizeof(num))
	if (cmd->size != LSIZE) {
		ans->status = CMD_STATUS_SIZE_ERROR;
		ans->size = 0;
		return;
	}

	/* Get parameter */
	#define LMAX		((CMD_ANSWER_BUF_LENGTH - 4 - LSIZE) / 2)
	num = CMD_GET_WORD(cmd, 0);		// Get read number
	if (num > LMAX)
		num = LMAX;
	addr = CMD_GET_DWORD(cmd, 2);	// Get start address

	/* Run */
	status = avr32_ebi_blockread16(addr, num, (uint16_t*)&(ans->data[LSIZE]));
	if (status == AVR32_EBI_STATUS_OK) {
		CMD_SET_WORD(ans, 0, num);
		CMD_SET_DWORD(ans, 2, addr);
		ans->size = num*2 + LSIZE;
	} else {
		ans->status = CMD_STATUS_ERROR;
		ans->size = 0;
	}
	#undef LMAX
	#undef LSIZE
}
