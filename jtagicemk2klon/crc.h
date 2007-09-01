// taken from atmels datasheet avr067

unsigned short crc16_checksum(char * buf,unsigned long length);
void crc16_append(char * buf, unsigned long length);

#define CRC_INIT 0xFFFF
#define CRC(crcval,newchar) crcval = ((crcval >> 8) ^ pgm_read_word(&crc_table[(crcval ^ newchar) & 0x00ff]))

