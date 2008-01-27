#include "cmd.h"


#include <string.h>


static CMD_STR err_ans;

void cmd_init(void)
{
	err_ans.command = 0xFF;
	err_ans.status = USBPROG_STATUS_OK;
	err_ans.size = 0;
}

void cmd_set_error(const CMD_STR *ans)
{
	memcpy(&err_ans, ans, sizeof(err_ans));
}

CMD_STR* cmd_get_error(void)
{
	return &err_ans;
}

int cmd_execute(CMD_STR *cmd, int max_receive_size, int sleep_ms, int timeout)
{
	int r;

	/* Send command */
	cmd->status = 0x00;
	r = usbprog_send((char*)cmd, cmd->size + CMD_HEAD_SIZE, timeout);
	if (r < 0)
		return USBPROG_STATUS_SEND_ERROR;

	/* Sleep */
	if (sleep_ms > 0)
		sleep(sleep_ms/1000); //linux
		// windows Sleep(sleep_ms)

	/* Get result */
	r = usbprog_receive((char*)cmd, max_receive_size, timeout);
	if (r < 0)
		return USBPROG_STATUS_RECEIVE_ERROR;
	if (cmd->status != 0x00) {
		cmd_set_error(cmd);
		return USBPROG_STATUS_EXECUTE_ERROR;
	}

	return CMD_STATUS_OK;
}

int cmd_comm_firmware_version(int *version)
{
	CMD_STR cmd;
	int r;

	cmd.command = CMD_COMM_FIRMWARE_VERSION;
	cmd.size = 0;

	r = cmd_execute(&cmd);
	*version = (r == CMD_STATUS_OK ? CMD_GET_WORD(&cmd, 0) : 0);

	return r;
}

int cmd_comm_init(void)
{
	CMD_STR cmd;

	cmd.command = CMD_COMM_INIT;
	cmd.size = 0;

	return cmd_execute(&cmd);
}

int cmd_comm_led(int on)
{
	CMD_STR cmd;

	cmd.command = CMD_COMM_LED_RED;
	cmd.data[0] = (on==0 ? 0x00 : 0x01);
	cmd.size = 1;

	return cmd_execute(&cmd);
}


