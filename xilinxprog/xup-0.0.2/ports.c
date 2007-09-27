/*******************************************************/
/* file: ports.c                                       */
/* abstract:  This file contains the routines to       */
/*            output values on the JTAG ports, to read */
/*            the TDO bit, and to read a byte of data  */
/*            from the prom                            */
/*                                                     */
/*******************************************************/
#include "ports.h"
/*#include "prgispx.h"*/

#include "stdio.h"
extern FILE *in;

#ifdef WIN95PP
#include "conio.h"

#define DATA_OFFSET    (unsigned short) 0
#define STATUS_OFFSET  (unsigned short) 1
#define CONTROL_OFFSET (unsigned short) 2

typedef union outPortUnion {
    unsigned char value;
    struct opBitsStr {
        unsigned char tdi:1;
        unsigned char tck:1;
        unsigned char tms:1;
        unsigned char zero:1;
        unsigned char one:1;
        unsigned char bit5:1;
        unsigned char bit6:1;
        unsigned char bit7:1;
    } bits;
} outPortType;

typedef union inPortUnion {
    unsigned char value;
    struct ipBitsStr {
        unsigned char bit0:1;
        unsigned char bit1:1;
        unsigned char bit2:1;
        unsigned char bit3:1;
        unsigned char tdo:1;
        unsigned char bit5:1;
        unsigned char bit6:1;
        unsigned char bit7:1;
    } bits;
} inPortType;

static inPortType in_word;
static outPortType out_word;
static unsigned short base_port = 0x378;
static int once = 0;
#else
typedef union outPortUnion {
    unsigned char value;
    struct opBitsStr {
        unsigned char bit0:1;
        unsigned char bit1:1;
        unsigned char bit2:1;
        unsigned char tck:1;
        unsigned char tms:1;
        unsigned char bit5:1;
        unsigned char tdi:1;
        unsigned char bit7:1;
    } bits;
} outPortType;

typedef union inPortUnion {
    unsigned char value;
    struct ipBitsStr {
        unsigned char bit0:1;
        unsigned char bit1:1;
        unsigned char bit2:1;
        unsigned char bit3:1;
        unsigned char bit4:1;
        unsigned char tdo:1;
        unsigned char bit6:1;
        unsigned char bit7:1;
    } bits;
} inPortType;

static inPortType in_word;
static outPortType out_word;

int usb_jtag_write(unsigned char val);
unsigned char usb_jtag_read(void);
#endif


/*BYTE *xsvf_data=0;*/


/* if in debugging mode, then just set the variables */
void setPort(short p,short val)
{
#ifdef WIN95PP
    if (once == 0) {
        out_word.bits.one = 1;
        out_word.bits.zero = 0;
        once = 1;
    }
    if (p==TMS)
        out_word.bits.tms = (unsigned char) val;
    if (p==TDI)
        out_word.bits.tdi = (unsigned char) val;
    if (p==TCK) {
        out_word.bits.tck = (unsigned char) val;
        (void) _outp( (unsigned short) (base_port + 0), out_word.value );
    }
#else
    /* Linux USB */
    if (p==TMS)
        out_word.bits.tms = (unsigned char) val;
    if (p==TDI)
        out_word.bits.tdi = (unsigned char) val;
    if (p==TCK) {
        out_word.bits.tck = (unsigned char) val;
        usb_jtag_write(out_word.value);
    }
#endif
}


/* toggle tck LH */
void pulseClock()
{
    setPort(TCK,0);  /* set the TCK port to low  */
    setPort(TCK,1);  /* set the TCK port to high */
}


/* read in a byte of data from the prom */
void readByte(unsigned char *data)
{
    /* pretend reading using a file */
    fscanf(in,"%c",data);
    /**data=*xsvf_data++;*/
}

/* read the TDO bit from port */
unsigned char readTDOBit()
{
#ifdef WIN95PP
    in_word.value = (unsigned char) _inp( (unsigned short) (base_port + STATUS_OFFSET) );
    if (in_word.bits.tdo == 0x1) {
        return( (unsigned char) 1 );
    }
    return( (unsigned char) 0 );
#else
    in_word.value = usb_jtag_read();
    if(in_word.bits.tdo == 0x1)
    {
        return 1;
    }
    return 0;
#endif
}


/* Wait at least the specified number of microsec.                           */
/* Use a timer if possible; otherwise estimate the number of instructions    */
/* necessary to be run based on the microcontroller speed.  For this example */
/* we pulse the TCK port a number of times based on the processor speed.     */
void waitTime(long microsec)
{
    static long tckCyclesPerMicrosec    = 1;
    long        tckCycles   = microsec * tckCyclesPerMicrosec;
    long        i;

    /* For systems with TCK rates >= 1 MHz;  This implementation is fine. */
    for ( i = 0; i < tckCycles; ++i )
    {
        pulseClock();
    }

#if 0
    /* For systems with TCK rates << 1 MHz;  Consider this implementation. */
    if ( microsec >= 50L )
    {
        /* Make sure TCK is low during wait for XC18V00/XCF00 */
        /* Or, a running TCK implementation as shown above is an OK alternate */
        setPort( TCK, 0 );

        /* Use Windows Sleep().  Round up to the nearest millisec */
        _sleep( ( microsec + 999L ) / 1000L );
    }
    else    /* Satisfy Virtex-II TCK cycles */
    {
        for ( i = 0; i < microsec;  ++i )
        {
            pulseClock();
        }
    }
#endif

#if 0
    /* If Virtex-II support is not required, then this implementation is fine */
    /* Make sure TCK is low during wait for XC18V00/XCF00 */
    /* Or, a running TCK implementation as shown above is an OK alternate */
    setPort( TCK, 0 );
    /* Use Windows Sleep().  Round up to the nearest millisec */
    _sleep( ( microsec + 999L ) / 1000L );
#endif
}
