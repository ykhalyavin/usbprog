#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <inttypes.h>

#include "uart.h"
#include "usbn2mc.h"


//this is only my debug  tool
void Terminal(char cmd)
{  
  char h,l;
  unsigned char tmp;
  int i;
  struct list_entry *ptr;
  char *values;

  switch(cmd)
  {   
    case 'i':
      USBNStart();   
    break;
    // write to usb register
    case 'w':
      //UARTWrite("write to USB reg:");
      //USBNDEBUGPRINT("write to USB reg:");
      h = UARTGetChar();
      l = UARTGetChar();
      SendHex(AsciiToHex(h,l));
      tmp = AsciiToHex(h,l);
      UARTWrite("value:");
      h = UARTGetChar();
      l = UARTGetChar();
      SendHex(AsciiToHex(h,l));
      //USBNWrite(tmp,AsciiToHex(h,l));
      UARTWrite("result:");
      SendHex(USBNRead(tmp));
      UARTWrite("\r\n");
    break;

    // read from usb register
    case 'r':
      UARTWrite("read USB reg:");
      h = UARTGetChar();
      l = UARTGetChar();
      SendHex(AsciiToHex(h,l));
      UARTWrite("->");
      SendHex(USBNRead(AsciiToHex(h,l)));
      UARTWrite("\r\n");
    break;
    case 'h':
      UARTWrite("i usbn init procedure\r\n");
      UARTWrite("w write USBN Register <h,l>(address) <h,l> (value) e.g 05 00\r\n");
      UARTWrite("r read USBN Register <h,l> e.g. 02 ( RID)\r\n");
      UARTWrite("s show all USBN Registers\r\n");
      UARTWrite("b send test data from func to host\r\n");
      UARTWrite("d show descriptors\r\n");
    break;
    // show all registers
    case 's':
      for(i=0;i<=63;i++)
      {
        SendHex(i);
        UARTWrite("->");
        SendHex(USBNRead(i));
        UARTWrite("\r\n");
      }
    break;

    case 'd':
      USBNDebug("\r\nDescriptor List\r\n");
      ptr = DescriptorList;
      while(ptr != NULL) {
	values = (char*)ptr->data;
	SendHex(ptr->type);
	SendHex(ptr->len);
	SendHex(ptr->conf);
	SendHex(ptr->interf);
	USBNDebug("  ");
	for(i=0;i<ptr->len;i++)
	  SendHex(values[i]);
	USBNDebug("\r\n");

	ptr=ptr->next;
      }
    break;

    case 'b':
      UARTWrite("send test data from fifo1\r\n");
      int j,i;
      char stat;

      USBNWrite(TXC1,FLUSH);
      USBNWrite(TXD1,0x01);
      for(j=0;j<63;j++)
	USBNBurstWrite((unsigned char)j);

      USBNWrite(TXC1,TX_LAST+TX_EN);

      //USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
    break;
    
    case 'p':
      USBNWrite(TXC1,TX_LAST+TX_EN);
    break;
    default:
      UARTWrite("unknown command\r\n");
  }
}

