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

#define F_CPU 16000000
#include <util/delay.h>

#include "wait.h"

#include "../usbprog_base/firmwarelib/avrupdate.h"
//#include "uart.h"
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
static uint16_t temp_OCR1A;


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
} pgmmode;


unsigned short sck_lookup[]PROGMEM =
{
	2,4,8,16,32,64,128,166,178,190,202,214,226,238,250,262,
	274,286,310,322,346,370,382,406,418,442,466,490,514,538,562,586,
	622,646,682,718,754,790,826,862,910,946,994,1042,1102,1150,1210,1270,
	1330,1390,1463,1534,1606,1690,1762,1665,1942,1808,2134,2242,2350,2470,2590,2722,
	2854,2986,3142,3285,3453,3622,3801,3981,4185,4390,4606,4834,5062,5577,5852,6128,
	6441,6754,7089,7435,7797,8180,8574,8994,9440,9907,10396,10899,11445,12003,12589,13201,
	13853,14532,15253,16000,16789,17602,18476,19370,20330,21333,22378,23460,24615,25848,27119,29851,
    31311,32854,34409,36117,37915,39801,41667,43716,45845,48193,50473,52980,55556,58394,61303,64257
};



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
  if(usbprog.sck_duration > 0x82)  // prevent error when EEPROM is erased
    usbprog.sck_duration = 0x03;  //  1MHz as default

  PORTB   &=  ~((1 << SCK)|(1<<MISO)|(1<<MOSI)/*|(1<<RESET_PIN)*/);  // SCK have to be low in IDLE
  DDR_SPI &=~(1 << MISO);
  DDR_SPI = (1 << MOSI)|(1 << SCK)|(1 << RESET_PIN);

  SPCR = 0;
  SPSR = 0;
  TCCR1B = 0;

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
        // TC1 initialisation
        temp_OCR1A = pgm_read_word(&(sck_lookup[usbprog.sck_duration]));
		TCNT1 = 0;
        TCNT1 = 0;
        TCCR1A = 0;
        TCCR1B = ((1<<WGM12)|(1<<CS10));  //CTCmode, f=16MHz
        TIMSK &= ~((1 <<TICIE1)|(1<<OCIE1A)|(1<<OCIE1B)|(1<<TOIE1));	// no Interrupts
        OCR1A = temp_OCR1A;
        OCR1B = (temp_OCR1A>>2);
		break;
    }
}


void spi_idle(void)
{
  DDR_SPI = 0x00;
  PORTB = 0xFF;	// only temp for jar
}

void spi_active(void)
{
  PORTB   &=  ~((1<<MISO)|(1 << MOSI)|(1 << SCK)|(1 << RESET_PIN));  // switch off unused Pullup resistors
  DDR_SPI &=~(1 << MISO);
  DDR_SPI = (1 << MOSI)|(1 << SCK)|(1 << RESET_PIN);
//  DDR_SPI = 0xa1;
}


void wait_OCF1A(void)
{
    if(TIFR & (1<<OCF1A))
      TIFR |= (1<<OCF1A);
    while(!(TIFR & (1<<OCF1A)))
	  __asm__ __volatile__ ("nop");

  OCR1B = (temp_OCR1A>>2);
}

void wait_OCF1B(void)
{
    if(TIFR & (1<<OCF1B))
      TIFR |= (1<<OCF1B);
    while(!(TIFR & (1<<OCF1B)))
	  __asm__ __volatile__ ("nop");

  OCR1B += (temp_OCR1A>>2);
}


void spi_out(char data)
{

  if((usbprog.sck_duration >= 0) && (usbprog.sck_duration <= 6))
  {
    SPDR = data;
    while ( !(SPSR & (1 << SPIF)) ) ;
  }

  else    // software SPI with TC1
  {
    TCNT1 = 0;  // set Timer register to zero
    OCR1B = (temp_OCR1A>>2);
    for (int i = 0; i < 8; i++)
    {
      if(data & 0x80)
        PORTB   |=  (1 << MOSI);
      else
        PORTB   &=  ~(1 << MOSI);
      data = data << 1;
      wait_OCF1B();
      PORTB   |=  (1 << SCK);
      wait_OCF1B();
      wait_OCF1B();
      PORTB   &=  ~(1 << SCK);
      wait_OCF1A();
    }
  }
}


