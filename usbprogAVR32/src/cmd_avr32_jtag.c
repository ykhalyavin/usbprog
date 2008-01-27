#include "cmd_avr32_jtag.h"

#include "avr32.h"


void cmd_avr32_jtag_init(void)
{
}

void cmd_avr32_jtag(const CMD_STR* cmd, CMD_STR* ans)
{
	uint8_t command = cmd->command;
	uint16_t   size = cmd->size;
	uint32_t id;

	ans->status = CMD_STATUS_OK;
	ans->size = 0;
	switch (command) {
		case CMD_AVR32_JTAG_BYPASS:
			if (size != 0) {
				ans->status = CMD_STATUS_SIZE_ERROR;
			} else {
				if (avr32_send_ins(AVR32_JTAG_BYPASS) != CMD_STATUS_OK)
					ans->status = CMD_STATUS_ERROR;
			}
			break;

		case CMD_AVR32_JTAG_CLAMP:
			ans->status = CMD_STATUS_NOT_IMPLEMENTED;
			break;

		case CMD_AVR32_JTAG_EXTEST:
			ans->status = CMD_STATUS_NOT_IMPLEMENTED;
			break;

		case CMD_AVR32_JTAG_IDCODE:
			if (size != 0) {
				ans->status = CMD_STATUS_SIZE_ERROR;
			} else {
				if (avr32_get_idcode(&id) == CMD_STATUS_OK) {
					ans->size = 4;
					CMD_SET_DWORD(ans, 0, id);
				} else {
					ans->status = CMD_STATUS_ERROR;
				}
			}
			break;

		case CMD_AVR32_JTAG_INTEST:
			ans->status = CMD_STATUS_NOT_IMPLEMENTED;
			break;

		case CMD_AVR32_JTAG_SAMPLE:
			ans->status = CMD_STATUS_NOT_IMPLEMENTED;
			break;

		case CMD_AVR32_JTAG_AVR_RESET:
			if (size != 1) {
				ans->status = CMD_STATUS_SIZE_ERROR;
			} else {
				if (avr32_avr_reset(cmd->data[0]) != CMD_STATUS_OK)
					ans->status = CMD_STATUS_ERROR;
			}
			break;

		case CMD_AVR32_JTAG_CANCEL_ACCESS:
			ans->status = CMD_STATUS_NOT_IMPLEMENTED;
			break;

		case CMD_AVR32_JTAG_MEM_WORD_ACCESS:
			ans->status = CMD_STATUS_NOT_IMPLEMENTED;
			break;

		case CMD_AVR32_JTAG_MEM_BLOCK_ACCESS:
			ans->status = CMD_STATUS_NOT_IMPLEMENTED;
			break;

		case CMD_AVR32_JTAG_NEXUS_ACCESS:
			ans->status = CMD_STATUS_NOT_IMPLEMENTED;
			break;

		case CMD_AVR32_JTAG_SYNC:
			ans->status = CMD_STATUS_NOT_IMPLEMENTED;
			break;

		case CMD_AVR32_JTAG_PRIVATE2:
		case CMD_AVR32_JTAG_PRIVATE3:
		case CMD_AVR32_JTAG_PRIVATE4:
		case CMD_AVR32_JTAG_PRIVATE9:
			ans->status = CMD_STATUS_NOT_IMPLEMENTED;
			break;
/*
		case :
			ans->status = CMD_STATUS_NOT_IMPLEMENTED;
			ans->size = 0;
			break;
*/
		default:
			cmd_answer_error(cmd->command, CMD_STATUS_UNKOWN_COMMAND, ans);
	}
}
