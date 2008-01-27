#include "cmd_avr32_ebi.h"


void cmd_avr32_ebi_init(void)
{
}

void cmd_avr32_ebi(const CMD_STR* cmd, CMD_STR* ans)
{
	//uint8_t command = cmd->command;
	//uint16_t   size = cmd->size;
	
	ans->status = CMD_STATUS_NOT_IMPLEMENTED;
	ans->size = 0;
}
