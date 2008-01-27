#ifndef __CMD_JTAG__
#define __CMD_JTAG__

#include "cmd.h"

void cmd_jtag_init(void);
void cmd_jtag(const CMD_STR* cmd, CMD_STR* ans);

#endif /* __CMD_JTAG__ */
