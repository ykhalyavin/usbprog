#include "cmd_tap.h"

#include "jtag_tap.h"


static void tap_trans(const CMD_STR* cmd, CMD_STR* ans);
static void tap_shift(const CMD_STR* cmd, CMD_STR* ans);

void cmd_tap_init(void)
{
	jtag_tap_init();
}

void cmd_tap(const CMD_STR* cmd, CMD_STR* ans)
{
	uint8_t command = cmd->command;
	uint16_t   size = cmd->size;

	ans->status = CMD_STATUS_OK;
	switch (command) {
		case CMD_TAP_SET_SRST:
			if (size != 1)
				ans->status = CMD_STATUS_SIZE_ERROR;
			else
				jtag_tap_set_srst(cmd->data[0]);
			ans->size = 0;
			break;

		case CMD_TAP_RESET:
			if (size != 2) {
				ans->status = CMD_STATUS_SIZE_ERROR;
				ans->size = 0;
			} else {
				jtag_tap_reset(CMD_GET_WORD(cmd, 0));
				ans->data[0] = jtag_tap_get_state();
				ans->size = 1;
			}
			break;

		case CMD_TAP_STATE:
			if (size != 0) {
				ans->status = CMD_STATUS_SIZE_ERROR;
				ans->size = 0;
			} else {
				ans->data[0] = jtag_tap_get_state();
				ans->size = 1;
			}
			break;

		case CMD_TAP_TRANS:
			tap_trans(cmd, ans);
			break;

		case CMD_TAP_SHIFT:
			tap_shift(cmd, ans);
			break;
		
		default:
			cmd_answer_error(cmd->command, CMD_STATUS_UNKOWN_COMMAND, ans);
	}
}

static void tap_trans(const CMD_STR* cmd, CMD_STR* ans)
{
	uint8_t tms, bit_size;
	STATUS_T status;
	
	/* Check */
	if (cmd->size != 2) {
		ans->status = CMD_STATUS_ERROR;
		ans->size = 0;
		return;
	}

	/* Get parameter */
	tms = cmd->data[0];
	bit_size = cmd->data[1];	// bit_size <= 8
	
	status = jtag_tap_trans_state2(tms, bit_size);
	if (status == TAP_STATUS_OK) {
		ans->status = CMD_STATUS_OK;
		ans->data[0] = jtag_tap_get_state();	// current TAP state
		ans->size = 1;
		return;									// data size
	} else {
		if (status == TAP_STATUS_INVALID_PARAM)
			ans->status = CMD_STATUS_INVALID_PARAM;
		else
			ans->status = CMD_STATUS_ERROR;
		ans->size = 0;
		return;
	}
}

static void tap_shift(const CMD_STR* cmd, CMD_STR* ans)
{
	uint16_t bit_size, buf_size, size;
	uint8_t  i, o, tms, *out_buf, *in_buf, state;

	/* Get parameter */
	bit_size = CMD_GET_WORD(cmd, 0);
	tms = cmd->data[2];
	i = tms & 0x04;
	o = tms & 0x02;
	tms &= 0x01;
	buf_size = bit_size/8 + (bit_size%8 == 0 ? 0 : 1);
	
	/* Check */
	if ((i == 0) && (o == 0)) {
		ans->status = CMD_STATUS_INVALID_PARAM;
		ans->size = 0;
		return;
	}
	size = sizeof(bit_size) + sizeof(tms) + (o == 0 ? 0 : buf_size);
	if (cmd->size != size) {
		ans->status = CMD_STATUS_SIZE_ERROR;
		ans->size = 0;
		return;
	}
	
	state = jtag_tap_get_state();
	if ((state != TAPSTATE_SHIFT_DR) && (state != TAPSTATE_SHIFT_IR)) {
		ans->status = CMD_STATUS_ERROR;
		ans->size = 0;
		return;
	}

	/* Run */
	out_buf = (o == 0 ? NULL : ((uint8_t*)&(cmd->data[3])));  // point to cmd->data[3]
	in_buf  = (i == 0 ? NULL : ((uint8_t*)&(ans->data[3])));  // point to ans->data[3]
	jtag_tap_shift(out_buf, in_buf, bit_size, tms);
	ans->data[0] = cmd->data[0];	// set low byte of bit_size
	ans->data[1] = cmd->data[1];	// set high byte of bit_size
	ans->data[2] = cmd->data[2];	// set tms
	ans->size = 3 + (i == 0 ? 0 : buf_size);
}


