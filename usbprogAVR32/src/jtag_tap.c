#include "jtag_tap.h"

#include <stdint.h>
#include "jtag_bus.h"
#include "wait.h"


typedef struct tap_state_ {
	uint8_t next_state[2];
} tap_state_t;

/* Used to record the current TAP current_state */
static uint8_t current_state = TAPSTATE_UNDEFINED;
//volatile register uint8_t current_state asm ("r21");

/* TAP current_state transition array -
 *		This array descripes the TAP current_state machine. It is used
 *		to instruct the TAP current_state transition.
 */
static const tap_state_t state_trans[NUM_OF_STATES] = {		//current state
	{ {TAPSTATE_SHIFT_DR, TAPSTATE_UPDATE_DR} },			//TAPSTATE_EXIT2_DR
	{ {TAPSTATE_PAUSE_DR, TAPSTATE_UPDATE_DR} },			//TAPSTATE_EXIT1_DR
	{ {TAPSTATE_SHIFT_DR, TAPSTATE_EXIT1_DR} },				//TAPSTATE_SHIFT_DR
	{ {TAPSTATE_PAUSE_DR, TAPSTATE_EXIT2_DR} },				//TAPSTATE_PAUSE_DR
	{ {TAPSTATE_CAPTURE_IR, TAPSTATE_TSTLOG_RST} },			//TAPSTATE_SELECT_IR_SCAN
	{ {TAPSTATE_RUNTEST_IDLE, TAPSTATE_SELECT_DR_SCAN} },	//TAPSTATE_UPDATE_DR
	{ {TAPSTATE_SHIFT_DR, TAPSTATE_EXIT1_DR} },				//TAPSTATE_CAPTURE_DR
	{ {TAPSTATE_CAPTURE_DR, TAPSTATE_SELECT_IR_SCAN} },		//TAPSTATE_SELECT_DR_SCAN
	{ {TAPSTATE_SHIFT_IR, TAPSTATE_UPDATE_IR} },			//TAPSTATE_EXIT2_IR
	{ {TAPSTATE_PAUSE_IR, TAPSTATE_UPDATE_IR} },			//TAPSTATE_EXIT1_IR
	{ {TAPSTATE_SHIFT_IR, TAPSTATE_EXIT1_IR} },				//TAPSTATE_SHIFT_IR
	{ {TAPSTATE_PAUSE_IR, TAPSTATE_EXIT2_IR} },				//TAPSTATE_PAUSE_IR
	{ {TAPSTATE_RUNTEST_IDLE, TAPSTATE_SELECT_DR_SCAN} },	//TAPSTATE_RUNTST_IDLE
	{ {TAPSTATE_RUNTEST_IDLE, TAPSTATE_SELECT_DR_SCAN} },	//TAPSTATE_UPDATE_IR
	{ {TAPSTATE_SHIFT_IR, TAPSTATE_EXIT1_IR} },				//TAPSTATE_CAPTURE_IR
	{ {TAPSTATE_RUNTEST_IDLE, TAPSTATE_TSTLOG_RST} },		//TAPSTATE_TSTLOG_RST
};

/* local utility routines prototype */
static void rti_to_shiftdr(void) __attribute__((always_inline));
static void rti_to_shiftir(void) __attribute__((always_inline));
static void shiftxr_to_rti(void) __attribute__((always_inline));
static void shift_bit(void) __attribute__((always_inline));
static STATUS_T detect_reg_size(uint16_t max_size, uint16_t *size);


void jtag_tap_init(void)
{
	jtag_bus_init();
	current_state = TAPSTATE_UNDEFINED;
}

void jtag_tap_reset(int ms)
{
	uint8_t i;

	jtag_tap_init();

	/* Reset the TAP controller through ntrst */
	if (ms > 0)
		jtag_tap_assert_trst(ms);
	
	/* Reset the TAP controller through TMS */
	jtag_bus_set_tms(1);
	for (i = 0; i < 6; i++)
		shift_bit();

	/* Update the current_state of TAP controller */
	current_state = TAPSTATE_TSTLOG_RST;

	/* Now enter Run-Test/Idle current_state and stay here */
	jtag_tap_trans_state(0);	//Test-Logic Reset  -> Run-Test/Idle
	jtag_tap_trans_state(0); 	//Run-Test/Idle     -> Run-Test/Idle
	//jtag_bus_update();			//clear TCK, keep others
}

STATUS_T jtag_tap_trans_state(uint8_t tms)
{
	if (current_state > 0x0F)
		return TAP_STATUS_INVALID_TAPSTATE;

	jtag_bus_set_tms(tms);
	shift_bit();
	current_state = state_trans[current_state].next_state[(tms ? 1 : 0)];

	return TAP_STATUS_OK;
}

