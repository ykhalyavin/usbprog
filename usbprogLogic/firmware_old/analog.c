#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <inttypes.h>

#include "uart.h"
#include "usbn2mc.h"

void Terminal(char cmd);

SIGNAL(SIG_UART_RECV)
{
  Terminal(UARTGetChar());
  UARTWrite("usbn>");
}



SIGNAL(SIG_INTERRUPT0)
{
  USBNInterrupt();
}


uint16_t ReadChannel(uint8_t mux)
{
  uint8_t i;
  uint16_t result = 0;         //Initialisieren wichtig, da lokale Variablen
                               //nicht automatisch initialisiert werden und
                               //zufällige Werte haben. Sonst kann Quatsch rauskommen
  ADCSRA = (1<<ADEN) | (1<<ADPS1) | (1<<ADPS0);    // Frequenzvorteiler 
                               // setzen auf 8 (1) und ADC aktivieren (1)

  ADMUX = mux;                      // Kanal waehlen
  ADMUX |= (1<<REFS1) | (1<<REFS0); // interne Referenzspannung nutzen 

  /* nach Aktivieren des ADC wird ein "Dummy-Readout" empfohlen, man liest
     also einen Wert und verwirft diesen, um den ADC "warmlaufen zu lassen" */
  ADCSRA |= (1<<ADSC);              // eine ADC-Wandlung 
  while ( ADCSRA & (1<<ADSC) );     // auf Abschluss der Konvertierung warten 

  /* Eigentliche Messung - Mittelwert aus 4 aufeinanderfolgenden Wandlungen */
  for(i=0;i<4;i++)
  {
    ADCSRA |= (1<<ADSC);            // eine Wandlung "single conversion"
    while ( ADCSRA & (1<<ADSC) );   // auf Abschluss der Konvertierung warten
    result += ADCW;		    // Wandlungsergebnisse aufaddieren
  }
  ADCSRA &= ~(1<<ADEN);             // ADC deaktivieren (2)

  result /= 4;                     // Summe durch vier teilen = arithm. Mittelwert

  return result;
}

int togl=0;
void ADConverter(char *buf)
{
  uint16_t tmp; 
 
  tmp = ReadChannel(buf[0]);

  USBNWrite(TXC1,FLUSH);     
  USBNWrite(TXD1,(char)tmp);
  tmp = tmp >>8;
  USBNWrite(TXD1,(char)tmp);

  if(togl==0)
  {
    USBNWrite(TXC1,TX_LAST+TX_EN);
    togl=1;
  } else 
  {
    USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
    togl=0;
  }
}


int main(void)
{
  int conf, interf;
  UARTInit();
  USBNInit();   
  // setup your usbn device

  USBNDeviceVendorID(0x0400);
  USBNDeviceProductID(0x9876);
  USBNDeviceBCDDevice(0x0201);


  char lang[]={0x09,0x04};
  _USBNAddStringDescriptor(lang); // language descriptor

  
  USBNDeviceManufacture ("www.vscope.de");
  USBNDeviceProduct	("VScope Device");
  //USBNDeviceSerialNumber("2006-04-24");

  conf = USBNAddConfiguration();

  //USBNConfigurationName(conf,"StandardKonfiguration");
  USBNConfigurationPower(conf,50);

  interf = USBNAddInterface(conf,0);
  USBNAlternateSetting(conf,interf,0);

  //USBNInterfaceName(conf,interf,"usbstorage");
  

  USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&ADConverter);
  USBNAddInEndpoint(conf,interf,1,0x03,BULK,64,0,NULL);

  
  USBNInitMC();

  // start usb chip
  USBNStart();

  DDRB=0xff;
  PORTB=0x00;
  while(;;)
  {
    PORTB = 0x00;
    PORTB = 0xFF;
  }
}

