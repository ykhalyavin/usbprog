/*
* usbprog - A Downloader/Uploader for AVR device programmers
* Copyright (C) 2006 Benedikt Sauter
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>


//#define AT89MODE

#define F_CPU 16000000
#include <util/delay.h>

#define DEBUG_ON 0

#include "wait.h"

#include "../usbprog_base/firmwarelib/avrupdate.h"
#include "uart.h"
#include "usbn2mc.h"

/* command descriptions for mk2 */
#include "avr069.h"

#include <avr/pgmspace.h>

#include <avr/eeprom.h>

#ifndef EEMEM
// alle Textstellen EEMEM im Quellcode durch __attribute__ ... ersetzen
#define EEMEM  __attribute__ ((section (".eeprom")))
#endif

uint8_t ee_sck_duration EEMEM = 1;
uint16_t sck_delay_value;

/**Structures to parse incoming packets more clearly
 */

struct cmd_enter_progmode_s {
  unsigned char id;
  unsigned char timeout;
  unsigned char stabdelay;
  unsigned char cmdexedelay;
  unsigned char synchloops;
  unsigned char bytedelay;
  unsigned char pollvalue;
  unsigned char pollindex;
  unsigned char cmd1;
  unsigned char cmd2;
  unsigned char cmd3;
  unsigned char cmd4;
};

struct cmd_spi_multi_s {
  unsigned char id;
  unsigned char numTx;
  unsigned char numRx;
  unsigned char rxStart;
  unsigned char txData[256]; //This can only be used if multiple-packet commands are supported
};

#define DDR_SPI     DDRB
#define SS PB4
#define MOSI  PB5
#define MISO  PB6
#define SCK   PB7
#define RESET_PIN   PB0
#define RESET_PORT  PORTB
#define LED_PIN     PA4
#define LED_PORT    PORTA

#define LED_on     (LED_PORT   |=  (1 << LED_PIN))   // red led
#define LED_off    (LED_PORT   &= ~(1 << LED_PIN))
#define RESET_high (RESET_PORT |=  (1 << RESET_PIN))
#define RESET_low  (RESET_PORT &= ~(1 << RESET_PIN)) // reset

/*** prototypes and global vars ***/
/* send a command back to pc */
void CommandAnswer(int length);

volatile struct usbprog_t {
  char lastcmd;
  int longpackage;
  int cmdpackage;
  int datatogl;
  unsigned int sck_duration;
  int fragmentnumber;
  int avrstudio;
  int reset_pol;
} usbprog;

#define _BUF_LEN     300
#define _TMP_OFFSET  32
volatile char answer[_BUF_LEN];

struct pgmmode_t {
  unsigned short numbytes;
  uint8_t mode;
  uint8_t delay;
  uint8_t cmd1;
  uint8_t cmd2;
  uint8_t cmd3;
  uint8_t poll1;
  uint8_t poll2;
  uint32_t address;
  uint32_t pageaddress;
} pgmmode;


unsigned short sck_lookup[]PROGMEM =
{
    0, 0, 0, 0, 2, 6, 14, 18, 20, 21, 23, 24, 26, 27, 29, 30, 32, 33, 36, 38,
    39, 41, 44, 45, 48, 50, 53, 56, 59, 62, 65, 68, 71, 75, 78, 83, 87, 92, 96,
    101, 105, 111, 116, 122, 128, 135, 141, 149, 156, 164, 171, 180, 189, 198,
    209, 218, 230, 240, 252, 264, 278, 291, 306, 321, 338, 354, 371, 390, 408,
    429, 450, 473, 495, 521, 546, 573, 602, 630, 662, 695, 729, 763, 803, 842,
    884, 927, 972, 1020, 1069, 1122, 1177, 1236, 1297, 1360, 1428, 1498, 1571,
    1648, 1729, 1814, 1904, 1998, 2096, 2198, 2307, 2419, 2539, 2664, 2795,
    2930, 3074, 3229, 3387, 3550, 3729, 3911, 4104, 4299, 4512, 4737, 4973,
    5206, 5462, 5728, 6022, 6307, 6620, 6942, 7297, 7660, 8030, 8401, 8847,
    9257, 9706, 10202, 10693, 11233, 11762, 12343, 12985, 13603, 14283, 14923,
    15623, 16391, 17239, 18016, 19045, 19998, 20962, 22000, 23092, 24211, 25410,
    26664, 27970, 29323, 30767, 32308, 33896, 35521, 37311, 39136
};

#define MAX_SCK_DURATION ((sizeof(sck_lookup)/sizeof(unsigned short))-1)



