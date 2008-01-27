#ifndef __CMD_H__
#define __CMD_H__

#include <stdint.h>
#include <avr/io.h>
#include "../include/basic.h"
#include "../include/commands.h"

#define FIRMWARE_VERSION					0x0001

#define LED_PORT_DDR						DDRA
#define LED_PORT_SET						PORTA
#define LED_PORT_BIT						BIT(4)
#define LED_ON()						(LED_PORT_SET |= LED_PORT_BIT)
#define LED_OFF()						(LED_PORT_SET &= ~LED_PORT_BIT)
#define LED_TOGGLE()						(LED_PORT_SET ^= LED_PORT_BIT)

#define CMD_CHECK_SIZE(cmd, expectation, ans)	{		\
	if ((cmd)->size != (expectation)) {			\
		cmd_answer_error((cmd)->command, CMD_STATUS_SIZE_ERROR, (ans));	\
		break;						\
	}								\
}

void cmd_init(void);
void cmd_parser(char *buf);
void cmd_answer_rest(void);
void cmd_answer_error(uint8_t command, STATUS_T status, CMD_STR *ans);

void cmd_comm(const CMD_STR* cmd, CMD_STR* ans);

#endif /* __CMD_H__ */

