#define  F_CPU   16000000L

void interrupt_ep_send(void);
void rs232_send(void);

struct usb_cdc_line_coding {
	char	dwDTERrate[4];   //data terminal rate, in bits per second
	char    bCharFormat;  //num of stop bits (0=1, 1=1.5, 2=2)
	char    bParityType;  //parity (0=none, 1=odd, 2=even, 3=mark, 4=space)
	char    bDataBits;    //data bits (5,6,7,8 or 16)
};

enum {
	SEND_ENCAPSULATED_COMMAND = 0,
	GET_ENCAPSULATED_RESPONSE,
	SET_COMM_FEATURE,
	GET_COMM_FEATURE,
	CLEAR_COMM_FEATURE,
	SET_LINE_CODING = 0x20,
	GET_LINE_CODING,
	SET_CONTROL_LINE_STATE,
	SEND_BREAK
};