SIGNAL(SIG_INTERRUPT0)
{
  USBNInterrupt();
}

/* id need for live update of firmware */
void USBNDecodeVendorRequest(DeviceRequest *req)
{
  if(req->bRequest == STARTAVRUPDATE)
    avrupdate_start();
}



void spi_init(void)
{
  usbprog.sck_duration = eeprom_read_byte(&ee_sck_duration);
  if(usbprog.sck_duration > MAX_SCK_DURATION)  // prevent error when EEPROM is erased
    usbprog.sck_duration = 0x06;  //  125kHz as default

  PORTB   &=  ~((1 << SCK)|(1<<MISO)|(1<<MOSI)/*|(1<<RESET_PIN)*/);  // SCK have to be low in IDLE
  DDR_SPI &=~(1 << MISO);
  DDR_SPI = (1 << MOSI)|(1 << SCK)|(1 << RESET_PIN);
  // switches back to slave mode !
  DDR_SPI |= (1<<SS); // make SS an output for SPI master.

  SPCR = 0;
  SPSR = 0;

  //This delay value is also required in hardware SPI mode in spi_pulseclockonce
  sck_delay_value=pgm_read_word(&(sck_lookup[usbprog.sck_duration]));

    switch(usbprog.sck_duration)
    {
       case 0x00:  //08MHz
        SPCR = (1<<SPE)|(1<<MSTR);
        SPSR = (1<<SPI2X);
        break;

      case 0x01:  //04MHz
        SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1);
        SPSR = (1<<SPI2X);
        break;

      case 0x02:  //02MHz
        SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
        SPSR = (1<<SPI2X);
        break;

      case 0x03:	//01MHz
        SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
        SPSR = 0x00;
        break;

      case 0x04:  //500kHz
        SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1);
        SPSR = (1<<SPI2X);
        break;

      case 0x05:  //250kHz
        SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1);
        SPSR = 0x00;
        break;

      case 0x06:  //125kHz
        SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1);
        SPSR = 0x00;
        break;

	  default:
        PORTB   &=  ~(1 << SCK);
        DDR_SPI &=~(1 << MISO);
        DDR_SPI = (1 << MOSI)|(1 << SCK)|(1 << RESET_PIN);
		break;
    }
}


void spi_idle(void)
{
  //DDR_SPI = 0x00;
  DDR_SPI &= ~( (1<<MOSI) | (1<<SCK) | (1<<RESET_PIN) ); // don't make SS an input !
  //PORTB = 0xFF;	// only temp for jar
  PORTB = 0x00; // holger klabunde: tri state without pullups
}

void spi_active(void)
{
  PORTB   &=  ~((1<<MISO)|(1 << MOSI)|(1 << SCK)|(1 << RESET_PIN));  // switch off unused Pullup resistors
  DDR_SPI &=~(1 << MISO);
  DDR_SPI |= (1 << MOSI)|(1 << SCK)|(1 << RESET_PIN);
}

/** Full-duplex SPI communication. For sck_duration<=6, use hardware SPI. For
 * >=6, use software spi.
 * From AT90S2313 datasheet, timing is:
 * set data, wait for 1/2 period, clock high, wait another
 * 1/2 period, read data, clock low. In other words: AVR clocks in on rising
 * edge, AVRISP clocks in on falling edge
 *
 * However, for AT89xx (AT89S51, chapter 21), there is a setup period from data
 * to sck high and also a hold period from sck low to data change. Therefore,
 * sck and mosi run 90 degree out of phase:
 * -Setup data, wait
 * -SCK high, wait
 * -Read data, wait
 * -SCK low, wait
 *
 * This guarantees compatibility with both AVR and AT89 controllers, when
 * soft SPI mode is used.
 */

unsigned char spi_inout(unsigned char data)
{
  unsigned char bitvalue,din=0;

  if(usbprog.sck_duration <= 6)
  {
    SPDR = data;
    while ( !(SPSR & (1 << SPIF)) ) ;
    return SPDR;
  }
  else    // software SPI with delay
  {
    for (bitvalue=128;bitvalue;bitvalue>>=1)
    {
      //set new data
      if(data & bitvalue)
        PORTB   |=  (1 << MOSI);
      else
        PORTB   &=  ~(1 << MOSI);

      //wait 1/4 period
      _delay_loop_2(sck_delay_value>>1);

      //set clock high
      PORTB   |=  (1 << SCK);

      //wait another 1/4 period
      _delay_loop_2(sck_delay_value>>1);

      //read in bit
      if( PINB & (1<<MISO)) {
        din|=bitvalue;
      }

      //wait another 1/4 period
      _delay_loop_2(sck_delay_value>>1);

      //clear clock
      PORTB   &=  ~(1 << SCK);

      //wait another 1/4 period
      _delay_loop_2(sck_delay_value>>1);

    }
    return din;
  }
}

