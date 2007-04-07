

int idcode(char *buf);
int bypass();
int avr_reset(int true);
int avr_prog_enable();
int avr_prog_cmd();
int avr_jtag_instr(unsigned char instr, int delay);


void avr_sequence(char tdi2, char tdi1, char * tdo);

