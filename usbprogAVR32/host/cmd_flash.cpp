#include "cmd_flash.h"

#include "../include/commands.h"
#include "usbprog.h"
#include "cmd.h"
#include <string.h>

#define FLASH_DEVICE_AT49BV642D			0x001F01D6
#define FLASH_DEVICE_AT49BV642DT		0x001F01D2
#define FLASH_DEVICE_AT49BV6416			0x001F00D6
#define FLASH_DEVICE_AT49BV6416T		0x001F00D2
#define FLASH_DEVICE_AM29LV64XD			0x000122D7		// X = 0, 1


static FLASH_STR g_flashs[] = {
	{	"AT49BV642D", 0x001F01D6, 8388608, 135,
		2, { {8192L, 8L, 0L}, {65536L, 127L, 0x10000}, {}, {} }
	},
	{	"AT49BV642DT", 0x001F01D2, 8388608, 135,
		2, { {65536L, 127L, 0L}, {8192L, 8L, 0x7F0000}, {}, {} }
	},
	{	"AT49BV6416", 0x001F00D6, 8388608, 135,
		2, { {8192L, 8L, 0L}, {65536L, 127L, 0x10000}, {}, {} }
	},
	{	"AT49BV6416T", 0x001F00D2, 8388608, 135,
		2, { {65536L, 127L, 0L}, {8192L, 8L, 0x7F0000}, {}, {} }
	},
	{   NULL, 0, 0, 0,
		0, { {}, {}, {}, {} }		/* This is the last array element */
	},
	{	"AM29LV64xD", 0x000122D7, 8388608, 128,
		1, { {65536L, 128L, 0L}, {}, {}, {} }
	},
};


int cmd_flash_init(void)
{
	return CMD_STATUS_OK;
}

FLASH_STR *cmd_flash_index(U32 id)
{
	FLASH_STR *p;
	int i = 0;

	while (1) {
		p = &g_flashs[i++];
		if (p->type == NULL) {
			/* This is the last array element */
			return NULL;
		}
		if (p->id == id)
			return p;
	}
}

int cmd_flash_id(U32 *id)
{
	CMD_STR cmd;
	int r;

	cmd.command = CMD_FLASH_ID;
	cmd.size = 0;
	r = cmd_execute(&cmd);
	*id = (r==0 ? CMD_GET_DWORD(&cmd, 0) : 0UL);

	return r;
}

int cmd_flash_erase_chip(void)
{
/*
	CMD_STR cmd;
	cmd.command = CMD_FLASH_EREASE_CHIP;
	cmd.size = 0;
	r = cmd_execute(&cmd);
	return r;
*/
	return CMD_STATUS_NOT_IMPLEMENTED;
}

int cmd_flash_erase_verify(U32 start_addr, U32 end_addr)
{
	CMD_STR cmd;
	U32 timeout;

	if (end_addr < start_addr)
		return CMD_STATUS_INVALID_PARAM;

	cmd.command = CMD_FLASH_ERASE_VERIFY;
	CMD_SET_DWORD(&cmd, 0, start_addr);
	CMD_SET_DWORD(&cmd, sizeof(start_addr), end_addr);
	cmd.size = sizeof(start_addr) + sizeof(end_addr);
	timeout = (end_addr - start_addr) * 2;
	return cmd_execute(&cmd, USB_PACKAGE_SIZE, 0, timeout);
}

/* Local definations for function 'cmd_flash_read':
 * CF_PARA_SIZE: size of parameter 'num' and 'start_addr'
 * CF_MAX_SIZE:  max data byte size per CMD package
 * CF_MAX_NUM:   max 16-bit data size per CMD package
 * CF_TIMEOUT:	 time out for receiving data
 */
#define CF_PACKAGE_SIZE	(USB_PACKAGE_SIZE)	// (CMD_ANSWER_BUF_LENGTH)
#define CF_PARA_SIZE	(sizeof(U16) + sizeof(U32))
#define CF_MAX_SIZE    	(CF_PACKAGE_SIZE - CMD_HEAD_SIZE - CF_PARA_SIZE)
#define CF_MAX_NUM		(CF_MAX_SIZE / 2)
#define CF_TIMEOUT		(4 * CF_MAX_NUM)

