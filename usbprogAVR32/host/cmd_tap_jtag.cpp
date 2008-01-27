#include "cmd_tap_jtag.h"

#include "cmd.h"
#include "usbprog.h"
//#include <windows.h>
#include <string.h>

static int detect_xr(U16 max_size, U16 *size, int dr);
static int shift(U8 command, const U32* so, U32* si, U16 bit_size, int tms);

int cmd_tap_set_srst(int value)
{
	CMD_STR cmd;

	cmd.command = CMD_TAP_SET_SRST;
	cmd.status = 0x00;
	cmd.data[0] = (char)value;
	cmd.size = 1;

	return cmd_execute(&cmd);
}

int cmd_tap_reset(U16 time)
{
	CMD_STR cmd;
	int t;

	t = (time > 500 ? 500 : time);

	cmd.command = CMD_TAP_RESET;
	cmd.status = 0x00;
	CMD_SET_WORD(&cmd, 0, t);
	cmd.size = 2;

	return cmd_execute(&cmd, 64, t);
/*
	CMD_STR cmd;
	char* buf = (char*)&cmd;
	int r;

	cmd.command = CMD_TAP_RESET;
	cmd.status = 0x00;
	CMD_SET_WORD(&cmd, 0, time);
	cmd.size = 2;

	r = usbprog_send(buf, cmd.size + 4);
	if (r < 0)
		return USBPROG_STATUS_ERROR;
	sleep(time);
	r = usbprog_receive(buf, 64);
	if ((r < 0) || (cmd.status != 0x00))
		return USBPROG_STATUS_ERROR;

	return USBPROG_STATUS_OK;
*/
}

int cmd_tap_state(U8 *state)
{
	CMD_STR cmd;
	char* buf = (char*)&cmd;
	int r;

	*state = TAPSTATE_UNDEFINED;

	cmd.command = CMD_TAP_STATE;
	cmd.status = 0x00;
	cmd.size = 0;

	r = usbprog_send(buf, cmd.size + 4);
	if (r < 0)
		return USBPROG_STATUS_ERROR;
	r = usbprog_receive(buf, 64);
	if ((r < 0) || (cmd.status != 0x00))
		return USBPROG_STATUS_ERROR;

	*state = cmd.data[0];

	return USBPROG_STATUS_OK;
}

int cmd_tap_trans(U8 tms, U8 bit_size, U8* state)
{
	CMD_STR cmd;
	char* buf = (char*)&cmd;
	int r;

	*state = TAPSTATE_UNDEFINED;

	if (bit_size > 8)
		return USBPROG_STATUS_INVALID_PARAM;

	cmd.command = CMD_TAP_TRANS;
	cmd.status = 0x00;
	cmd.size = 2;
	cmd.data[0] = tms;
	cmd.data[1] = bit_size;

	r = usbprog_send(buf, cmd.size + 4);
	if (r < 0)
		return USBPROG_STATUS_ERROR;
	r = usbprog_receive(buf, 64);
	if ((r < 0) || (cmd.status != 0x00))
		return USBPROG_STATUS_ERROR;

	*state = cmd.data[0];

	return USBPROG_STATUS_OK;
}

static int shift(U8 command, const U32* so, U32* si, U16 bit_size, int tms)
{
	char buf[CMD_ANSWER_BUF_LENGTH];
	CMD_STR* cmd = (CMD_STR*)buf;
	char io_tms;
	int r, size;

	if ((so == NULL) && (si == NULL))
		return USBPROG_STATUS_INVALID_PARAM;

	size = bit_size/BUF_BIT_WIDTH + (bit_size%BUF_BIT_WIDTH == 0 ? 0 : 1);
	/* Shift out/in data should not exceed the max capacity of the buf:
	 * 7 = cmd struct head size (4, including command, status and size)
	 *     + size of bit_size (2) + size of io_tms (1)
	 */
	if (size > (CMD_ANSWER_BUF_LENGTH - 7))
		return USBPROG_STATUS_INVALID_PARAM;

	io_tms  = (si == NULL ? 0x00 : 0x04);
	io_tms += (so == NULL ? 0x00 : 0x02);
	io_tms += (tms == 0   ? 0x00 : 0x01);

	cmd->command = command;
	cmd->status = 0x00;
	CMD_SET_WORD(cmd, 0, bit_size);
	cmd->data[2] = io_tms;
	cmd->size = sizeof(bit_size) + sizeof(io_tms);
	if (so != NULL) {
		memcpy((void*)&(cmd->data[3]), (void*)so, size);
		cmd->size += size;
	}

	r = usbprog_send(buf, cmd->size + 4);
	if (r < 0)
		return USBPROG_STATUS_ERROR;
	r = usbprog_receive(buf, CMD_ANSWER_BUF_LENGTH);
	if ((r < 0) || (cmd->status != 0x00))
		return USBPROG_STATUS_ERROR;

	if ((si != NULL) && (cmd->size > 3))
		memcpy((void*)si, (void*)&(cmd->data[3]), size);

	return USBPROG_STATUS_OK;
}

