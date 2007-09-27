/* fw.c -- cy8c68013a-100axc jtag firmware

   copyright (c) 2006 inisyn research
   License: GPLv2

   revision history
   2006-05-26 initial
   2006-08-19 xilinx friendly IO mapping
*/


#define EXTERN
#define _AT_(a) at a
typedef unsigned char BYTE;

EXTERN xdata _AT_(0xE600) volatile BYTE CPUCS;
EXTERN xdata _AT_(0xE601) volatile BYTE IFCONFIG;
EXTERN xdata _AT_(0xE604) volatile BYTE FIFORESET;

EXTERN xdata _AT_(0xE68D) volatile BYTE EP1OUTBC;
EXTERN xdata _AT_(0xE68F) volatile BYTE EP1INBC;
EXTERN xdata _AT_(0xE6A1) volatile BYTE EP1OUTCS;
EXTERN xdata _AT_(0xE6A2) volatile BYTE EP1INCS;
EXTERN xdata _AT_(0xE780) volatile BYTE EP1OUTBUF[64];
EXTERN xdata _AT_(0xE7C0) volatile BYTE EP1INBUF[64];


EXTERN xdata _AT_(0xE6A3) volatile BYTE EP2CS;
EXTERN xdata _AT_(0xE6A5) volatile BYTE EP6CS;
EXTERN xdata _AT_(0xE6A6) volatile BYTE EP8CS;


EXTERN xdata _AT_(0xE690) volatile BYTE EP2BCH;
EXTERN xdata _AT_(0xE691) volatile BYTE EP2BCL;

EXTERN xdata _AT_(0xF000) volatile BYTE EP2FIFOBUF[1024];

EXTERN xdata _AT_(0xE698) volatile BYTE EP6BCH;
EXTERN xdata _AT_(0xE699) volatile BYTE EP6BCL;

EXTERN xdata _AT_(0xF800) volatile BYTE EP6FIFOBUF[1024];

EXTERN xdata _AT_(0xE69C) volatile BYTE EP8BCH;
EXTERN xdata _AT_(0xE69D) volatile BYTE EP8BCL;

EXTERN xdata _AT_(0xFC00) volatile BYTE EP8FIFOBUF[1024];


sfr at 0x80 IOA;
sfr at 0xB2 OEA;

sfr at 0xA0 IOC;
sfr at 0xB4 OEC;

sfr at 0xB0 IOD;
sfr at 0xB5 OED;

sfr at 0xB1 IOE;
sfr at 0xB6 OEE;


#define bmBIT1	2
#define bmBIT2	4
#define bmBIT3	8

#define NOP   _asm \
		nop; \
		_endasm;


void init_usb(void)
{
    /* 48MHz */
    CPUCS = 0x10;

    /* 48 MHz */
    IFCONFIG = 0xC0;
    NOP;

    FIFORESET = 0x80;
    NOP;
    FIFORESET = 2;
    NOP;
    FIFORESET = 4;
    NOP;
    FIFORESET = 6;
    NOP;
    FIFORESET = 8;
    NOP;
    FIFORESET = 0;
    NOP;
}


void main(void)
{
    int i, j, l;
    char b;

    init_usb();

    /* LED */
    OEA = 0x3;
    NOP;
    IOA = 0x1;
    NOP;

    /* System JTAG: Output */
    OEC = ((1 << 3) | (1 << 4) | (1 << 6));
    NOP;
    IOC = 0x00;
    NOP;

    /* System JTAG: Input */
    OED = 0;
    NOP;

    /* CPLD JTAG */
    OEE = ((1 << 3) | (1 << 4) | (1 << 6));
    NOP;
    IOE = 0x00;
    NOP;

    i = 0;
    j = 0;

    /* arm EP2 (host->fw) */
    EP2BCL = 0xff;
    NOP;
    EP2BCL = 0xff;
    NOP;

    while(1)
    {
        if(EP6CS & bmBIT2)
        {
            /* ready to send */
            EP6FIFOBUF[0] = IOA;
            NOP;
            EP6FIFOBUF[1] = IOE;
            NOP;
            EP6FIFOBUF[2] = IOD;
            NOP;
            EP6BCH = 0;
            NOP;
            EP6BCL = 3;
            NOP;
        }

        if((EP2CS & bmBIT2) == 0)
        {
            l = (EP2BCH << 8) | EP2BCL;

            if(EP2FIFOBUF[0] == 0x00)
            {
                /* explicit clocking */
                for(i = 2; i < l; i++)
                {
                    b = EP2FIFOBUF[i];
                    IOC = b;
                }
            }
            else if(EP2FIFOBUF[0] == 0x01)
            {
                /* explicit clocking */
                for(i = 2; i < l; i++)
                {
                    b = EP2FIFOBUF[i];
                    IOE = b;
                }
            }
            else if(EP2FIFOBUF[0] == 0x80)
            {
                /* auto clocking */
                for(i = 2; i < l; i++)
                {
                    b = EP2FIFOBUF[i];
                    IOC = b;
                    IOC = b | (1 << 3);
                }
            }
            else if(EP2FIFOBUF[0] == 0x81)
            {
                /* auto clocking */
                for(i = 2; i < l; i++)
                {
                    b = EP2FIFOBUF[i];
                    IOE = b;
                    IOE = b | (1 << 3);
                }
            }

            /* ack it */
            EP2BCH = 0xff;
            NOP;
            EP2BCL = 0xff;
            NOP;
        }
    }
}

