#include "cmd.h"

#include "usbn2mc.h"

#include "cmd_tap.h"
#include "cmd_jtag.h"
#include "cmd_avr32_jtag.h"
#include "cmd_avr32_bsr.h"
#include "cmd_avr32_ebi.h"
#include "cmd_flash.h"

/* Locally globale variable */
static char g_answer_buf[CMD_ANSWER_BUF_LENGTH];
static CMD_STR* g_ans = (CMD_STR*)g_answer_buf;

static uint8_t  g_rest = 0;
static uint16_t g_size = 0;
static uint16_t g_index = 0;

static uint8_t g_data_toggle = 0;

/* Local function prototype */
static void answer(uint16_t size);
static void cmd_comm_init(void);


static void cmd_comm_init(void)
{
	cmd_avr32_jtag_init();
	cmd_tap_init();
	cmd_flash_init();
}

void cmd_init(void)
{
	g_data_toggle = 0;
	g_rest = 0;
	g_size = 0;
	g_index = 0;
	
	LED_PORT_DDR |= LED_PORT_BIT;

	cmd_comm_init();
}

void cmd_answer_error(uint8_t command, STATUS_T status, CMD_STR *ans)
{
	ans->command = command;
	ans->status  = status;
	ans->size    = 0;
}

/* central command parser */
void cmd_parser(char *buf)
{
	CMD_STR* cmd = (CMD_STR*)buf;

	g_ans->command = cmd->command;
	g_ans->status = CMD_STATUS_OK;
	g_ans->size = 0;
	switch (cmd->command & CMD_GROUP_MASK) {
		case CMD_GROUP_COMM:
			cmd_comm(cmd, g_ans);
			break;
		case CMD_GROUP_TAP:
			cmd_tap(cmd, g_ans);
			break;
		case CMD_GROUP_JTAG:
			cmd_jtag(cmd, g_ans);
			break;
		case CMD_GROUP_AVR32_JTAG1:
		case CMD_GROUP_AVR32_JTAG2:
			cmd_avr32_jtag(cmd, g_ans);
			break;
		case CMD_GROUP_AVR32_BSR:
			cmd_avr32_bsr(cmd, g_ans);
			break;
		case CMD_GROUP_AVR32_EBI:
			cmd_avr32_ebi(cmd, g_ans);
			break;
		case CMD_GROUP_FLASH:
			cmd_flash(cmd, g_ans);
			break;

		default:
			cmd_answer_error(cmd->command, CMD_STATUS_UNKOWN_COMMAND, g_ans);
			break;
	}
	
	for (uint16_t i=g_ans->size; i<(CMD_ANSWER_BUF_LENGTH-4); i++)
		g_ans->data[i] = 0xAA;

	answer(g_ans->size + 4);
}

void cmd_comm(const CMD_STR* cmd, CMD_STR* ans)
{
	uint8_t command = cmd->command;
	uint16_t   size = cmd->size;

	ans->status = CMD_STATUS_OK;
	switch (command) {
		case CMD_COMM_FIRMWARE_VERSION:
			if (size != 0) {
				ans->status = CMD_STATUS_SIZE_ERROR;
				ans->size = 0;
			} else {
				ans->status = CMD_STATUS_OK;
				CMD_SET_WORD(ans, 0, FIRMWARE_VERSION);
				ans->size = 2;
			}
			break;

		case CMD_COMM_INIT:
			if (size != 0) {
				ans->status = CMD_STATUS_SIZE_ERROR;
			} else {
				cmd_comm_init();
				ans->status = CMD_STATUS_OK;
			}
			ans->size = 0;
			break;

		case CMD_COMM_LED_RED:
			if (cmd->size != 1) {
				g_ans->status = CMD_STATUS_SIZE_ERROR;
			} else {
				if (cmd->data[0] == 0)
					LED_OFF();
				else
					LED_ON();
			}
			
			break;

		default:
			cmd_answer_error(cmd->command, CMD_STATUS_UNKOWN_COMMAND, ans);
			break;
	}
}

void cmd_answer_rest(void)
{
	uint16_t diff;
	
	if (g_rest == 0)
		return;

	if (g_index < g_size) {
		diff = g_size - g_index; 
		g_index += 64;
		if (diff > 64) {
			answer(64);
		} else {
			/* last package */
			answer(diff);
			g_rest = 0;
		}
	}
}

void answer(uint16_t size)
{
	uint16_t i;

	/* if first packet of a long message */
	if ((size > 64) && (g_rest == 0)) {
		g_index = 0;
		g_size = size;
		g_rest = 1;
		size = 64;
	}

	USBNWrite(TXC1, FLUSH);

	for(i=0; i<size; i++)
		USBNWrite(TXD1, g_answer_buf[g_index + i]);

	/* control togl bit */
	if (g_data_toggle == 0) {
		USBNWrite(TXC1, TX_LAST + TX_EN);
		g_data_toggle = 1;
	} else {
		USBNWrite(TXC1, TX_LAST + TX_EN + TX_TOGL);
		g_data_toggle = 0;
	}
}


