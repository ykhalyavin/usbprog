void SendByte(unsigned char);
void _monitor_ (void);
#define TRAP() asm volatile ("rcall gdb_break")