/**According to the syncronisation protocol recommended by atmel, the SCLK must
 * be pulsed once every retry to try to get in sync. This can't be done in
 * hardware spi mode, so switch to software temporarily
 */

void spi_pulseclockonce(void) {
  unsigned char spcr_save,spsr_save;
  spcr_save=SPCR;
  spsr_save=SPSR;
  SPCR = 0;
  SPSR = 0;
  //pulse
  _delay_loop_2(sck_delay_value);
  PORTB   |=  (1 << SCK);
  _delay_loop_2(sck_delay_value);
  PORTB   &=  ~(1 << SCK);

  SPCR=spcr_save;
  SPSR=spsr_save;
}

void spi_out(char data) {
  spi_inout(data);
}
char spi_in(void) {
  return spi_inout(0);;
}

void word_mode_write_wait(unsigned char cmd,unsigned char addrh,
    unsigned char addrl, unsigned char expect) {

  uint8_t delay=pgmmode.delay;

  if ((expect==pgmmode.poll1) | (pgmmode.mode & 1)) {
    //If the value written equals the polling value, polling is not possible
    //Also if timed delay is requested, do timed delay
    wait_ms(pgmmode.delay);
  } else {
    //Read back the flash. If programming is completed, the read back should be
    //the expected value. Time out as given by pgmmode.delay
    while (delay--) {
      wait_ms(1);
      spi_out(cmd);
      spi_out(addrh);
      spi_out(addrl);
      if (spi_in()==expect) return;
    }
  }

}
void spi_write_program_memory(unsigned short wordaddr,char wordl,char wordh)
{
  char addrh,addrl;

  addrh = (char)(wordaddr >> 8);
  addrl = (char)(wordaddr);

  // write low
  spi_out(pgmmode.cmd1);
  spi_out(addrh);
  spi_out(addrl);
  spi_out(wordl);

  //In word mode, wait for programming
  if (!(pgmmode.mode & 1)) word_mode_write_wait(pgmmode.cmd3,addrh,addrl,wordl);

  // write high
  spi_out(pgmmode.cmd1|8);
  spi_out(addrh);
  spi_out(addrl);
  spi_out(wordh);

  //In word mode, wait for programming
  if (!(pgmmode.mode & 1)) word_mode_write_wait(pgmmode.cmd3|8,addrh,addrl,wordh);

}

/* programm finite state machine
*
*/
void flash_program_fsm(char * buf)
{
  int i = 0, datastart=0, databytes = 0;

  datastart=usbprog.cmdpackage ? 10:0; //Number of command bytes in this packet
  databytes=64-datastart;             //Max. number of data bytes in this packet

  if (pgmmode.numbytes>databytes) {
    pgmmode.numbytes-=databytes;
    usbprog.longpackage=1;        //Expect more data
  } else {
    databytes=pgmmode.numbytes;   //The number of valid bytes in this packet
    pgmmode.numbytes=0;           //No more data left
    usbprog.longpackage=0;        //Expect no more data
  }

  //Write the program memory bytes
  for(i=0; i < databytes; i+=2){
    spi_write_program_memory(pgmmode.address, buf[datastart+i], buf[datastart + i + 1]);
    pgmmode.address++;
  }

  if (pgmmode.numbytes==0) {
    //If in page mode, and the write bit is set, perform page write
    if ((pgmmode.mode & 0x1) && (pgmmode.mode & 0x80)) {
      // write page to flash
       spi_out(pgmmode.cmd2);
       spi_out((char)(pgmmode.pageaddress >> 8));  //high-byte of page address
       spi_out((char)(pgmmode.pageaddress));        //low-byte  of page address
       spi_out(0x00);
       wait_ms(20); //Timed delay, TODO: other modes
    }

    // acknowlegde to host
    answer[0] = CMD_PROGRAM_FLASH_ISP;
    answer[1] = STATUS_CMD_OK;
    CommandAnswer(2);
  }
}

unsigned char eeprom_read_byte_isp(unsigned char high, unsigned char low)
{
  unsigned char res;

  spi_out(pgmmode.cmd3);
  spi_out(pgmmode.address >> 8);
  spi_out(pgmmode.address);
  res = spi_in();
  return(res);
}