STATUS_T jtag_tap_trans_state2(uint8_t tms, uint8_t bit_size)
{
	uint8_t i, one_hot, t;
	
	if (bit_size > 8)
		return TAP_STATUS_INVALID_PARAM;

	if (current_state > 0x0F)
		return TAP_STATUS_INVALID_TAPSTATE;
	
	jtag_bus_set_tdi(0);
	for (i=0, one_hot=1; i<bit_size; i++, one_hot<<=1) {
		t = tms & one_hot;
		jtag_bus_set_tms(t);
		jtag_bus_update();
		jtag_bus_pulse_tck();
		current_state = state_trans[current_state].next_state[(t ? 1 : 0)];
	}

	return TAP_STATUS_OK;
}

uint8_t jtag_tap_get_state(void)
{
	return current_state;
}

/* 
 * Shifting bit stream into / out from the TAP's instrument or data register.
 * This function assumes that the TAP is in Shift-IR or Shift-DR status
 * and the input parameters are valid (buf_out!=NULL and size>0).
 * And the TAP controller's current_state will return to Run-Test/Idle.
 */
void jtag_tap_shift(const BUF_T *buf_out, BUF_T *buf_in, uint16_t bit_size, uint8_t tms)
{
	uint16_t bit_count, buf_count;
	register uint8_t data = 0, one_hot;

	jtag_bus_set_tms(0);
	for (bit_count=1, buf_count=0, one_hot=1; bit_count<=bit_size; bit_count++) {
		if ( one_hot == 1 )
			data = (buf_out == NULL) ? 0 : buf_out[buf_count];

		jtag_bus_set_tdi(data & one_hot);	// write a bit
		if ( bit_count == bit_size )
			jtag_tap_trans_state(tms);		// shift the last bit with current_state transforming
		else
			shift_bit();					// no current_state transforming
		if ( jtag_bus_get_tdo() )			// read a bit
			data |= one_hot;
		else
			data &= ~one_hot;

		/* update buf for tdo when have read 8 bits or last bit */
		if ( (one_hot==(1<<(BUF_BIT_WIDTH-1))) || (bit_count==bit_size) ) {
			if ( buf_in != NULL )
				buf_in[buf_count] = data;
			buf_count++;
		}

		/* 循环左移 one_hot */
		one_hot = ( one_hot == (1<<(BUF_BIT_WIDTH-1)) ) ? 1 : (one_hot << 1);
	}  // end for
}

STATUS_T jtag_tap_shift_ir(BUF_T *instruction, uint16_t bit_size)
{
	if ( bit_size <= 1 )
		return TAP_STATUS_INVALID_PARAM;

	/* check the current TAP controller's current_state */
	if ( current_state != TAPSTATE_RUNTEST_IDLE )
		return TAP_STATUS_INVALID_TAPSTATE;

	/* transform the TAP controller's current_state to Shift-DR */
	rti_to_shiftir();
	/* shift bit stream and go to Exit1-IR state (tms = 1) */
	jtag_tap_shift(instruction, instruction, bit_size, 1);
	
	/* Current TAP controller's current_state should be Exit1-xR */
	jtag_tap_trans_state(1);		// Exit1-IR  -> Update-IR
	jtag_tap_trans_state(0);		// Update-IR -> Run-Test/Idle

	return TAP_STATUS_OK;
}

STATUS_T jtag_tap_shift_dr(const BUF_T *data_out, BUF_T *data_in, uint16_t bit_size)
{
	if ( (bit_size < 1) || ((data_out==NULL) && (data_in==NULL)) )
		return TAP_STATUS_INVALID_PARAM;

	/* check the current TAP controller's current_state */
	if ( current_state != TAPSTATE_RUNTEST_IDLE )
		return TAP_STATUS_INVALID_TAPSTATE;

	/* transform the TAP controller's current_state to Shift-DR */
	rti_to_shiftdr();
	/* shift bit stream and go to Exit1-IR state (tms = 1) */
	jtag_tap_shift(data_out, data_in, bit_size, 1);
	
	/* Current TAP controller's current_state should be Exit1-DR */
	jtag_tap_trans_state(1);		// Exit1-DR  -> Update-DR
	jtag_tap_trans_state(0);		// Update-DR -> Run-Test/Idle

	return TAP_STATUS_OK;
}

void jtag_tap_assert_trst(int ms)
{
	jtag_bus_set_ntrst(0);  // drive low
	wait_ms(ms);
	jtag_bus_set_ntrst(1);  // drive high
	wait_ms(1);
}

void jtag_tap_assert_srst(int ms)
{
	jtag_bus_set_nsrst(0);  // drive low
	wait_ms(ms);
	jtag_bus_set_nsrst(1);  // drive high
	wait_ms(1);
}

void jtag_tap_set_srst(uint8_t value)
{
	jtag_bus_set_nsrst(value);
}

