//#define ANALOG              'A'
//#define WRITECONFIG         'C'     // Set Program Counter to 0x2000 (Config Mem)
//#define CMDTABLE            'c'     // Download command table
//#define WRITEDATA           'D'     // Write data memory
//#define WRITEDATAEXT        'd'     // Write data memory externally timed
//#define ERASEPGM            'E'     // Bulk erase program memory
//#define ERASEDATA           'e'     // Bulk erase data memory
//#define RECEIVE             'g'     // Get n bytes from flash device
//#define INCADDRESS          'I'     // Increment address
//#define VDDVPPSTATUS		'M'		// Get status byte and Vdd, Vpp ADC Conversions WEK
//#define WRITEnWORDSEXT      'n'     // Midrange write by n, externally timed
//#define WRITEnWORDSINT      'N'     // Midrange write by n, internally timed
//#define ENTERPROGVDD1ST     'O'     // Enter programming mode
//#define ENTERPROG           'P'     // Enter programming mode
//#define EXITPROG            'p'     // Exit programming mode
//#define READPGM             'R'     // Read program memory
//#define READDATA            'r'     // Read EE data memory
//#define CHECKSUM            'S'     // Calculate checksum
//#define SETVDDVPP           's'     // Set VDD and VPP voltages. WEK
//#define TRANSFER            't'     // Transfer n bytes to flash device
//#define POWERCTRL           'V'     // Turn Vdd on/off; turn 5kHz square wave on/off
#define GETVERSION          'v'     // Get firmware version number
//#define WRITEPGM            'W'     // Write program memory
//#define WRITEPGMEXT         'w'     // Write 1 word to program memory, externally timed
//#define WRITE4WORDS         '4'     // Write 4 words to program memory
#define BOOTMODE            'B'     // Enter bootloader mode
//#define PIC18FREAD          0x80    // Read n bytes starting at address
//#define PIC18FEEDATAREAD    0x81
//#define PIC18FWRITE1        0x82
//#define PIC18FWRITE2        0x83
//#define PIC18FWRITECONFIG   0x84
//#define PIC18FERASE         0x85
//#define PIC18FERASE1        0x86
//#define PIC18FEEDATAWRITE   0x87
//#define	PIC18J_ENTERPROG	0x90
//#define	PIC18J_EXITPROG		0x91
//#define PIC18J_ERASE		0x92
//#define PIC18J_BUFFER32		0x93
//#define PIC18J_WRITE64		0x94
#define	NO_OPERATION		'Z'		// does nothing	 

#define SETVDD				0xA0
#define SETVPP				0xA1
#define READ_STATUS 		0xA2
#define READ_VOLTAGES		0xA3
#define DOWNLOAD_SCRIPT		0xA4
#define RUN_SCRIPT			0xA5
#define EXECUTE_SCRIPT		0xA6
#define CLR_DOWNLOAD_BUFFER	0xA7
#define DOWNLOAD_DATA		0xA8
#define CLR_UPLOAD_BUFFER	0xA9
#define UPLOAD_DATA			0xAA
#define CLR_SCRIPT_BUFFER	0xAB
#define UPLOAD_DATA_NOLEN   0xAC
#define END_OF_BUFFER       0xAD
#define RESET               0xAE
#define SCRIPT_BUFFER_CHKSM 0xAF
#define SET_VOLTAGE_CALS    0xB0
#define WR_INTERNAL_EE      0xB1
#define RD_INTERNAL_EE      0xB2
#define ENTER_UART_MODE     0xB3
#define EXIT_UART_MODE      0xB4


// SCRIPT CONTROL BYTE DEFINITIONS
#define	VDD_ON				0xFF
#define VDD_OFF				0xFE 
#define VDD_GND_ON			0xFD
#define VDD_GND_OFF			0xFC
#define VPP_ON				0xFB
#define VPP_OFF				0xFA
#define VPP_PWM_ON			0xF9
#define VPP_PWM_OFF			0xF8
#define MCLR_GND_ON			0xF7
#define MCLR_GND_OFF		0xF6
#define BUSY_LED_ON			0xF5
#define BUSY_LED_OFF		0xF4
#define SET_ICSP_PINS		0xF3
#define WRITE_BYTE_LITERAL	0xF2
#define WRITE_BYTE_BUFFER   0xF1
#define READ_BYTE_BUFFER    0xF0
#define READ_BYTE  			0xEF
#define WRITE_BITS_LITERAL	0xEE
#define WRITE_BITS_BUFFER	0xED
#define READ_BITS_BUFFER	0xEC
#define READ_BITS			0xEB
#define SET_ICSP_SPEED      0xEA
#define LOOP				0xE9
#define DELAY_LONG 			0xE8
#define DELAY_SHORT			0xE7
#define IF_EQ_GOTO			0xE6
#define IF_GT_GOTO			0xE5
#define GOTO_INDEX	        0xE4
#define EXIT_SCRIPT			0xE3
#define PEEK_SFR			0xE2
#define POKE_SFR			0xE1
#define ICDSLAVE_RX         0xE0
#define ICDSLAVE_TX_LIT     0xDF
#define ICDSLAVE_TX_BUF     0xDE
#define LOOPBUFFER			0xDD
#define ICSP_STATES_BUFFER  0xDC
#define POP_DOWNLOAD		0xDB
#define COREINST18          0xDA
#define COREINST24			0xD9
#define NOP24               0xD8
#define VISI24        		0xD7
#define RD2_BYTE_BUFFER		0xD6
#define RD2_BITS_BUFFER		0xD5
#define WRITE_BUFWORD_W		0xD4
#define WRITE_BUFBYTE_W		0xD3
#define CONST_WRITE_DL		0xD2
#define WRITE_BITS_LIT_HLD  0xD1
#define WRITE_BITS_BUF_HLD	0xD0
#define SET_AUX				0xCF
#define AUX_STATE_BUFFER	0xCE
#define I2C_START			0xCD
#define I2C_STOP			0xCC
#define I2C_WR_BYTE_LIT		0xCB
#define I2C_WR_BYTE_BUF		0xCA
#define I2C_RD_BYTE_ACK		0xC9
#define I2C_RD_BYTE_NACK	0xC8
#define SPI_WR_BYTE_LIT		0xC7
#define SPI_WR_BYTE_BUF		0xC6
#define SPI_RD_BYTE_BUF		0xC5
#define SPI_RDWR_BYTE_LIT   0xC4
#define SPI_RDWR_BYTE_BUF	0xC3

#define ACK_BYTE            0x00
#define NO_ACK_BYTE         0x0F

#define STATUSHI_ERRMASK    0xFE
