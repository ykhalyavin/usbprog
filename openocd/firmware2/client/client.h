#include <usb.h>

/* vendor requests */

#define FIRMWARE_UPDATE	0x01
#define SET_SPEED	0x02
#define GET_SPEED	0x03
#define USER_INTERFACE	0x04
#define GET_VERSION	0x05


/* USER_INTERFACE wValues */
#define LED_ON	    0x01
#define LED_OFF	    0x02
#define GET_JUMPER  0x03


/* SPEED wValues */
#define 6_KHZ	  0x0006
#define 100_KHZ	  0x0064
#define 250_KHZ	  0x00FA
#define 500_KHZ	  0x01F4
#define 1_MHZ	  0x03EB
#define 5 MHZ	  0x1388


/* JTAG COMMANDS for command buffer */
					      /* 0 0 1 T B R W V (T:1=TDI;T:0=TMS, B:1=Byte;B:0=Bit) */
#define CLOCK_DATA_BYTES_OUT		0x3A  /* 0 0 1 1 1 0 1 0 */
#define CLOCK_DATA_BITS_OUT		0x32  /* 0 0 1 1 0 0 1 0 */
#define CLOCK_DATA_BYTES_IN		0x3C  /* 0 0 1 1 1 1 0 0 */
#define CLOCK_DATA_BITS_IN		0x34  /* 0 0 1 1 0 1 0 0 */
#define CLOCK_DATA_BYTES_OUT_IN		0x3E  /* 0 0 1 1 1 1 1 0 */
#define CLOCK_DATA_BITS_OUT_IN		0x36  /* 0 0 1 1 0 1 1 0 */
#define CLOCK_DATA_BIT_TMS_TDI_1	0x23  /* 0 0 1 0 0 0 1 1 */
#define CLOCK_DATA_BIT_TMS_TDI_0	0x22  /* 0 0 1 0 0 0 1 0 */

#define CLOCK_DATA_TMS_TDI_1_WITH_READ	0x27  /* 0 0 1 0 0 1 1 1 */
#define CLOCK_DATA_TMS_TDI_0_WITH_READ	0x26  /* 0 0 1 0 0 1 1 0 */

/* GPIO COMMANDS for command buffer */

#define SET_TDI(x)  (0x05 | (1<<1) | (x > 0) ? 1:0)
#define SET_TMS(x)  (0x05 | (2<<1) | (x > 0) ? 1:0)
#define SET_TCK(x)  (0x05 | (3<<1) | (x > 0) ? 1:0)
#define SET_TRST(x) (0x05 | (4<<1) | (x > 0) ? 1:0)
#define SET_SRST(x) (0x05 | (5<<1) | (x > 0) ? 1:0)

#define GET_TDO() (0x04 | (6<<1))


/* open connection */
usb_dev_handle *usbprog_locate(void);

/* close connection */
int usbprog_close(usb_dev_handle * usbprog_handle);

/* transmit and receive command buffer */
int usbprog_command_buffer(usb_dev_handle * usbprog_handle, char *read_buffer, int read_length, char *write_buffer, int write_length);

/* control trst signal 0:low, 1:high */
int usbprog_trst(usb_dev_handle * usbprog_handle, int value);

/* control trst signal 0:low, 1:high */
int usbprog_srst(usb_dev_handle * usbprog_handle, int value);

/* control led 0:off, 1:on */
int usbprog_led(usb_dev_handle * usbprog_handle, int value);

/* value = kHz (6 = kHz, 5000 = 5 MHz) */
int usbprog_speed(usb_dev_handle * usbprog_handle, int value);

