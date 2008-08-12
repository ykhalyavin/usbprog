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
/*
  if (CLOCK_DATA_BYTES_OUT==jtagcmd.actual_cmd) {
  
  } else if (CLOCK_DATA_BITS_OUT==jtagcmd.actual_cmd) {

  } else if (CLOCK_DATA_BYTES_IN==jtagcmd.actual_cmd) {

  } else if (CLOCK_DATA_BITS_IN==jtagcmd.actual_cmd) {

  } else if (CLOCK_DATA_BYTES_OUT_IN==jtagcmd.actual_cmd) {

  } else if (CLOCK_DATA_BITS_OUT_IN==jtagcmd.actual_cmd) {

  } else if (CLOCK_DATA_BIT_TMS_TDI_1==jtagcmd.actual_cmd) {

  } else if (CLOCK_DATA_BIT_TMS_TDI_0==jtagcmd.actual_cmd) {

  } else if (CLOCK_DATA_TMS_TDI_1_WIDTH_READ==jtagcmd.actual_cmd) {

  } else if (CLOCK_DATA_TMS_TDI_0_WIDTH_READ==jtagcmd.actual_cmd) {

  } else {

  }
*/
}



void bit_in(uint8_t byte, int length, char * in)
{
  for(length;length>0;length--){

   if(SET_TDO())
      in[0]|= 1;
    
    in[0] = in[0] << 1;

  }
}



void bit_out(uint8_t byte, int length)
{
  for(length;length>0;length--){
    
    JTAG_SET_CLOCK();
    
    if(byte&1)
      SET_TDI();
    else
      CLEAR_TDI();
    byte = byte >> 1;

    JTAG_SPEED();

    JTAG_CLEAR_CLOCK();
    
    JTAG_SPEED();
  }
}



void bit_out_in(uint8_t byte, int length, char * in)
{
  for(length;length>0;length--){
    if(byte&1)
      SET_TDI();
    else
      CLEAR_TDI();
    byte = byte >> 1;

    if(SET_TDO())
      in[0]|= 1;
    
    in[0] = in[0] << 1;

    JTAG_SPEED();
  }
 //in[0] = 0x88;  // for result of actual byte
}


void bit_in_tms(uint8_t byte, int length, char * in,  int tdi)
{
  if(tdi==1) SET_TDI(); else CLEAR_TDI();

}

void bit_out_tms(uint8_t byte, int length, int tdi)
{
  if(tdi==1) SET_TDI(); else CLEAR_TDI();

  for(length;length>0;length--){
    if(byte&1)
      SET_TMS();
    else
      CLEAR_TMS();
    byte = byte >> 1;
    JTAG_SPEED();
  }

}


void bit_out_in_tms(uint8_t byte, int length, char * in, int tdi)
{
  if(tdi==1) SET_TDI(); else CLEAR_TDI();

}