char spi_in(void)
{
  if((usbprog.sck_duration >= 0) && (usbprog.sck_duration <= 6))
  {
    SPDR = 0;
//    int timeout = 1000;
    while(!(SPSR & (1 << SPIF))) ;
//	{
//      timeout--;
//      if(timeout == 0)
//	    break;
//    }
    return SPDR;
  }

  else  // software spi with TC1
  {
    char b = 0;
    PORTB   &=  ~(1 << MOSI);
    TCNT1 = 0;  // set Timer register to zero
    OCR1B = (temp_OCR1A>>2);
    for(int i = 0; i < 8; i++)
    {
      b = b << 1;
      wait_OCF1B();
      PORTB   |=  (1 << SCK);
      wait_OCF1B();
      if( PINB & (1<<MISO))
      {
         b |= 1;
      }
      wait_OCF1B();
      PORTB   &=  ~(1 << SCK);
      wait_OCF1A();
    }
    return b;
  }
}


void spi_write_program_memory(char data)
{


}

void spi_write_program_memory_page(unsigned short wordaddr,char wordl,char wordh)
{
  char addrh,addrl;

  addrh = (char)(wordaddr >> 8);
  addrl = (char)(wordaddr);

  // write low
  spi_out(0x40);
  spi_out(addrh);
  spi_out(addrl);
  spi_out(wordl);

  // write high
  spi_out(0x48);
  spi_out(addrh);
  spi_out(addrl);
  spi_out(wordh);
}