void eeprom_write_byte_isp(unsigned char high, unsigned char low, unsigned char value)
{
  spi_out(0xc0);
  spi_out(high);
  spi_out(low);
  spi_out(value);
  wait_ms(10);
return;
}

unsigned int _tmp_buf_index = 0;

void eeprom_write(char * buf){

unsigned int bufindex = 0, end_index = 0, i;

  if(usbprog.cmdpackage == 1){
    bufindex = 10;    // first packet, skip the packet header
    usbprog.cmdpackage = 0;
    if (pgmmode.numbytes >= (_BUF_LEN - _TMP_OFFSET)) {
      answer[0] = CMD_PROGRAM_EEPROM_ISP;  // we are not able to program more than this
      answer[1] = STATUS_CMD_FAILED;       // in one cycle, because we haven't enough RAM
      CommandAnswer(2);            // AVR-Studio sends max. 128 Bytes in one USB-Packet
    }                      // so this should never happen...
    else {
      _tmp_buf_index = _TMP_OFFSET;    // reserve some space at beginning of the buffer to handle
                      // answer packets during programming if necessary
    }
  }

  if(pgmmode.numbytes > (64 - bufindex)) {
    end_index = 64;
    pgmmode.numbytes = pgmmode.numbytes - 64 + bufindex;
    usbprog.longpackage = 1; // we do expect more data from FIFO
  }
  else {
    end_index = pgmmode.numbytes + bufindex;
    pgmmode.numbytes = 0;
    usbprog.longpackage = 0; // we do not expect mor data from FIFO
  }
  for(; bufindex < end_index; _tmp_buf_index++, bufindex++){
    answer[_tmp_buf_index] = buf[bufindex];     // copy the USB-FIFO to temporary memory to speed up the transfer
                        // therefore we will borrow some memory from answer[]-buffer
  }
  // we've received all bytes to programm, so program it and send acknowlegde to host
  // perhaps this should be done in the idle-loop to accept other commands during programming
  // because it'll take 10ms per byte, we'll see...
  if (pgmmode.numbytes == 0){
    for(i = _TMP_OFFSET; i < _tmp_buf_index; i++, pgmmode.address++){
      eeprom_write_byte_isp((unsigned char)(pgmmode.address >> 8), (unsigned char)pgmmode.address, answer[i]);
    }
    answer[0] = CMD_PROGRAM_EEPROM_ISP;
    answer[1] = STATUS_CMD_OK;
    CommandAnswer(2);
  }
}

void USBToglAndSend(void)
{
  if(usbprog.datatogl == 1) {
    USBNWrite(TXC1, TX_LAST+TX_EN+TX_TOGL);
    usbprog.datatogl = 0;
  } else {
    USBNWrite(TXC1, TX_LAST+TX_EN);
    usbprog.datatogl = 1;
  }
}

void SendCompleteAnswer(void)
{
  if(pgmmode.numbytes == 0)
    return;

  USBNWrite(TXC1, FLUSH);

  int i;
  if(pgmmode.numbytes > 64){
    for(i = 0; i < 64; i++)
      USBNWrite(TXD1, answer[usbprog.fragmentnumber * 64 + i]);

    usbprog.fragmentnumber++;
    pgmmode.numbytes = pgmmode.numbytes - 64;
    USBToglAndSend();
  }
  else {
    for(i = 0; i<pgmmode.numbytes; i++)
      USBNWrite(TXD1, answer[usbprog.fragmentnumber * 64 + i]);

    usbprog.fragmentnumber = 0;
    pgmmode.numbytes = 0;
    USBToglAndSend();
  }
}

#if 0
void NackEvent(unsigned int number)
{
  if(number & 0x20) 
    USBNWrite(TXC1, FLUSH);

  if(number & 0x02) 
    USBNWrite(RXC1, FLUSH+RX_EN);
    USBNWrite(TXC1, FLUSH);
    if(usbprog.datatogl == 1) {
      USBNWrite(TXC1, TX_LAST+TX_EN+TX_TOGL);
      usbprog.datatogl=0;
    } else {
      USBNWrite(TXC1, TX_LAST+TX_EN);
      usbprog.datatogl=1;
    }
  }
}
#endif


void CommandAnswer(int length)
{
  int i;

  USBNWrite(TXC1, FLUSH);
  for(i = 0; i < length; i++)
    USBNWrite(TXD1, answer[i]);

  /* control togl bit */
  USBToglAndSend();
}

/** Enter ISP programming mode
 */
