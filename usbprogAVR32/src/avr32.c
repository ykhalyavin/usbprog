#include "avr32.h"

#include "jtag_tap.h"
#include "avr32_bsr.h"

#define IDCODE_MANUFACTUER_MASK 	0x00000FFE
#define IDCODE_PARTNUMBER_MASK   	0x0FFFF000
#define IDCODE_REVISION_MASK 		0xF0000000

#define ID_AP7000					0x01E8203E	// 0000_0001_1110_1000_0010_0000_0011_1110
#define ID_AP7001					0x01E8203E	// 0000_0001_1110_1000_0010_0000_0011_1110

//debug #include "uart.h"	// for debug

//BUF_T bsr[MAX_CHAIN_BUF_SIZE];
static uint8_t cpu = AVR32_UNKNOWN;

void avr32_init(void)
{
	cpu = AVR32_UNKNOWN;
	//jtag_tap_init();
}

STATUS_T avr32_send_ins(uint8_t ins)
{
	uint8_t r = ins;

	/* check the current TAP state */
	if (jtag_tap_get_state() != TAPSTATE_RUNTEST_IDLE )
		return AVR32_STATUS_INVALID_TAP_STATE;

	if (jtag_tap_shift_ir(&r, AVR32_IR_SIZE))
		return AVR32_STATUS_SHIFT_IR_ERROR;
	
	if ((r & AVR32_IR_CAPTURE_BIT) != AVR32_INSTRUCTION_CAPTURE)
		return AVR32_STATUS_INVALID_IR_CAPTURE;

	if ((r & AVR32_IR_BUSY_BIT) != 0)
		return AVR32_STATUS_IR_BUSY;
	
	if ((r & AVR32_IR_ERROR_BIT) != 0)
		return AVR32_STATUS_IR_ERROR;
	
	if ((r & AVR32_IR_PROTECT_BIT) != 0)
		return AVR32_STATUS_IR_PROTECT;
	
	return AVR32_STATUS_OK;
}

STATUS_T avr32_get_idcode(uint32_t* id)
{
	STATUS_T r;
	uint8_t* p = (uint8_t*)id;
#define GET_ID_METHOD 0
#if (GET_ID_METHOD == 0)
	r = avr32_send_ins(AVR32_JTAG_IDCODE);
	if (r != AVR32_STATUS_OK)
		return r;
	r = jtag_tap_shift_dr(NULL, p, 32);
	if (r != TAP_STATUS_OK)
		return AVR32_STATUS_SHIFT_DR_ERROR;
#else
	uint8_t eb;

	r = avr32_mem_access(1, 0x01, 0, id, &eb);
	if (r != AVR32_STATUS_OK)
		return AVR32_STATUS_ERROR;
	if (eb & (AVR32_IR_ERROR_BIT | AVR32_IR_BUSY_BIT))
		return AVR32_STATUS_ERROR;
	//debug uart_puthex(eb);
#endif	
#undef GET_ID_METHOD

	/* The LSB of IDCODE must be 1, according to the JEDEC Standard.
	 * If not, it is an invalid IDCODE value.
	 */
	if ( (*p & 0x01) == 0 )
		return AVR32_STATUS_INVALID_IDCODE;
	
	return AVR32_STATUS_OK;
}

uint8_t avr32_cpu_type(uint32_t id)
{
	uint32_t u = id & (IDCODE_MANUFACTUER_MASK | IDCODE_PARTNUMBER_MASK);
	uint8_t cpu_type;
	
	if (u == ID_AP7000)
		cpu_type = AVR32_AP7000;
	else if (u == ID_AP7001)
		cpu_type = AVR32_AP7001;
	else
		cpu_type = AVR32_UNKNOWN;
	
	return cpu_type;
}

