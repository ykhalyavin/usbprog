/**** ATMEL AVR - A P P L I C A T I O N   N O T E  ************************
*
* Title:		AVR068 - STK500 Communication Protocol
* Filename:		command.h
* Version:		1.0
* Last updated:	10.01.2005
*
* Support E-mail:	avr@atmel.com
*
**************************************************************************/

/*****************[ General command constants ]**************************/

#define CMD_SIGN_ON                         0x01
#define CMD_SET_PARAMETER                   0x02
#define CMD_GET_PARAMETER                   0x03
#define CMD_OSCCAL                          0x05
#define CMD_LOAD_ADDRESS                    0x06
#define CMD_FIRMWARE_UPGRADE                0x07
#define CMD_RESET_PROTECTION                0x0A

/*****************[ ISP command constants ]******************************/

#define CMD_ENTER_PROGMODE_ISP              0x10
#define CMD_LEAVE_PROGMODE_ISP              0x11
#define CMD_CHIP_ERASE_ISP                  0x12
#define CMD_PROGRAM_FLASH_ISP               0x13
#define CMD_READ_FLASH_ISP                  0x14
#define CMD_PROGRAM_EEPROM_ISP              0x15
#define CMD_READ_EEPROM_ISP                 0x16
#define CMD_PROGRAM_FUSE_ISP                0x17
#define CMD_READ_FUSE_ISP                   0x18
#define CMD_PROGRAM_LOCK_ISP                0x19
#define CMD_READ_LOCK_ISP                   0x1A
#define CMD_READ_SIGNATURE_ISP              0x1B
#define CMD_READ_OSCCAL_ISP                 0x1C
#define CMD_SPI_MULTI                       0x1D

/*****************[ status constants ]***************************/

// Success
#define STATUS_CMD_OK                       0x00

// Warnings
#define STATUS_CMD_TOUT                     0x80
#define STATUS_RDY_BSY_TOUT                 0x81
#define STATUS_SET_PARAM_MISSING            0x82

//Errors
#define STATUS_CMD_FAILED                   0xC0
#define STATUS_CMD_UNKNOWN                  0xC9

/*****************[ parameter constants ]***************************/
#define PARAM_BUILD_NUMBER_LOW              0x80
#define PARAM_BUILD_NUMBER_HIGH             0x81
#define PARAM_HW_VER                        0x90
#define PARAM_SW_MAJOR                      0x91
#define PARAM_SW_MINOR                      0x92
#define PARAM_VTARGET                       0x94
#define PARAM_SCK_DURATION                  0x98
#define PARAM_DATA                          0x9D
#define PARAM_RESET_POLARITY                0x9E
#define PARAM_STATUS_TGT_CONN               0xA1
#define PARAM_DISCHARGEDELAY               	0xA4

/*****************[ status constants ]***************************/

#define STATUS_ISP_READY					0x00
#define STATUS_CONN_FAIL_MOSI				0x01
#define STATUS_CONN_FAIL_RST				0x02
#define STATUS_CONN_FAIL_SCK				0x04
#define STATUS_TGT_NOT_DETECTED				0x10
#define STATUS_TGT_REVERSE_INSERTED			0x20
