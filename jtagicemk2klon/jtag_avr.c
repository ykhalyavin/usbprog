#include "jtag_avr.h"
#include "jtag_avr_defines.h"
#include "jtag.h"


int idcode(char *buf)
{
	// READ IDCODE
	jtag_goto_state(SHIFT_IR);
	char jtagbuf[1];
	jtagbuf[0]=AVR_IDCODE;
  jtag_write(4,jtagbuf);

	jtag_goto_state(SHIFT_DR);
	jtag_read(32,buf);
}

int bypass()
{

}

int avr_reset()
{

}

int avr_prog_enable()
{

}

int avr_prog_cmd()
{

}