int cmd_flash_read(U32 start_addr, U32 num, U16 *data)
{
	CMD_STR cmd;
	U32 addr = start_addr & 0xFFFFFFFE;		// Align to 16-bit word
	U32 i, len;
	int r;

	cmd.command = CMD_FLASH_READ;
	for (i=0; i < num; i+=CF_MAX_NUM) {
		cmd.status = 0x00;
		len = num - i;
		if (len > CF_MAX_NUM)
			len = CF_MAX_NUM;
		CMD_SET_WORD(&cmd, 0, (U16)len);
		CMD_SET_DWORD(&cmd, 2, addr);
		cmd.size = CF_PARA_SIZE;

		r = cmd_execute(&cmd, CF_PACKAGE_SIZE, 0, CF_TIMEOUT);
		if (r != CMD_STATUS_OK)
			return r;
		len *= sizeof(data[0]);
		memcpy( (void*)&data[i],
				(void*)&(cmd.data[CF_PARA_SIZE]),
				len );
		//addr = CMD_GET_DWORD(&cmd, 2);
		addr += len;
	}

	return CMD_STATUS_OK;
}
#undef CF_PACKAGE_SIZE
#undef CF_PARA_SIZE
#undef CF_MAX_SIZE
#undef CF_MAX_NUM
#undef CF_TIMEOUT

/* Local definations for function 'cmd_flash_read':
 * CF_PARA_SIZE: size of parameter 'num' and 'start_addr'
 * CF_MAX_SIZE:  max data byte size per CMD package
 * CF_MAX_NUM:   max 16-bit data size per CMD package
 * CF_TIMEOUT:	 time out for receiving data
 */
#define CF_PARA_SIZE	(sizeof(U16) + sizeof(U32))
#define CF_MAX_SIZE    	(USB_PACKAGE_SIZE - CMD_HEAD_SIZE - CF_PARA_SIZE)
#define CF_MAX_NUM		(CF_MAX_SIZE / 2)
#define CF_TIMEOUT		(50 * CF_MAX_NUM)

int cmd_flash_program(U32 start_addr, U32 num, const U16 *data)
{
	CMD_STR cmd;
	U32 addr = start_addr & 0xFFFFFFFE;		// Align to 16-bit word
	U32 i, len;
	int r;

	cmd.command = CMD_FLASH_PROGRAM;
	for (i=0; i < num; i+=CF_MAX_NUM) {
		memset((void*)cmd.data, 0, USB_PACKAGE_SIZE);
		cmd.status = 0x00;
		len = num - i;
		if (len > CF_MAX_NUM)
			len = CF_MAX_NUM;
		CMD_SET_WORD(&cmd, 0, (U16)len);
		CMD_SET_DWORD(&cmd, 2, addr);
		len *= sizeof(data[0]);
		memcpy( (void*)&(cmd.data[CF_PARA_SIZE]),
				(void*)&data[i],
				len );
		cmd.size = CF_PARA_SIZE + len;

		r = cmd_execute(&cmd, USB_PACKAGE_SIZE, 0, CF_TIMEOUT);
		if (r != CMD_STATUS_OK)
			return r;
		addr += len;
	}

	return CMD_STATUS_OK;
}
#undef CF_PARA_SIZE
#undef CF_MAX_SIZE
#undef CF_MAX_NUM
#undef CF_TIMEOUT

/* Local definations for function 'cmd_flash_read':
 * CF_PARA_SIZE: size of parameter 'num'
 * CF_MAX_SIZE:  max data byte size per CMD package
 * CF_MAX_NUM:   max 32-bit data size per CMD package
 * CF_TIMEOUT:	 time out for receiving data
 */
#define CF_PARA_SIZE	(sizeof(U16))
#define CF_MAX_SIZE    	(USB_PACKAGE_SIZE - CMD_HEAD_SIZE - CF_PARA_SIZE)
#define CF_MAX_NUM		(CF_MAX_SIZE / 4)
#define CF_TIMEOUT		(500 * CF_MAX_NUM)

int flash_unlock_erase(const U32 *sect_addrs, U32 num, int erase)
{
	CMD_STR cmd;
	U32 i, j, len;
	int r;

	cmd.command = (erase==0 ? CMD_FLASH_UNLOCK_SECTOR : CMD_FLASH_ERASE_SECTOR);
	for (i=0; i<num; i+=CF_MAX_NUM) {
		cmd.status = 0x00;
		len = num - i;
		if (len > CF_MAX_NUM)
			len = CF_MAX_NUM;
		CMD_SET_WORD(&cmd, 0, (U16)len);
		r = CF_PARA_SIZE;
		for (j=0; j<len; j++, r+=sizeof(sect_addrs[0]))
			CMD_SET_DWORD(&cmd, r, sect_addrs[j]);
		cmd.size = r;

		r = cmd_execute(&cmd, USB_PACKAGE_SIZE, 0, CF_TIMEOUT);
		if (r != CMD_STATUS_OK)
			return r;
	}

	return CMD_STATUS_OK;
}
#undef CF_PARA_SIZE
#undef CF_MAX_SIZE
#undef CF_MAX_NUM
#undef CF_TIMEOUT

int cmd_flash_unlock_sector(const U32 *sect_addrs, U32 num)
{
	return flash_unlock_erase(sect_addrs, num, 0);
}

int cmd_flash_erase_sector(const U32 *sect_addrs, U32 num)
{
	return flash_unlock_erase(sect_addrs, num, 1);
}


