#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "uart.h"
#include "usbn2mc.h"
#include "../usbprog_base/firmwarelib/avrupdate.h"


#define LED_PIN     PA4
#define LED_PORT    PORTA

#define LED_on     (LED_PORT   |=  (1 << LED_PIN))   // red led
#define LED_off    (LED_PORT   &= ~(1 << LED_PIN))


#define AT45_CMD_STATUS         0x57
#define AT45_CMD_AREAD          0x68
#define AT45_CMD_APROG          0x82

#define AT45_STATE_READY        0x80

#define CS_AT45         PB4

#define DDR_SPI         DDRB
#define DD_MOSI         PB5
#define DD_SCK          PB7


volatile char answer[320];

volatile struct usbprog_t
{
  int long_index;
  int long_bytes;
  int long_running;
  int datatogl;
} usbprog;

SIGNAL(SIG_INTERRUPT0)
{
  USBNInterrupt();
}

void USBNDecodeVendorRequest(DeviceRequest *req)
{
  if(req->bRequest == STARTAVRUPDATE)
      avrupdate_start();
}

void CommandAnswer(length)
{
  int i;

  // if first packet of a lang message
  if(length>64 && usbprog.long_running==0){
    usbprog.long_index=0;
    usbprog.long_bytes=length;
    usbprog.long_running=1;
    length=64;
  }

  USBNWrite(TXC1, FLUSH);

  for(i = 0; i < length; i++)
    USBNWrite(TXD1, answer[usbprog.long_index+i]);

  /* control togl bit */
  if(usbprog.datatogl == 1) {
    USBNWrite(TXC1, TX_LAST+TX_EN+TX_TOGL);
    usbprog.datatogl = 0;
  } else {
    USBNWrite(TXC1, TX_LAST+TX_EN);
    usbprog.datatogl = 1;
  }
}


void CommandAnswerRest()
{

  if(usbprog.long_running==1){
    PORTA ^= (1<<PA7);
    if(usbprog.long_index < usbprog.long_bytes){
      int dif = usbprog.long_bytes-usbprog.long_index; 
      usbprog.long_index=usbprog.long_index+64;

      if(dif > 64){
	CommandAnswer(64);
      }
      else {
	// last packet
	CommandAnswer(dif);
	usbprog.long_running=0;
      }
    }
  }
}


void Commands(char * buf)
{
  //PORTA ^= (1<<PA7);

  at45_write( (uint16_t)(buf[0]|(buf[1]<<8)), &buf[2]);
  at45_read( (uint16_t)(buf[0]|(buf[1]<<8)), 1);

  //if(buf[0]==0x77 && buf[1]==0x88)
    CommandAnswer(320);
}


int main(void)
{
  int conf, interf;
  
  //UARTInit();
  
  USBNInit();   
  usbprog.long_running=0;

  DDRA = (1 << PA4); // status led
  DDRA = (1 << PA7); // switch pin


  // setup your usbn device

  USBNDeviceVendorID(0x1786);
  USBNDeviceProductID(0x0c62);
  USBNDeviceBCDDevice(0x0007);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor
  
  USBNDeviceManufacture ("EmbeddedProjects");
  USBNDeviceProduct	("usbprogSkeleton ");

  conf = USBNAddConfiguration();

  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,&CommandAnswerRest);
  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&Commands);

  
  USBNInitMC();
  sei();
  USBNStart();

  //LED_on;
  int i;
  for(i=0;i<320;i++)
    answer[i]=i+2;
  
  PORTA &= ~(1<<PA7);
  //CommandAnswer(320);

  while(1){
    //PORTA |= (1<<PA7);
    //PORTA &= ~(1<<PA7);
  }
}


void spi_init(void) {
        PORTB = (1<<CS_AT45);
        DDRB = (1<<CS_AT45);

        DDR_SPI |= (1<<DD_MOSI) | (1<<DD_SCK);
        SPCR = (1<<SPE) | (1<<MSTR) | (1<<CPOL) | (1<<CPHA);
        SPSR = (1<<SPI2X);
}

uint8_t spi_txrx(uint8_t data) {
        SPDR = data;
        while(!(SPSR & (1<<SPIF))) {}
        return SPDR;
}

void spi_select(uint8_t val) {
        uint8_t sav;

        //cli();

        sav = PORTB | (1<<CS_AT45);
        PORTB = sav ^ (1<<val);

        //sei if necessary
}



/* TODO: timeout */
void at45_busywait(void) {
        uint8_t status;

        do {
                spi_select(CS_AT45);

                spi_txrx(AT45_CMD_STATUS);
                status = spi_txrx(0);

                spi_select(0);
        } while (!(status & AT45_STATE_READY));

}

void at45_read(uint16_t addr, uint8_t pagemode) {

        at45_busywait();

        spi_select(CS_AT45);

        spi_txrx(AT45_CMD_AREAD);

        spi_txrx(addr>>7);
        spi_txrx(addr<<1);
        spi_txrx(0);

        spi_txrx(0);
        spi_txrx(0);
        spi_txrx(0);
        spi_txrx(0);

        if(!pagemode)
                return;

        for(addr=0; addr < 264; addr++)
                answer[addr] = spi_txrx(0);

        spi_select(0);
}



void at45_write(uint16_t addr, uint8_t *data) {

        at45_busywait();

        spi_select(CS_AT45);

        spi_txrx(AT45_CMD_APROG);

        spi_txrx(addr>>7);
        spi_txrx(addr<<1);
        spi_txrx(0);

        for(addr=0; addr < 264; addr++)
                spi_txrx(data[addr]);

        spi_select(0);
}







