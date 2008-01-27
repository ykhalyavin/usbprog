#ifndef __JTAG_TAP_H__
#define __JTAG_TAP_H__

#include <stdint.h>
#include "../include/basic.h"

/* TAP's states */
/* defined in basic.h
#define NUM_OF_STATES				0x10		//16 TAP States
#define TAPSTATE_UNDEFINED			0xF0
#define TAPSTATE_MASK				0x0F

#define TAPSTATE_TSTLOG_RST			0xF
#define TAPSTATE_RUNTEST_IDLE		0xC
#define TAPSTATE_SELECT_DR_SCAN		0x7
#define TAPSTATE_CAPTURE_DR			0x6
#define TAPSTATE_SHIFT_DR			0x2
#define TAPSTATE_EXIT1_DR			0x1
#define TAPSTATE_PAUSE_DR			0x3
#define TAPSTATE_EXIT2_DR			0x0
#define TAPSTATE_UPDATE_DR			0x5
#define TAPSTATE_SELECT_IR_SCAN		0x4
#define TAPSTATE_CAPTURE_IR			0xE
#define TAPSTATE_SHIFT_IR			0xA
#define TAPSTATE_EXIT1_IR			0x9
#define TAPSTATE_PAUSE_IR			0xB
#define TAPSTATE_EXIT2_IR			0x8
#define TAPSTATE_UPDATE_IR			0xD
*/

/* Status codes */
#define TAP_STATUS_OK					0x00
#define TAP_STATUS_INVALID_PARAM		0x01
#define TAP_STATUS_INVALID_TAPSTATE		0x02
#define TAP_STATUS_ACCESS_IR_FAIL		0x03
#define TAP_STATUS_ACCESS_DR_FAIL		0x04
#define TAP_STATUS_INVALID_IR_SIZE		0x05
#define TAP_STATUS_RESULT_OVERFLOW		0x05
#define TAP_STATUS_ERROR				0x0A


/* routines */
void    jtag_tap_init(void);
void    jtag_tap_reset(int ms);

STATUS_T jtag_tap_trans_state(uint8_t tms);
STATUS_T jtag_tap_trans_state2(uint8_t tms, uint8_t bit_size);
uint8_t  jtag_tap_get_state(void);

void     jtag_tap_shift(const BUF_T *buf_out, BUF_T *buf_in, uint16_t bit_size, uint8_t tms);
STATUS_T jtag_tap_shift_ir(BUF_T *instruction, uint16_t bit_size);
STATUS_T jtag_tap_shift_dr(const BUF_T *data_out, BUF_T *data_in, uint16_t bit_size);

STATUS_T jtag_tap_detect_ireg_size(uint16_t max_size, uint16_t *size);
STATUS_T jtag_tap_detect_dreg_size(uint16_t max_size, uint16_t *size);

void jtag_tap_assert_trst(int ms);
void jtag_tap_assert_srst(int ms);
void jtag_tap_set_srst(uint8_t value);

#endif  /* __JTAG_TAP_H__ */
