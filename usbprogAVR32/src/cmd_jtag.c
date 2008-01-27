#include "cmd_jtag.h"

//#include "basic.h"
#include "jtag_tap.h"


static void shift_instruction(const CMD_STR* cmd, CMD_STR* ans);
static void shift_data(const CMD_STR* cmd, CMD_STR* ans);


void cmd_jtag_init(void)
{
	jtag_tap_init();
}

void cmd_jtag(const CMD_STR* cmd, CMD_STR* ans)
{
	uint8_t command = cmd->command;
	uint16_t   size = cmd->size;
	STATUS_T status;
	
	ans->status = CMD_STATUS_OK;
	switch (command) {
		case CMD_JTAG_INSTRUCTION:
			shift_instruction(cmd, ans);
			break;
		
		case CMD_JTAG_DATA:
			shift_data(cmd, ans);
			break;

		case CMD_JTAG_DETECT_IR:
		case CMD_JTAG_DETECT_DR:
			if (size != 2) {
				ans->status = CMD_STATUS_SIZE_ERROR;
				ans->size = 0;
			} else {
				size = CMD_GET_WORD(cmd, 0);	// get parameter max_size
				if (command == CMD_JTAG_DETECT_IR)
					status = jtag_tap_detect_ireg_size(size, &size);
				else
					status = jtag_tap_detect_dreg_size(size, &size);
				if (status == TAP_STATUS_OK) {
					CMD_SET_WORD(ans, 0, size);
					ans->size = sizeof(size);
				} else {
					ans->status = CMD_STATUS_ERROR;
					ans->size = 0;
				}
			}
			break;

		case CMD_JTAG_DETECT:
			ans->status = CMD_STATUS_NOT_IMPLEMENTED;
			ans->size = 0;
			break;

		default:
			cmd_answer_error(cmd->command, CMD_STATUS_UNKOWN_COMMAND, ans);
	}
}

static void shift_instruction(const CMD_STR* cmd, CMD_STR* ans)
{
	uint16_t bit_size, buf_size, size;
	uint8_t  *instruction;
	STATUS_T status;

	/* Get parameter */
	bit_size = CMD_GET_WORD(cmd, 0);
	buf_size = bit_size/8 + (bit_size%8 == 0 ? 0 : 1);

	/* Check */
	if (bit_size < 2) {
		ans->status = CMD_STATUS_INVALID_PARAM;
		ans->size = 0;
		return;
	}
	size = sizeof(bit_size) + buf_size;
	if (cmd->size != size) {
		ans->status = CMD_STATUS_SIZE_ERROR;
		ans->size = 0;
		return;
	}

	/* Run */
	instruction = (uint8_t*)(cmd) + 6;		// point to cmd->data[2]
	status = jtag_tap_shift_ir(instruction, bit_size);
	if (status == TAP_STATUS_OK) {
		CMD_SET_WORD(ans, 0, bit_size);
		ans->size = size;
	} else {
		ans->status = CMD_STATUS_ERROR;
		ans->size = 0;
	}
}

static void shift_data(const CMD_STR* cmd, CMD_STR* ans)
{
	uint16_t bit_size, buf_size, size;
	uint8_t  i, o, *out_buf, *in_buf;
	STATUS_T status;

	/* Get parameter */
	bit_size = CMD_GET_WORD(cmd, 0);
	o = cmd->data[2];
	i = o & 0x04;
	o &= 0x02;
	buf_size = bit_size/8 + (bit_size%8 == 0 ? 0 : 1);
	
	/* Check */
	if ((i == 0) && (o == 0)) {
		ans->status = CMD_STATUS_INVALID_PARAM;
		ans->size = 0;
		return;
	}
	size = sizeof(bit_size) + sizeof(o) + (o == 0 ? 0 : buf_size);
	if (cmd->size != size) {
		ans->status = CMD_STATUS_SIZE_ERROR;
		ans->size = 0;
		return;
	}
	
	/* Run */
	out_buf = (o == 0 ? NULL : ((uint8_t*)&(cmd->data[3])));  // point to cmd->data[3]
	in_buf  = (i == 0 ? NULL : ((uint8_t*)&(ans->data[3])));  // point to ans->data[3]
	status = jtag_tap_shift_dr(out_buf, in_buf, bit_size);
	if (status == TAP_STATUS_OK) {
		ans->data[0] = cmd->data[0];
		ans->data[1] = cmd->data[1];
		ans->data[2] = cmd->data[2];
		ans->size = 3 + (i == 0 ? 0 : buf_size);
	} else {
		ans->status = CMD_STATUS_ERROR;
		ans->size = 0;
	}
}

