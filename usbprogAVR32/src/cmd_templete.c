#include "cmd_tap.h"

#include "cmd.h"

void cmd_tap_init(void)
{
}

uint16_t cmd_tap(CMD_STR cmd, uint8_t *answer_buf)
{
	CMD_STR* ans = (CMD_STR*)answer_buf;
	uint16_t size;
	
	switch (cmd.command) {
		case :
			break;
		case :
			break;
		
		default:
			size = cmd_unknown(cmd.command);
	}
	
	return size;
}