int cmd_tap_shift(const U32* so, U32* si, U16 bit_size, int tms)
{
	return shift(CMD_TAP_SHIFT, so, si, bit_size, tms);
}

int cmd_jtag_data(const U32* so, U32* si, U16 bit_size)
{
	return shift(CMD_JTAG_DATA, so, si, bit_size, 1);
}

int cmd_jtag_instruction(U32* instruction, U16 bit_size)
{
	char buf[CMD_ANSWER_BUF_LENGTH];
	CMD_STR* cmd = (CMD_STR*)buf;
	int r, size;

	if ((instruction == NULL) || (bit_size < 2))
		return USBPROG_STATUS_INVALID_PARAM;

	size = bit_size/BUF_BIT_WIDTH + (bit_size%BUF_BIT_WIDTH == 0 ? 0 : 1);
	/* Shift out/in data should not exceed the max capacity of the buf:
	 * 6 = cmd struct head size (4, including command, status and size)
	 *     + size of bit_size (2)
	 */
	if (size > (CMD_ANSWER_BUF_LENGTH - 6))
		return USBPROG_STATUS_INVALID_PARAM;

	cmd->command = CMD_JTAG_INSTRUCTION;
	cmd->status = 0x00;
	CMD_SET_WORD(cmd, 0, bit_size);
	memcpy((void*)&(cmd->data[2]), (void*)instruction, size);
	cmd->size = sizeof(bit_size) + size;

	r = usbprog_send(buf, cmd->size + 4);
	if (r < 0)
		return USBPROG_STATUS_ERROR;
	r = usbprog_receive(buf, CMD_ANSWER_BUF_LENGTH);
	if ((r < 0) || (cmd->status != 0x00))
		return USBPROG_STATUS_ERROR;

	memcpy((void*)instruction, (void*)&(cmd->data[2]), size);

	return USBPROG_STATUS_OK;
}

static int detect_xr(U16 max_size, int *size, int dr)
{
	char buf[CMD_ANSWER_BUF_LENGTH];
	CMD_STR* cmd = (CMD_STR*)buf;
	int r;

	if (max_size < 2)
		return USBPROG_STATUS_INVALID_PARAM;

	cmd->command = (dr == 0 ? CMD_JTAG_DETECT_IR : CMD_JTAG_DETECT_DR);
	cmd->status = 0x00;
	CMD_SET_WORD(cmd, 0, max_size);
	cmd->size = sizeof(max_size);

	r = usbprog_send(buf, cmd->size + 4);
	if (r < 0)
		return USBPROG_STATUS_ERROR;
	r = max_size/200 + (max_size%200 ? 1 : 0);
	sleep(r);		// Delay 1ms every shift 200 bits 
	r = usbprog_receive(buf, 64);
	if ((r < 0) || (cmd->status != 0x00))
		return USBPROG_STATUS_ERROR;

	*size = (int)CMD_GET_WORD(cmd, 0);

	return USBPROG_STATUS_OK;
}

int cmd_jtag_detect_ir(U16 max_size, int *size)
{
	return detect_xr(max_size, size, 0);
}

int cmd_jtag_detect_dr(U16 max_size, int *size)
{
	return detect_xr(max_size, size, 1);
}



