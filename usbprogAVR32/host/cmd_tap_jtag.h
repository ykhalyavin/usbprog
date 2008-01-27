#ifndef __CMD_TAP_JTAG_H__
#define __CMD_TAP_JTAG_H__

#include "../include/commands.h"
#include "../include/basic.h"


int cmd_tap_set_srst(int value);
int cmd_tap_reset(U16 time);
int cmd_tap_state(U8 *state);
//int cmd_tap_trans(U8 tms, U8 bit_size, U8* state = NULL);
int cmd_tap_trans(U8 tms, U8 bit_size, U8* state);
int cmd_tap_shift(const U32* so, U32* si, U16 bit_size, int tms = 1);

int cmd_jtag_instruction(U32* instruction, U16 bit_size);
int cmd_jtag_data(const U32* so, U32* si, U16 bit_size);
int cmd_jtag_detect_ir(U16 max_size, int *size);
int cmd_jtag_detect_dr(U16 max_size, int *size);

#endif /* __CMD_TAP_JTAG_H__ */
