#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include "uart.h"
#include "logic.h"
#include "usbn2mc.h"


#include "ring.h"

int datatogl=0;
void LogicPingPongTX1()
{
  logic.tx=1;
}

void LogicPingPongTX2()
{
  UARTWrite("tx2 demo\r\n");
  USBNWrite(EPC1,EP_EN+3);
  //logic.update2=1;
}

// send data to the application on pc
void LogicSendScopeData()
{
  //UARTWrite("read data\r\n");
  int i;
  USBNWrite(TXC1,FLUSH);
  
  USBNWrite(TXD1,ring_get_nowait(&logic.ring));
  for(i=1;i<64;i++)
      USBNBurstWrite(ring_get_nowait(&logic.ring));
  USBNWrite(TXC1,TX_LAST+TX_EN);
}

// get and extract commands from the application on the pc
void LogicCommand(char *buf)
{
  UARTWrite("COMMMAND");
  switch(buf[0])
  { 
    case CMD_SETMODE:
      logic.mode = buf[2];
    break;

    case CMD_SETSAMPLERATE:
      UARTWrite("set mode ");
      SendHex(buf[2]);
      UARTWrite("\r\n");
      logic.samplerate=buf[2];
    break;

    case CMD_STARTSCOPE:
      UARTWrite("start scope\r\n");
      datatogl=0;
      ring_init(&logic.ring, ringbuffer, BUFFER_SIZE);

      TCCR1A = 0;

      switch(logic.samplerate)
      {
	case SAMPLERATE_5US:
	  UARTWrite("5us\n\r");
	  TCCR1B = (1 << 3) | (1 << CS11); //8tel vom takt = 500ns 
	  OCR1A = 10; //10 * 500ns = 5us
	break;
	case SAMPLERATE_10US:
	  UARTWrite("100us\n\r");
	  TCCR1B = (1 << 3) | (1 << CS11); //8tel vom takt = 500ns 
	  OCR1A = 20; //200 * 500ns = 100us
	break;
	case SAMPLERATE_50US:
	  UARTWrite("100us\n\r");
	  TCCR1B = (1 << 3) | (1 << CS11); //8tel vom takt = 500ns 
	  OCR1A = 100; //200 * 500ns = 100us
	break;
	case SAMPLERATE_100US:
	  UARTWrite("100us\n\r");
	  TCCR1B = (1 << 3) | (1 << CS11); //8tel vom takt = 500ns 
	  OCR1A = 200; //200 * 500ns = 100us
	break;
	case SAMPLERATE_1MS:
	  UARTWrite("1ms\n\r");
	  TCCR1B = (1 << 3) | (1 << CS11); //8tel vom takt = 500ns 
	  OCR1A = 2000; //200 * 500ns = 1ms
	break;
	case SAMPLERATE_10MS:
	  UARTWrite("10ms\n\r");
	  TCCR1B = (1 << 3) | (1 << CS12) ; //6250 *16M/256 = 100ms 
	  OCR1A = 625;
	break;
	case SAMPLERATE_100MS:
	  UARTWrite("100ms\n\r");
	  TCCR1B = (1 << 3) | (1 << CS12) ; //6250 *16M/256 = 100ms 
	  OCR1A = 6250;
	break;
	default:
	  UARTWrite("default\n\r");
	  TCCR1B = (1 << 3) | (1 << CS11); //8tel vom takt = 500ns
	  OCR1A = 2000; //200 * 500ns = 1ms
      }
      // enable interrupt
      TIMSK |= (1 << OCIE1A);
      logic.state=STATE_RUNNING;
    break;

    case CMD_STOPSCOPE:
      logic.ring.count=0;
      logic.state=STATE_DONOTHING;
      UARTWrite("stop scope\r\n");
      TCCR1B = (1 << WGM12) | (0 << CS12) | (0 << CS11) | (0 << CS10); // Stop (Timer/Counter) 
    break;

    case CMD_GETSCOPEMODE:
      UARTWrite("get scope mode\r\n");
      SendHex(logic.mode);
    break;

    case CMD_GETSCOPESTATE:
      UARTWrite("get scope state\r\n");
      USBNWrite(TXC1,FLUSH);
      USBNWrite(TXD1,logic.state);
      USBNWrite(TXC1,TX_LAST+TX_EN);
    break;

    case CMD_GETFIFOLOAD:
      UARTWrite("get ring load ");
      SendHex(logic.ring.count);
      UARTWrite("\r\n");
    break;

    case CMD_GETDATA:
      UARTWrite("get scope data\r\n");
      LogicSendScopeData();
      LogicPingPongTX1();
    break;

    case CMD_SETEDGETRIG:
      logic.trigger=TRIGGER_EDGE;
      logic.trigger_channel=buf[2];
      logic.trigger_value=buf[3];
    break;

    case CMD_SETPATTRIG:
      logic.trigger=TRIGGER_PATTERN;
      uint8_t n = buf[2];
      // C code which reverses the bits in a byte
      n = (((n >> 1) & 0x55) | ((n << 1) & 0xaa));
      n = (((n >> 2) & 0x33) | ((n << 2) & 0xcc));
      n = (((n >> 4) & 0x0f) | ((n << 4) & 0xf0)); 
      logic.trigger_value=n;
      n = buf[3];
      n = (((n >> 1) & 0x55) | ((n << 1) & 0xaa));
      n = (((n >> 2) & 0x33) | ((n << 2) & 0xcc));
      n = (((n >> 4) & 0x0f) | ((n << 4) & 0xf0)); 
      
      logic.trigger_ignore=n;
    break;


    case CMD_DEACTIVTRIG:
      logic.trigger=TRIGGER_OFF;
    break;


    
    default:
      UARTWrite("unknown command\r\n");
  }
}