void cmd_enter_progmode(struct cmd_enter_progmode_s *cmd) {
  #if DEBUG_ON
  UARTWrite("enter\r\n");
  #endif
  #if DEBUG_ON
  int i;
  for (i=0;i<12;i++) {
    SendHex(*((char *)cmd+i));
    UARTPutChar(' ');
  }
  SendHex(cmd->cmd1);
  #endif
  pgmmode.address = 0;
  spi_active();
  LED_on;

  PORTB &= ~(1<<SCK);

  if(usbprog.reset_pol==1)
  {
    PORTB &= ~(1<<RESET_PIN);
    wait_ms(10);
    PORTB |= (1<<RESET_PIN);  // give reset a positive pulse
    wait_ms(10);
    PORTB &= ~(1<<RESET_PIN);
    wait_ms(10);
    
    #ifdef AT89MODE 
    PORTB |= (1<<RESET_PIN);  // give reset a positive pulse
    wait_ms(10);
    #endif
  }
  else
  {
    PORTB |= (1<<RESET_PIN);  // give reset a positive pulse
    wait_ms(10);
    PORTB &= ~(1<<RESET_PIN);
    wait_ms(10);
    PORTB |= (1<<RESET_PIN);  // give reset a positive pulse
    wait_ms(10);
  }

  answer[0] = CMD_ENTER_PROGMODE_ISP;

  wait_ms(cmd->cmdexedelay);

  int syncloops = cmd->synchloops;
  unsigned char result3,result4;
  for (;syncloops > 0; syncloops--) {
    spi_out(cmd->cmd1);
    wait_ms(cmd->bytedelay);
    spi_out(cmd->cmd2);
    wait_ms(cmd->bytedelay);
    result3 = spi_inout(cmd->cmd3);
    wait_ms(cmd->bytedelay);
    result4 = spi_inout(cmd->cmd4);
    wait_ms(cmd->bytedelay);

    if ((cmd->pollindex==0) ||
        ((cmd->pollindex==3) && (result3==cmd->pollvalue)) ||
        ((cmd->pollindex==4) && (result4==cmd->pollvalue))) {
      answer[1] = STATUS_CMD_OK;
      CommandAnswer(2);
      return;
    }

    //Apparently, we're not in sync. Pulse SCK once to get in sync
    #if DEBUG_ON
    UARTPutChar('#');
    #endif
    spi_pulseclockonce();
  };

  answer[1] = STATUS_CMD_FAILED;
  CommandAnswer(2);
  return;
}

/** Send/receive multiple spi bytes
 */
void cmd_spi_multi(struct cmd_spi_multi_s *cmd) {
  char send,receive;
  /*numsent *must* be int because char may overflow if numRx+rxStart
    becomes too large to fit in char */

  int numsent=0,numrecv=0;
  if (cmd->numTx>60||cmd->numRx>60) {
    //Not implemented
    answer[0] = CMD_SPI_MULTI;
    answer[1] = STATUS_CMD_UNKNOWN;
    CommandAnswer(2);
  } else {
    for(numsent=0;(numsent<cmd->numTx)||(numrecv<cmd->numRx);numsent++) {
      if (numsent<cmd->numTx) send=cmd->txData[numsent]; else send=0;
      receive=spi_inout(send);
      if (numsent>=cmd->rxStart) {
        //past this point, the data received from the AVR must be
        //sent to the USB
        answer[numrecv+2]=receive;
        numrecv++;
      }
    }
    answer[0] = CMD_SPI_MULTI;
    answer[1] = STATUS_CMD_OK;

    answer[numrecv+2] = STATUS_CMD_OK;
    CommandAnswer(numrecv+3);
  }

}


