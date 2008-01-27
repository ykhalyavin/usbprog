#ifndef __CMD_H__
#define __CMD_H__

#include "../include/commands.h"
#include "usbprog.h"


void cmd_init(void);
void cmd_set_error(const CMD_STR *ans);
int cmd_execute(CMD_STR *cmd,
				int max_receive_size = 64,
				int sleep_ms = 0,
				int timeout = DEFAULT_TIMEOUT);
CMD_STR* cmd_get_error(void);

int cmd_comm_firmware_version(int *version);
int cmd_comm_init(void);
int cmd_comm_led(int on);

#endif /* __CMD_H__ */