/* 
 * Detecting the TAP's instrument or data register's size.
 * This function assumes that the TAP is in Shift-IR or Shift-DR status,
 * and the input parameter is valid (max_size>0).
 */
static STATUS_T detect_reg_size(uint16_t max_size, uint16_t *size)
{
	uint16_t i;
	
	if ((max_size <2) || (max_size > 65530))
		return TAP_STATUS_INVALID_PARAM;

	/* clear the shift register(s) on the chain using 0 */
	jtag_bus_set_tms(0);	// clear tms to keep status
	jtag_bus_set_tdi(0);	// clear tdi
	for (i = 0; i < (max_size + 1); i++) {
		jtag_bus_update();				// Write a bit
		jtag_bus_pulse_tck();
	}

	/* detect the total length of the shift register(s) on the chain using 1 */
	jtag_bus_set_tdi(1);	// set tdi
	for (i = 0; i < (max_size + 1); i++) {
		jtag_bus_update();				// Write a bit
		jtag_bus_pulse_tck();
		if (jtag_bus_get_tdo())
			break;
	}
	*size = i;
	
	if (i > max_size)
		return TAP_STATUS_RESULT_OVERFLOW;
	else
		return TAP_STATUS_OK;
}

STATUS_T jtag_tap_detect_ireg_size(uint16_t max_size, uint16_t *size)
{
	uint8_t r;

	/* check the current TAP controller's current_state */
	if (current_state != TAPSTATE_RUNTEST_IDLE)
		return TAP_STATUS_INVALID_TAPSTATE;

	rti_to_shiftir();		// transform the TAP controller's current_state to Shift-IR

	/* 因为函数detect_reg_size给ireg发送了全1的bit串，
	 * Update-IR以后，相当于给TAP Controller发送BYPASS指令。
	 */
	r = detect_reg_size(max_size, size);

	shiftxr_to_rti();		// transform the TAP controller's current_state to Run-Test/Idle

	return r;
}

STATUS_T jtag_tap_detect_dreg_size(uint16_t max_size, uint16_t *size)
{
	register uint8_t r;

	/* check the current TAP controller's current_state */
	if (current_state != TAPSTATE_RUNTEST_IDLE)
		return TAP_STATUS_INVALID_TAPSTATE;

	rti_to_shiftdr();		// transform the TAP controller's current_state to Shift-DR

	/* 注意：测试dreg的位宽可能会导致硬件损坏。
	 * 最好能记录原始的dreg数据，在测试完成后恢复，或是复位一下TAP。
	 * 前者在RAM资源有限的情况下，max_size不可能太大；
	 * 后者中间状态会更新dreg的数据，除非目标支持TRST引脚功能。
	 */
	r = detect_reg_size(max_size, size);
	//shiftxr_to_rti();		// transform the TAP controller's current_state to Run-Test/Idle
	jtag_tap_reset(20);

	return r;
}


/* Transforming the TAP controller's current_state from Run-Test/Idle to Shift-DR.
 * The function *always assume* the current current_state of the TAP controller
 * is Run-Test/Idle!
 */
static void rti_to_shiftdr(void)
{
	jtag_tap_trans_state(1);		// Run-Test/Idle  -> Select-DR-Scan
	jtag_tap_trans_state(0);		// Select-DR-Scan -> Capture-DR
	jtag_tap_trans_state(0);		// Capture-DR	  -> Shift-DR
}

/* Transforming the TAP controller's current_state from Run-Test/Idle to Shift-IR.
 * The function *always assume* the current current_state of the TAP controller
 * is Run-Test/Idle!
 */
static void rti_to_shiftir(void)
{
	jtag_tap_trans_state(1);		// Run-Test/Idle  -> Select-DR-Scan
	jtag_tap_trans_state(1);		// Select-DR-Scan -> Select-IR-Scan
	jtag_tap_trans_state(0);		// Select-IR-Scan -> Capture-IR
	jtag_tap_trans_state(0);		// Capture-IR	  -> Shift-IR
}

/* Transforming the TAP controller's current_state from Shift-IR or Shift-DR
 * to Run-Test/Idle.
 * The function *always assume* the current current_state of the TAP controller
 * is Shift-IR or Shift-DR!
 */
static void shiftxr_to_rti(void)
{
	jtag_tap_trans_state(1);		// Shift-xR  -> Exit1-xR
	jtag_tap_trans_state(1);		// Exit1-xR  -> Update-xR
	jtag_tap_trans_state(0);		// Update-xR -> Run-Test/Idle
}

/*
uint8_t jtag_tap_shift_bit(uint8_t tdi, uint8_t tms)
{
	jtag_bus_set_tdi(tdi);
	jtag_tap_trans_state(tms);

	return jtag_bus_get_tdo();
}
*/

static void shift_bit(void)
{
	jtag_bus_update();
	jtag_bus_pulse_tck();
}