/* programm finite state machine
*
*/
void flash_program_fsm(char * buf)
{
  int bufindex = 0, bytes = 0;
  int start_address;

  start_address = pgmmode.address;  // store the page address (which in fact are the first 5 bits) for page write command
  // because we will increment pgmode.address during transfer

  // if page mode
  if(pgmmode.mode && 0x01) {
    if(usbprog.cmdpackage == 1) {
      // first packet contains header and data
      bufindex = 10;  // skip the packet header
      if(pgmmode.numbytes > 54) {
	usbprog.longpackage = 1; // we do expect more data
	bytes = 64;
	pgmmode.numbytes = pgmmode.numbytes - 64 + bufindex; // max packet is 64 bytes w/ header and data (USB FIFO-size)
	// so we have only 54 bytes space for data to program
      }
      else {
	usbprog.longpackage = 0; // we do not expect more data
	bytes = pgmmode.numbytes + bufindex;
	pgmmode.numbytes = 0;
      }

      for(; bufindex < bytes; bufindex = bufindex + 2){
	spi_write_program_memory_page(pgmmode.address, buf[bufindex], buf[bufindex + 1]);
	pgmmode.address++;
      }
      usbprog.cmdpackage = 0;
    }
    else {
      if(pgmmode.numbytes > 64) {
	usbprog.longpackage = 1; // we do expect more data
	bytes = 64;
	pgmmode.numbytes = pgmmode.numbytes - 64;
      }
      else {
	usbprog.longpackage = 0; // we do not expect more data
	bytes = pgmmode.numbytes;
	pgmmode.numbytes = 0;
      }

      for(bufindex = 0; bufindex < bytes; bufindex = bufindex + 2){
	spi_write_program_memory_page(pgmmode.address, buf[bufindex], buf[bufindex+1]);
	pgmmode.address++;
      }
    }

    // falls numbytes == 0
    if(pgmmode.numbytes == 0) {
      // write page to flash
      spi_out(pgmmode.cmd2);
      spi_out((char)(start_address >> 8));  //high-byte of page address
      spi_out((char)(start_address));        //low-byte  of page address
      spi_out(0x00);
      wait_ms(20);

      // acknowlegde to host
      answer[0] = CMD_PROGRAM_FLASH_ISP;
      answer[1] = STATUS_CMD_OK;
      CommandAnswer(2);
      usbprog.longpackage = 0; // we do not expect data anymore (for this page)
    }
  }
  // else word mode
  else {
    // ToDo
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

void CommandAnswer(int length)
{
  int i;

  USBNWrite(TXC1, FLUSH);
  for(i = 0; i < length; i++)
    USBNWrite(TXD1, answer[i]);

  /* control togl bit */
  USBToglAndSend();
}

/* central command parser */
void USBFlash(char *buf)
{
  char result = 0;
  int numbytes;

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
		if(buf[2] != usbprog.sck_duration)
		{
		  if(buf[2]>0x82)  // supports only frequencys down to 249Hz
		    buf[2] = 0x82;
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

        case PARAM_SW_MAJOR:  // avrisp mkII special
          answer[2] = 1;
        break;

        case PARAM_SW_MINOR:  // abrisp mkII special
          answer[2] = 6;
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
      pgmmode.address = 0;
      spi_active();
      LED_on;
      //spi_out(0xFF);

      //cbi  portb,SCK  ; clear SCK
      PORTB &= ~(1<<SCK);
      //PORTB |=(1<<SCK);
      //PORTB &= ~(1<<SCK);
      //PORTB |=(1<<SCK);
      //PORTB &= ~(1<<SCK);
      // set_reset    ;  set RESET = 1
      if(usbprog.reset_pol==1)
	  {
     	PORTB |= (1<<RESET_PIN);  // give reset a positive pulse
     	wait_ms(10);
     	PORTB &= ~(1<<RESET_PIN);
     	wait_ms(10);
      }
	  else
	  {
    	PORTB &= ~(1<<RESET_PIN);
    	wait_ms(10);
    	PORTB |= (1<<RESET_PIN);  // give reset a positive pulse
    	wait_ms(10);
      }

      answer[0] = CMD_ENTER_PROGMODE_ISP;
      answer[1] = STATUS_CMD_FAILED;

      cli();
      spi_out(0xac);
      spi_out(0x53);
      sei();

      //RESET_high;
      //RESET_low;

      //int syncloops = buf[4];
      int syncloops = 5;
      for (;syncloops > 0; syncloops--) {
	wait_ms(10);
        result = spi_in();
        //SendHex(result);
        if (result == buf[6]) {  //0x53 for avr
		spi_out(0x00);
          answer[1] = STATUS_CMD_OK;
          CommandAnswer(2);
          return;
          break;
        }
        spi_out(0x00);
        wait_ms(20);
        spi_out(0xac);
        spi_out(0x53);
      };

      spi_out(0x00);

      answer[1] = STATUS_CMD_FAILED;
      CommandAnswer(2);
      return;
    break;

    case CMD_LEAVE_PROGMODE_ISP:
      LED_off;
      RESET_high;
      spi_idle();
      answer[0] = CMD_LEAVE_PROGMODE_ISP;
      answer[1] = STATUS_CMD_OK;
      CommandAnswer(2);

      // wenn adapter vom avrdude aus angesteuert wird
      if(usbprog.avrstudio==0)
        usbprog.datatogl=0;  // to be sure that togl is on next session clear
      return;
    break;

    case CMD_CHIP_ERASE_ISP:
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

      usbprog.cmdpackage = 1;
      flash_program_fsm(buf);
      usbprog.cmdpackage = 0;
    break;

    case CMD_READ_FLASH_ISP:

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
      else CommandAnswer(pgmmode.numbytes + 2);

      return;
    break;

    case CMD_READ_LOCK_ISP:
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
      }
      else CommandAnswer(pgmmode.numbytes + 2);


    break;

    case CMD_PROGRAM_FUSE_ISP:
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
      usbprog.avrstudio=1;  // only avrdude use this command
      spi_out(buf[4]);
      spi_out(buf[5]);
      spi_out(buf[6]);

      // instruction
      switch(buf[4]) {
        // read low flash byte
        case 0x20:
        // read high flash byte
        case 0x28:
        // read signature
        case 0x30:
        // read lfuse
        case 0x50:
            // read lock
        case 0x38:
        // read hfuse and lock
        case 0x58:
        // read eeprom memory
        case 0xa0:
          result = spi_in();
        break;

        //write fuse and lock bit
        case 0xac:
        //write eeprom
        case 0xc0:
          spi_out(buf[7]);
          result = 0x00;
        break;
      }

      answer[2] = 0x00;
      answer[3] = 0x00;
      answer[4] = 0x00;
      answer[5] = result; // why result in position 5 ???

      answer[0] = CMD_SPI_MULTI;
      answer[1] = STATUS_CMD_OK;

      answer[6] = STATUS_CMD_OK;
      CommandAnswer(3 + buf[2]);
      return;
    break;
    }
  }
}


int main(void)
{

 int conf, interf;

  //UARTInit();

  spi_init();
  spi_idle();
  USBNInit();

  usbprog.longpackage = 0;
  usbprog.avrstudio = 1;   // 1 no
  usbprog.fragmentnumber = 0;  // read flash fragment
  usbprog.reset_pol = 1;  // 1= avr 0 = at89

  DDRA = (1 << PA4);
  LED_off;
  RESET_high;

  USBNDeviceVendorID(0x03eb);  //atmel ids
  USBNDeviceProductID(0x2104); // atmel ids

  USBNDeviceBCDDevice(0x0200);

  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  USBNDeviceManufacture("B.Sauter");
  USBNDeviceProduct("AVRISP mk2 Clone");
  USBNDeviceSerialNumber("0000A00128255");

  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,&SendCompleteAnswer);
  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&USBFlash);

  USBNInitMC();
  sei();

  // start usb chip
  USBNStart();

  while(1);
}