/* central command parser */
void USBFlash(char *buf)
{
  char result = 0;
  int numbytes;
  int i;
  
  USBNWrite(TXC1, 0x00);
  
  #if DEBUG_ON
  UARTWrite("cmd ");
  SendHex(buf[0]);
  UARTWrite("cmd\r\n");
  #endif
  USBNWrite(TXC1, FLUSH);
  // first see if this packet is expected by Flash or EEPROM programming
  if(usbprog.longpackage) {
    if(usbprog.lastcmd == CMD_PROGRAM_FLASH_ISP) {// last operation was flash programming
      flash_program_fsm(buf);
    }

    if(usbprog.lastcmd == CMD_PROGRAM_EEPROM_ISP) {// last operation was eeprom programming
      eeprom_write(buf);
    }
    return;
  }

  // if not, this is a command packet, we will decode here
  else {
    usbprog.lastcmd = buf[0]; // store current command for later use
    switch(buf[0]) {
    
    case CMD_SIGN_ON:


      answer[0] = CMD_SIGN_ON;
      answer[1] = STATUS_CMD_OK;
      answer[2] = 10; // fixed length
      answer[3] = 'A';
      answer[4] = 'V';
      answer[5] = 'R';
      answer[6] = 'I';
      answer[7] = 'S';
      answer[8] = 'P';
      answer[9] = '_';
      answer[10] = 'M';
      answer[11] = 'K';
      answer[12] = '2';
      CommandAnswer(13);

      return;
    break;
    case CMD_SET_PARAMETER:
      // do we like, all commands are successfully
      answer[0] = CMD_SET_PARAMETER;
      answer[1] = STATUS_CMD_OK;

      switch(buf[1]){
        case PARAM_RESET_POLARITY:
          if(buf[2]==0x00)
            usbprog.reset_pol=0;
          else
            usbprog.reset_pol=1;

        break;

        case PARAM_SCK_DURATION:
	  #if DEBUG_ON
          UARTPutChar('S');
          SendHex(buf[2]);
	  #endif
          if (buf[2]>MAX_SCK_DURATION) buf[2]=MAX_SCK_DURATION;
          if (buf[2] != usbprog.sck_duration)
          {
            usbprog.sck_duration = buf[2];
            eeprom_write_byte(&ee_sck_duration, usbprog.sck_duration);
          }
          spi_init();
          break;
      }

      CommandAnswer(2);
      return;
    break;
    case CMD_GET_PARAMETER:
      answer[0] = CMD_GET_PARAMETER;
      answer[1] = STATUS_CMD_OK;

      switch(buf[1]){
        case PARAM_STATUS_TGT_CONN:
          answer[2] = STATUS_ISP_READY;
        break;

        case PARAM_SW_MAJOR:  // avrisp mkII special 1
          answer[2] = 9;
        break;

        case PARAM_SW_MINOR:  // abrisp mkII special 6
          answer[2] = 9;
        break;

        case PARAM_HW_VER:
          answer[2] = 0;
        break;




        case PARAM_VTARGET:
          answer[2] = 50;
        break;

        case PARAM_SCK_DURATION:
          answer[2] = usbprog.sck_duration;
        break;

        default:
          answer[2] = 0x00; // FIXME all is not perfect!
      }
      CommandAnswer(3);
      return;

    break;
    case CMD_OSCCAL:
      /* peforms a calibration secquence */
      answer[0] = CMD_OSCCAL;
      answer[1] = STATUS_CMD_OK;
      CommandAnswer(2);
      return;
    break;

    case CMD_READ_OSCCAL_ISP:
      spi_out(buf[2]);
      spi_out(buf[3]);
      spi_out(buf[4]);
      result = spi_in();

      answer[0] = CMD_READ_OSCCAL_ISP;
      answer[1] = STATUS_CMD_OK;
      answer[2] = result;
      answer[3] = STATUS_CMD_OK;
      CommandAnswer(4);
      return;

    break;

    case CMD_LOAD_ADDRESS:
      // set given address
      pgmmode.address = buf[1];
      pgmmode.address = (pgmmode.address << 8) | buf[2];
      pgmmode.address = (pgmmode.address << 8) | buf[3];
      pgmmode.address = (pgmmode.address << 8) | buf[4];
      answer[0] = CMD_LOAD_ADDRESS;
      answer[1] = STATUS_CMD_OK;
      CommandAnswer(2);
      return;
    break;

    case CMD_FIRMWARE_UPGRADE:
      avrupdate_start();
      return;
    break;

    case CMD_RESET_PROTECTION:
      answer[0] = CMD_RESET_PROTECTION;
      answer[1] = STATUS_CMD_OK;  // this command returns always ok!
      return;
    break;

    case CMD_ENTER_PROGMODE_ISP:
      cmd_enter_progmode((struct cmd_enter_progmode_s *)buf);
      return;

      break;

    case CMD_LEAVE_PROGMODE_ISP:
      #if DEBUG_ON
      UARTWrite("leave\r\n");
      #endif
      LED_off;
      RESET_high;
      spi_idle();
      answer[0] = CMD_LEAVE_PROGMODE_ISP;
      answer[1] = STATUS_CMD_OK;
      CommandAnswer(2);

      // wenn adapter vom avrdude aus angesteuert wird
      if(usbprog.avrstudio==0)
      usbprog.datatogl=0;  // to be sure that togl is on next session clear 
      //usbprog.datatogl=1;  // to be sure that togl is on next session clear
      //USBNWrite(RXC1, RX_EN);

      return;
    break;

    case CMD_CHIP_ERASE_ISP:
      #if DEBUG_ON
      UARTWrite("erase\r\n");
      #endif
      spi_out(buf[3]);
      spi_out(buf[4]);
      spi_out(0x00);
      spi_out(0x00);
      wait_ms(buf[1]);
      answer[0] = CMD_CHIP_ERASE_ISP;
      answer[1] = STATUS_CMD_OK;
      CommandAnswer(2);
      return;
    break;

    case CMD_PROGRAM_FLASH_ISP:
      #if DEBUG_ON 
      UARTWrite("pflash\r\n");
      for(i=0;i<10;i++) {
        SendHex(buf[i]);
        UARTPutChar(' ');
      }
      #endif
      pgmmode.numbytes = (buf[1] << 8) | (buf[2]);

      // buf[3] = mode
      pgmmode.mode = buf[3];
      // buf[4] = delay
      pgmmode.delay = buf[4];

      // buf[5] = spi command for load page and write program memory (one byte at a time)
      pgmmode.cmd1 = buf[5];
      // buf[6] = spi command for write program memory page (one page at a time)
      pgmmode.cmd2 = buf[6];
      // buf[7] = spi command for read program memory
      pgmmode.cmd3 = buf[7];

      // store the page address (which in fact are the first 5 bits) for page write command
      // because we will increment pgmode.address during transfer
      pgmmode.pageaddress = pgmmode.address;
      usbprog.cmdpackage = 1;
      flash_program_fsm(buf);
      usbprog.cmdpackage = 0;
    break;

    case CMD_READ_FLASH_ISP:
      #if DEBUG_ON
      UARTWrite("rflash\r\n");
      #endif
      pgmmode.numbytes = ((buf[1] << 8) | (buf[2])) + 1; // number of bytes
      pgmmode.cmd3 = buf[3];  // read command
      numbytes = pgmmode.numbytes - 1;
      // collect max first 62 bytes
      int answerindex = 2;
      for(;numbytes > 0; numbytes--) {
        spi_out(pgmmode.cmd3);
        spi_out(pgmmode.address >> 8); // read from word address MSB
        spi_out(pgmmode.address);      // read from word address LSB
        answer[answerindex] = spi_in();
        answerindex++;
        if(pgmmode.cmd3 == 0x20){
          pgmmode.cmd3 = 0x28;
        }
        else {
          pgmmode.cmd3 = 0x20;
          pgmmode.address++;
        }
      }

      // then toggle send next read bytes
      // and finish with status_cmd_ok

      answer[0] = CMD_READ_FLASH_ISP;
      answer[1] = STATUS_CMD_OK;
      answer[pgmmode.numbytes + 1] = STATUS_CMD_OK;

      if(pgmmode.numbytes > 62){
        CommandAnswer(64);
        pgmmode.numbytes = pgmmode.numbytes - 62;
        usbprog.fragmentnumber = 1;
      }
      else {CommandAnswer(pgmmode.numbytes + 2);
      usbprog.datatogl=0; 
      }

      return;
    break;

    case CMD_READ_LOCK_ISP:
      #if DEBUG_ON
      UARTWrite("rlock\r\n");
      #endif
      spi_out(buf[2]);
      spi_out(buf[3]);
      spi_out(buf[4]);
      result = spi_in();

      answer[0] = CMD_READ_LOCK_ISP;
      answer[1] = STATUS_CMD_OK;
      answer[2] = result;
      answer[3] = STATUS_CMD_OK;
      CommandAnswer(4);
      return;
    break;

    case CMD_PROGRAM_LOCK_ISP:
      #if DEBUG_ON
      UARTWrite("plock\r\n");
      #endif
      spi_out(buf[1]);
      spi_out(buf[2]);
      spi_out(buf[3]);
      spi_out(buf[4]);

      answer[0] = CMD_PROGRAM_LOCK_ISP;
      answer[1] = STATUS_CMD_OK;
      answer[2] = STATUS_CMD_OK;
      CommandAnswer(3);
      return;
    break;

    case CMD_PROGRAM_EEPROM_ISP:
      #if DEBUG_ON
      UARTWrite("peerpom\r\n");
      #endif
      // buf[1..2] = number of bytes to program
      pgmmode.numbytes = (buf[1] << 8) | (buf[2]);
      // buf[3] = mode
      pgmmode.mode = buf[3];
      // buf[4] = delay
      pgmmode.delay = buf[4];
      // buf[5] = spi command for load page and write program memory (one byte at a time)
      pgmmode.cmd1 = buf[5];
      // buf[7] = spi command for read program memory
      pgmmode.cmd3 = buf[7];

      usbprog.cmdpackage = 1;
      eeprom_write(buf);
      usbprog.cmdpackage = 0;
      return;
    break;

    case CMD_READ_EEPROM_ISP:
      pgmmode.numbytes = ((buf[1] << 8) | (buf[2])) + 1; // number of bytes
      pgmmode.cmd3 = buf[3];  // read command
      numbytes = pgmmode.numbytes - 1;
      // collect max first 62 bytes
      answerindex = 2;
      for(;numbytes > 0; numbytes--) {
        spi_out(pgmmode.cmd3);
        spi_out(pgmmode.address >> 8);
        spi_out(pgmmode.address);
        answer[answerindex] = spi_in();
        answerindex++;
        pgmmode.address++;
      }

      // then toggle send next read bytes
      // and finish with status_cmd_ok

      answer[0] = CMD_READ_EEPROM_ISP;
      answer[1] = STATUS_CMD_OK;
      
      answer[pgmmode.numbytes + 1] = STATUS_CMD_OK;

      
      if(pgmmode.numbytes > 62){
        CommandAnswer(64);
        pgmmode.numbytes = pgmmode.numbytes - 62;
        usbprog.fragmentnumber = 1;
	return; 
      }
      else {
	CommandAnswer(pgmmode.numbytes + 2);
	if(usbprog.datatogl ==1)
	usbprog.datatogl = 0;
	else
	usbprog.datatogl = 1;
	return; 
      }


    break;

    case CMD_PROGRAM_FUSE_ISP:
      #if DEBUG_ON
      UARTWrite("pfuse\r\n");
      #endif
      spi_out(buf[1]);
      spi_out(buf[2]);
      spi_out(buf[3]);
      spi_out(buf[4]);

      answer[0] = CMD_PROGRAM_FUSE_ISP;
      answer[1] = STATUS_CMD_OK;
      answer[2] = STATUS_CMD_OK;
      CommandAnswer(3);
      return;

    break;

    case CMD_READ_FUSE_ISP:
      #if DEBUG_ON 
      UARTWrite("rfuse\r\n");
      #endif
      spi_out(buf[2]);
      spi_out(buf[3]);
      spi_out(buf[4]);
      result = spi_in();

      answer[0] = CMD_READ_FUSE_ISP;
      answer[1] = STATUS_CMD_OK;
      answer[2] = result;
      answer[3] = STATUS_CMD_OK;
      CommandAnswer(4);
      return;
    break;

    case CMD_READ_SIGNATURE_ISP:
      spi_out(buf[2]);
      spi_out(buf[3]);
      spi_out(buf[4]);
      result = spi_in();

      answer[0] = CMD_READ_SIGNATURE_ISP;
      answer[1] = STATUS_CMD_OK;
      answer[2] = result;
      answer[3] = STATUS_CMD_OK;
      CommandAnswer(4);
      return;
    break;

    case CMD_SPI_MULTI:

      //usbprog.datatogl=0;
      #if DEBUG_ON
      UARTWrite("multi\r\n");
      #endif
      usbprog.avrstudio=0;  // only avrdude use this command // this is not true! also at89 @ avr studio
      cmd_spi_multi((struct cmd_spi_multi_s *)buf);
      return;
    break;
    }
  }
}


int main(void)
{

 int conf, interf;
  #if DEBUG_ON
  UARTInit();
  #endif


  spi_init();
  spi_idle();
  USBNInit();

  usbprog.longpackage = 0;
  usbprog.avrstudio = 1;   // 1 no
  usbprog.fragmentnumber = 0;  // read flash fragment
  usbprog.reset_pol = 1;  // 1= avr 0 = at89
  usbprog.datatogl=0; 

  DDRA = (1 << PA4);
  LED_off;
  RESET_high;

  USBNDeviceVendorID(0x03eb);  //atmel ids
  USBNDeviceProductID(0x2104); // atmel ids

  USBNDeviceBCDDevice(0x0200);

  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  USBNDeviceManufacture ("Elektor ");
  USBNDeviceProduct ("AVRISP mk2 (CC2)");
  USBNDeviceSerialNumber("0000A00128255");

  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,&SendCompleteAnswer);
  
  //USBNNackEvent(&NackEvent);


  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&USBFlash);

  USBNInitMC();
  sei();

  // start usb chip
  USBNStart();

  while(1);
}








