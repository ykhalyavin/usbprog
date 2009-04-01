#include <stdint.h>

void uart_init(uint32_t baudrate, char stopbits, char parity, char databits);
void uart_putchar(unsigned char sign);
char uart_getchar(void);
void uart_puts(char *s);
char AsciiToHex(char high,char low);
void SendHex(unsigned char hex);
