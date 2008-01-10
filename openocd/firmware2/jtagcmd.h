#include <stdint.h>

// buffer for incoming request and outgoing repsonse
volatile unsigned char vendorrequest[8];

volatile struct jtagcmd_t {
  char jtagcmdbuf_rx[320];  // receive buffer
  char jtagcmdbuf_tx[320];  // transmit buffer
  short tx_length;	    // transmit length
  short rx_length;	    // receive length
  short tx_index;	    // transmit index
  short rx_index;	    // receive index
  unsigned char actual_cmd; // actual cmd
} jtagcmd;


void gpio();

void scan();

void led(int signal);

void speed();

