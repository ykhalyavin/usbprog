#include <stdint.h>

// buffer for incoming request and outgoing repsonse
volatile unsigned char vendorrequest[8];

/* vendor requests */

#define FIRMWARE_UPDATE 0x01
#define SET_SPEED       0x02
#define GET_SPEED       0x03
#define USER_INTERFACE  0x04
#define GET_VERSION     0x05


/* USER_INTERFACE wValues */
#define LED_ON      0x01
#define LED_OFF     0x02
#define GET_JUMPER  0x03


volatile struct jtagcmd_t {
  char jtagcmdbuf_rx[320];  // receive buffer
  char jtagcmdbuf_tx[320];  // transmit buffer
  short tx_length;	    // transmit length
  short rx_length;	    // receive length
  short tx_index;	    // transmit index
  short rx_index;	    // receive index
  unsigned char actual_cmd; // actual cmd
  short speed;		    // actual speed
} jtagcmd;

#define CLOCK_DATA_BYTES_OUT            0x3A  /* 0 0 1 1 1 0 1 0 */
#define CLOCK_DATA_BITS_OUT             0x32  /* 0 0 1 1 0 0 1 0 */
#define CLOCK_DATA_BYTES_IN             0x3C  /* 0 0 1 1 1 1 0 0 */
#define CLOCK_DATA_BITS_IN              0x34  /* 0 0 1 1 0 1 0 0 */
#define CLOCK_DATA_BYTES_OUT_IN         0x3E  /* 0 0 1 1 1 1 1 0 */
#define CLOCK_DATA_BITS_OUT_IN          0x36  /* 0 0 1 1 0 1 1 0 */
#define CLOCK_DATA_BIT_TMS_TDI_1        0x23  /* 0 0 1 0 0 0 1 1 */
#define CLOCK_DATA_BIT_TMS_TDI_0        0x22  /* 0 0 1 0 0 0 1 0 */

#define CLOCK_DATA_TMS_TDI_1_WITH_READ  0x27  /* 0 0 1 0 0 1 1 1 */
#define CLOCK_DATA_TMS_TDI_0_WITH_READ  0x26  /* 0 0 1 0 0 1 1 0 */

void gpio();

void scan_gpio_command();

void led(int signal);

void speed();

void clock_data_bytes_out();
void clock_data_bits_out();

void clock_data_bytes_in();
void clock_data_bits_in();

void clock_data_bytes_out_in();
void clock_data_bits_out_in();

void clock_data_bit_tms_tdi_1();
void clock_data_bit_tms_tdi_0();

void clock_data_bit_tms_tdi_1_read();
void clock_data_bit_tms_tdi_0_read();
