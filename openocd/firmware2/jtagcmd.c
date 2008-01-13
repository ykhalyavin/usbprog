#include <stdint.h>

#include "jtagcmd.h"

/*
 *
 * // buffer for incoming request and outgoing repsonse
 * volatile unsigned char vendorrequest[8];

 * volatile struct jtagcmd_t {
 *   char jtagcmdbuf_rx[320];  // receive buffer
 *   char jtagcmdbuf_tx[320];  // transmit buffer
 *   short tx_length;	    // transmit length
 *   short rx_length;	    // receive length
 *   short tx_index;	    // transmit index
 *   short rx_index;	    // receive index
 *   unsigned char actual_cmd; // actual cmd
 * } jtagcmd;
 */

void gpio(){


}

void scan_gpio_command(){

  if (CLOCK_DATA_BYTES_OUT==jtagcmd.actual_cmd) {
  
  } else if (CLOCK_DATA_BITS_OUT==jtagcmd.actual_cmd) {

  } else if (CLOCK_DATA_BYTES_IN==jtagcmd.actual_cmd) {

  } else if (CLOCK_DATA_BITS_IN==jtagcmd.actual_cmd) {

  } else if (CLOCK_DATA_BYTES_OUT_IN==jtagcmd.actual_cmd) {

  } else if (CLOCK_DATA_BITS_OUT_IN==jtagcmd.actual_cmd) {

  } else if (CLOCK_DATA_BIT_TMS_TDI_1==jtagcmd.actual_cmd) {

  } else if (CLOCK_DATA_BIT_TMS_TDI_0==jtagcmd.actual_cmd) {

  } else if (CLOCK_DATA_BIT_TMS_TDI_1_WIDTH_READ==jtagcmd.actual_cmd) {

  } else if (CLOCK_DATA_BIT_TMS_TDI_0_WIDTH_READ==jtagcmd.actual_cmd) {

  } else {

  }

}


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

}


void led(int signal){


}

void speed(){


}


void clock_data_bytes_out()
{

}


void clock_data_bits_out()
{

}

void clock_data_bytes_in()
{

}

void clock_data_bits_in()
{

}

void clock_data_bytes_out_in()
{

}

void clock_data_bits_out_in()
{

}

void clock_data_bit_tms_tdi_1()
{

}

void clock_data_bit_tms_tdi_0()
{

}


void clock_data_bit_tms_tdi_1_read()
{

}

void clock_data_bit_tms_tdi_0_read()
{

}

