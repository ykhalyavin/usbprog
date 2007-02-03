/* 
 * Benedikt Sauter <sauter@ixbat.de> 2006-04-10
 *	m32		- atmega32
 *
 *	Message to usbflash Hardware:
 *
 * 	type cmd 
 * 	
 *	upload:
 * 	type cmd startaddrhigh staraddrlow len 
 *
 *	erase:
 *	type cmd
 * 
 *  reset:
 *	type cmd

 */

#define MAXPACKETSIZE	64

#define FALSE 0
#define TRUE  1
#define BUF 4096


#define AT89	0x01
#define M8		0x02	// atmega8
#define M32		0x03	// atmega32


#define RESET 0x00
#define ERASE 0x01
#define UPLOAD 0x02