STATUS_T avr32_mem_access(uint8_t read, uint8_t sab_slave, uint32_t address, uint32_t *data, uint8_t *eb)
{
	uint8_t r;
	uint32_t add = address >> 2;
	//debug uart_putchar('\''); uart_puthex_32(add); uart_putchar('\'');

	r = avr32_send_ins(AVR32_JTAG_MEM_WORD_ACCESS);
	if (r != AVR32_STATUS_OK)
		return r;

	r = avr32_send_ins(AVR32_JTAG_MEM_WORD_ACCESS);
	if (r != AVR32_STATUS_OK)
		return r;

	/* ?增加判断返回的r值的判断 */
	
	/* MEMORY_WORD_ACCESS address phase */
	r = read ? 0x01 : 0x00;
	jtag_tap_trans_state2(BIN(001), 3);					// Run-Test/Idle  -> Shift-DR
	jtag_tap_shift(&r, NULL, 1, 0);						// Shift out read/write bit
	jtag_tap_shift((uint8_t*)(&add), NULL, 30, 0);		// Shift out 32LSB bits address of SAB
	r = sab_slave;
	jtag_tap_shift(&r, NULL, 4, 1);				// Shift out 4MSB bits address of SAB, and Shift-DR -> Exit1-DR
	jtag_tap_trans_state2(BIN(0011), 4);				// Exit1-DR -> Shift-DR
	/* MEMORY_WORD_ACCESS data phase */
	if (read) {
		jtag_tap_shift(NULL, (uint8_t*)data, 32, 0);	// Shift in 32 bits data
		jtag_tap_shift(NULL, eb, 3, 1);					// Shift in 3 bits error and busy status
	} else {
		jtag_tap_shift(NULL, eb, 3, 0);					// Shift in 3 bits error and busy status
		jtag_tap_shift((uint8_t*)data, NULL, 32, 1);	// Shift out 32 bits data
	}
	jtag_tap_trans_state2(BIN(01), 2);					// Exit1-DR -> Run-Test/Idle
	
	return AVR32_STATUS_OK;
/*
# TMS       TDI         TDO 
b111111     b000000     xxxxxx      # JTAG Reset 
b00110      b00000      xxxxx       # Goto SHIFT-IR 
b10000      b10001      00001       # Shift in MEMORY_WORD_ACCESS 
b00101      b00000      xxxxx       # Goto RTI then SHIFT-DR 
b000        b001        xxx         # Shift in read-bit & 2 bits of address 
0x80000000  0x10000000  xxxxxxxx    # Shift in remaining 32 bits of address 
b0011       b0000       xxxx        # Move thru Update-DR, back to Shift-DR 
b000        b000        000         # Shift in error and busy bits (both 0) 
0x80000000  0x00000000  0x00000000  # Read the memory location 
b01         b00         xx          # Update-DR -> RTI 

*/
}

STATUS_T avr32_avr_reset(uint8_t domain)
{
	uint8_t d;
	
	d = AVR32_JTAG_AVR_RESET;
	if (jtag_tap_shift_ir(&d, AVR32_IR_SIZE))
		return AVR32_STATUS_ERROR;
	//uart_puts("r=0x"); uart_puthex(r);
	d = domain;
	if (jtag_tap_shift_dr(&d, &d, 5))
		return AVR32_STATUS_ERROR;
	//uart_puts(", r=0x"); uart_puthex(r);
	
	return AVR32_STATUS_OK;
}

STATUS_T avr32_extest(const BUF_T *bsr_out, BUF_T *bsr_in, uint8_t add_on)
{
	STATUS_T r;

	if ((bsr_out == NULL) && (bsr_in == NULL))
		return AVR32_STATUS_INVALID_PARAM;

	if (add_on == 0) {
		r = avr32_send_ins(AVR32_JTAG_SAMPLE);
		if (r != AVR32_STATUS_OK)
			return r;
	}

	if (jtag_tap_shift_dr(bsr_out, bsr_in, BSR_SIZE))
		return AVR32_STATUS_SHIFT_DR_ERROR;

	if (add_on == 0) {
		r = avr32_send_ins(AVR32_JTAG_EXTEST);
		if (r != AVR32_STATUS_OK)
			return r;
	}

	return AVR32_STATUS_OK;
}

#if 0
STATUS_T avr32_detect(uint8_t* cpu_type)
{
	uint16_t size;
	uint8_t	r;
	uint32_t id;
	
	jtag_tap_reset(100);
	if (jtag_tap_detect_ireg_size(AVR32_IR_SIZE*2, &size) != TAP_STATUS_OK)
		return AVR32_STATUS_SHIFT_IR_ERROR;
	if (size != AVR32_IR_SIZE);
		return AVR32_STATUS_ERROR;
	
	r = avr32_get_idcode(&id);
	if (r != AVR32_STATUS_OK)
		return r;

	*cpu_type = avr32_cpu_type(id);
	
	return AVR32_STATUS_OK;
}
#endif

