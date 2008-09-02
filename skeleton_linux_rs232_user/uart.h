void UARTInit(unsigned int baud0, unsigned int baud1, char stopbits, char parity, char databits);
void UARTPutChar(unsigned char sign);
unsigned char UARTGetChar(void);
void UARTWrite(char* msg);

unsigned char AsciiToHex(unsigned char high,unsigned char low);
void SendHex(unsigned char hex);



