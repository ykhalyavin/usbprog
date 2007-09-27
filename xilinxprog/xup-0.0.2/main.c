/* main.c -- xilinx spartan3e starter kit usb programmer

   This file provides XSVF JTAG playback. It can be used
   to program the USB JTAG CPLD, or devices on the system
   JTAG chain (based on the ifdef below).

   XC3SProg is highly recommended for programming
   Spartan3 devices on the system JTAG chain
   (it is about twice as fast, and reads .bit files directly).

   copyright (c) 2006 inisyn research
   License: GPLv2

   Permission is expressely granted to link with the
   included versions of "lenval.[ch]", "micro.[ch]", and
   "ports.[ch]".
   
   These files originated in Xilinx App Note 58:
   http://www.xilinx.com/bvdocs/appnotes/xapp058.pdf
   ftp://ftp.xilinx.com/pub/swhelp/cpld/eisp_pc.zip

   Xilinx did not specify a license for these files;
   they are presumably public domain (?).

   These files have been slightly modified to work here;
   the modifications are released under the GPL, as
   well as the original license (whatever it may be).

   revision history
   2006-05-26 initial
*/


/* the players:

    cy7c68013a-100axc aka "USB"
    xc2s256-vq1006c   aka "CPLD"

   IO directionality:
    Xilinx iMPACT EZ-USB OEA=0xfb:
    Xilinx iMPACT EZ-USB OEB=0x00: (?)
    Xilinx iMPACT EZ-USB OEC=0xff:
    Xilinx iMPACT EZ-USB OED=0x00:
    Xilinx iMPACT EZ-USB OEE=0xd8;

   their ties:

    (reverse engineered with blunt probes;
     do not assume this info is correct!!)

    XC2S256 JTAG:
    CPLD TDI (45) <- USB PE6/T2EX (92)
    CPLD TMS (47) <- USB PE4/RXD1OUT (90)
    CPLD TCK (48) <- USB PE3/RXD0OUT (89)
    CPLD TDO (83) -> USB PE5/INT6 (91)

    USB<->CPLD Programmer Bus:
    CPLD GCLK/IO (23) <-> USB CLKOUT (100)
    CPLD IO (56) <-> USB PE2/T2OUT (88)

    CPLD IO (22) <-> USB CTL0/FLAGA (54)

    CPLD IO (32) <-> USB PB0/FD0  (34)
    CPLD IO (33) <-> USB PB1/FD1  (35)
    CPLD IO (34) <-> USB PB2/FD2  (36)
    CPLD IO (35) <-> USB PB3/FD3  (37)
    CPLD IO (36) <-> USB PB4/FD4  (44)
    CPLD IO (37) <-> USB PB5/FD5  (45)
    CPLD IO (39) <-> USB PB6/FD6  (46)
    CPLD IO (40) <-> USB PB7/FD7  (47)

    CPLD IO (14) <-> USB PC0/GPIFADR0 (57)
    CPLD IO (15) <-> USB PC1/GPIFADR1 (58)
    CPLD IO (16) <-> USB PC2/GPIFADR2 (59)
    CPLD IO (17) <-> USB PC3/GPIFADR3 (60)
    CPLD IO (18) <-> USB PC4/GPIFADR4 (61)
    CPLD IO (19) <-> USB PC5/GPIFADR5 (62)
    CPLD IO (29) <-> USB PC6/GPIFADR6 (63)
    CPLD IO (30) <-> USB PC7/GPIFADR7 (64)

    CPLD IO (41) <-> USB PD0/FD8  (80)
    CPLD IO (42) <-> USB PD1/FD9  (81)
    CPLD IO (43) <-> USB PD2/FD10 (82)
    CPLD IO (44) <- USB PD3/FD11 (83) --> USB TCK
    CPLD IO (46) <- USB PD4/FD12 (95) --> USB TMS
    CPLD IO (49) -> USB PD5/FD13 (96) --> USB TDO
    CPLD IO (50) <- USB PD6/FD14 (97) --> USB TDI
    CPLD IO (52) <-> USB PD7/FD15 (98)

    NC:
    NC (?) <-> USB CTL3 (51)
    NC (?) <-> USB CTL4 (52)
    NC (?) <-> USB CTL1/FLAGB (55)
    NC (?) <-> USB CTL2/FLAGC (56)
    NC (?) <-> USB RD (31)
    NC (?) <-> USB WR (32)
    NC (?) <-> USB INT5 (84) [10k pullup to 1.8v]

    NC (?) <-> USB PA2 (69)
    NC (?) <-> USB PA3 (70)
    NC (?) <-> USB PA4 (71)
    NC (?) <-> USB PA5 (72)
    NC (?) <-> USB PA6 (73)
    GND (?) <-> USB PA7 (74)

    NC (?) <-> USB PE0 (86)
    NC (?) <-> USB PE1 (87)
    NC (?) <-> USB PE2 (88)

    NC (?) <-> USB RDY0 (3) -- probably connected
    NC (?) <-> USB RDY1 (4) -- probably connected
    NC (?) <-> USB RDY2 (5) -- probably connected

    USB LEDs:
    LD-R <- USB PA1/INT1 (68)
    LD-G <- USB PA0/INT0 (67)

    User JTAG:
    System TCK <- R30 (15 ohm) <- CPLD IO (66)
    System TMS <- R29 (15 ohm) <- CPLD IO (67)
    Header TDI / S3E TDI <- CPLD IO (68)
    XC2C64/FX2 TDO -> CPLD GCLK/IO (27)

    CPLD VAUX = 1.8V (same 1.8V as USB)

    R24 + R26 (S3E TDI) + R27 + R28 (10k ohm) -> GND

    JTAG Chain:

    XC3s500e TDI (R77 to JTAG hdr)
    XC3s500e TMS (R76 to JTAG hdr)

    XC3s500e TDO -> PROM TDI
    PROM TDO -> XC2c64 TDI
    XC2c64 TDO -> FX2 TDO (optionally)


    --------------------------------------------------
    Device   ID Code        IR Length    Part Name
     1       01c22093           6        XC3S500E
     2       05046093           8        XCF04S
     3       06e5e093           8        XC2C64A_VQ44_1532
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <usb.h>


#undef DEBUG

/* define to program the xc2c256 (you must program it with prog.vhd
   prior to accessing the system jtag)

   undef to access system jtag through the programmed USB CPLD
*/
#define USB_CPLD_PROG

#define BENCHMARK
#ifdef BENCHMARK
#include <sys/time.h>
#include <time.h>
#endif


#define ENDPT_OUT 2
#define ENDPT_IN 6

static usb_dev_handle *dh;

static struct usb_device *find_dev(void)
{
    struct usb_bus *b;
    struct usb_device *d;

    b = usb_get_busses();

    while(1)
    {
        if(b == NULL)
        {
            break;
        }

        d = b->devices;
        while(1)
        {
            if(d == NULL)
            {
                break;
            }

            if((d->descriptor.idVendor == 0x03fd) &&
               (d->descriptor.idProduct == 0x000d))
            {
                return d;
            }

#if 0
            printf("%04x %04x\n",
                   d->descriptor.idVendor,
                   d->descriptor.idProduct);
#endif

            d = d->next;
        }

        b = b->next;
    }

    return NULL;
}



/* many thanks to http://volodya-project.sourceforge.net/fx2_programmer.php */

int download_usb_data(usb_dev_handle *dh,
                      char *data,
                      int addr,
                      int len)
{
    int ret;

    ret = usb_control_msg(dh,
                          0x40, /* vendor req out */
                          0xa0, /* firmware load */
                          addr,    /* address */
                          0,    /* index */
                          data,  /* data buffer */
                          len,   /* length */
                          1000  /* ms timeout */);
    if(ret != len)
    {
        return -1;
    }

    return 0;
}


/* NOTE: do not feed download_usb untrusted input.
         no care has been taken to audit for or
         prevent buffer overflow. it IS exploitable.

         DO NOT RUN AS ROOT
         CERTAINLY DO NOT INSTALL SUID
*/

int download_usb(usb_dev_handle *dh, char *fn)
{
    FILE *fp;
    char line[1024], data[1024];
    int ret, count;
    int addr, len, type, b;
    unsigned char csum_want, csum_got;

    /* unfortunately we cannot bitbang the USB chip's I/O pins
       directly.. we have to use its 8051..

       "The CPUCS register at E600 is the only EZ-USB
        register that can be written using Firmware DL"

       Also, we can access 0x0000-0x3fff (program/data ram)
       and 0xe000-0xe1ff (data ram) only when the cpu is
       in reset
    */
    data[0] = 0x01; /* 8051 assert reset */
    ret = usb_control_msg(dh,
                          0x40, /* vendor req out */
                          0xa0, /* firmware load */
                          0xe600,    /* address */
                          0,    /* index */
                          data,  /* data buffer */
                          1,   /* length */
                          1000  /* ms timeout */);
    if(ret != 1)
    {
        return -1;
    }

    /* download 8051 firmware */
    fp = fopen(fn, "r");
    if(fp == NULL)
    {
        return -1;
    }

    while(1)
    {
        fgets(line, sizeof(line), fp);

        if(feof(fp) || ferror(fp))
        {
            break;
        }

        if((line[0] != ':') || (strlen(line) < 9))
        {
            printf("invalid line in ihx\n");
            break;
        }

        ret = sscanf(&line[1], "%02x", &len);
        if(ret != 1)
        {
            break;
        }

        ret = sscanf(&line[3], "%04x", &addr);
        if(ret != 1)
        {
            break;
        }

        ret = sscanf(&line[7], "%02x", &type);
        if(ret != 1)
        {
            break;
        }

#ifdef DEBUG
        printf("len %u addr %04x type %u\n", len, addr, type);
#endif

        if(type == 1)
        {
            /* EOF */
            break;
        }

        if(type != 0)
        {
            printf("ihx: unknown type %u\n", type);
            /*break;*/
            continue;
        }

        csum_got = len + (addr & 0xff) + (addr >> 8) + type;

        for(count = 0; count < len; count++)
        {
            ret = sscanf(&line[9+count*2], "%02x", &b);
            if(ret != 1)
            {
                printf("ihx: data sscanf failed!\n");
                break;
            }

            data[count] = b;
            csum_got += data[count];
        }

        if(ret != 1)
        {
            break;
        }

        ret = sscanf(&line[9+len*2], "%02x", &b);
        if(ret != 1)
        {
            break;
        }

        csum_want = b;
        if(((csum_got+csum_want) & 0xff) != 0x00)
        {
            printf("ihx: checksum failure! want %02x got %02x\n",
                   csum_want, csum_got);
            break;
        }

        ret = download_usb_data(dh, data, addr, len);
        if(ret != 0)
        {
            printf("ihx: download_usb_data failed\n");
            break;
        }

#ifdef DEBUG
        puts(line);
#endif

        /* TODO: read back + verify */
    }

    fclose(fp);

    data[0] = 0x00; /* 8051 release reset */
    ret = usb_control_msg(dh,
                          0x40, /* vendor req out */
                          0xa0, /* firmware load */
                          0xe600,    /* address */
                          0,    /* index */
                          data,  /* data buffer */
                          1,   /* length */
                          1000  /* ms timeout */);
    if(ret != 1)
    {
        return -1;
    }

#if 0
    ret = usb_control_msg(dh,
                          0xc0, /* vendor req in -- 0x40 out */
                          0xa0, /* firmware load */
                          0x0000,    /* address */
                          0,    /* index */
                          buf,  /* data buffer */
                          1,   /* length */
                          1000  /* ms timeout */);
    if(ret >= 0)
    {
        for(count = 0; count < 64; count++)
        {
            printf("%02x ", (unsigned char)buf[count]);
        }
        printf("\n");

    }
    else
    {
        printf("usb_control_msg in failed\n");
    }
#endif

    return 0;
}



#define WRITE_CHUNK_LEN 512
#define WRITE_BUF_LEN 8192

#if 0
static char usb_auto_buf[WRITE_BUF_LEN];
#endif

static char usb_write_buf[2*WRITE_BUF_LEN];
static int usb_write_len;


#if 0
static int usb_jtag_write_auto(void)
{
    char now;
    int count, first, orig, out_pos;

    if(usb_write_buf[2] & (1 << 3))
    {
        /* we want a rising edge on prior data */
        /* safety of this is based on data NEVER changing
           between falling and rising edge
        */
        first = 1;
        count = 3;
    }
    else
    {
        first = 0;
        count = 2;
    }

    orig = usb_write_buf[2];

    out_pos = 2;

    while(1)
    {
        if((count & (WRITE_CHUNK_LEN-1)) == 0)
        {
            /* skip cmd bytes */
            count++;
            count++;
        }

        if((usb_write_buf[count] & (1 << 4)) !=
           (orig & (1 << 4)))
        {
            printf("tms varied at count %u\n", count);
            return -1;
        }

        if((count & 0x1) == first)
        {
            now = usb_write_buf[count];

            usb_auto_buf[out_pos] = now;
            out_pos++;
        }
        else
        {
            if(usb_write_buf[count] != (now | (1 << 3)))
            {
                printf("cannot autoclock: count %u\n", count);

    for(count = 0; count < usb_write_len; count++)
    {
        if((count&0xf) == 0)
        {
            printf("\n%02x: ", count);
        }
        printf("%02x ", usb_write_buf[count]);
    }
    printf("\n");

                return -1;
            }
        }

        count++;
        if(count == usb_write_len)
        {
            break;
        }
    }



    usb_auto_buf[0] = 0x80;

    if(first)
    {
        usb_auto_buf[0] |= 0x40;
    }
    else
    {
        printf("FIRST ZERO\n");
    }

    usb_auto_buf[1] = 0x00;

    for(count = 0; count < out_pos; count++)
    {
        if((count&0xf) == 0)
        {
            printf("\n%02x: ", count);
        }
        printf("%02x ", (unsigned char)usb_auto_buf[count]);
    }
    printf("\n");

    return 0;
}
#endif


static int usb_jtag_write_real(void)
{
#ifdef DEBUG
    int count;
#endif
    int ret, chunk, pos;

#if 0
    usb_jtag_write_auto();
#endif

#ifdef DEBUG
    for(count = 0; count < usb_write_len; count++)
    {
        if((count&0xf) == 0)
        {
            printf("\n%02x: ", count);
        }
        printf("%02x ", usb_write_buf[count]);
    }
    printf("\n");
#endif

    pos = 0;

    while(1)
    {
        if(usb_write_len > WRITE_CHUNK_LEN)
        {
            chunk = WRITE_CHUNK_LEN;
        }
        else
        {
            chunk = usb_write_len;
        }

#if 0
        printf("wl %u pos %u chunk %u\n",
               usb_write_len, pos, chunk);
#endif

        ret = usb_bulk_write(dh, ENDPT_OUT,
                             usb_write_buf + pos,
                             chunk, 
                             1000);
    
        if(ret != chunk)
        {
            printf("usb_bulk_write ret %d want %d\n",
                   ret, chunk);
        }

        pos = pos + chunk;
        usb_write_len = usb_write_len - chunk;

        if(usb_write_len == 0)
        {
            break;
        }
    }

    return 0;
}


int usb_jtag_write(unsigned char val)
{
    if(usb_write_len == WRITE_BUF_LEN)
    {
        usb_jtag_write_real();
        usb_write_len = 0;
    }

    if((usb_write_len & (WRITE_CHUNK_LEN-1)) == 0)
    {
#ifdef USB_CPLD_PROG
        usb_write_buf[usb_write_len] = 0x01;
#else
        usb_write_buf[usb_write_len] = 0x00;
#endif
        usb_write_len++;
        usb_write_len++;
    }

    usb_write_buf[usb_write_len] = val;

#ifdef DEBUG
    printf("usb_jtag_write %02x %02x\n", usb_write_len, val);
#endif

    usb_write_len++;

    return 0;
}


unsigned char usb_jtag_read(void)
{
    char data[3];
    int ret;

    /* reads force us to flush any queued writes */
    usb_jtag_write_real();
    usb_write_len = 0;

    /* clear out the stale read (forces fw to resample the IO) */
    ret = usb_bulk_read(dh, ENDPT_IN, data, 3, 1000);
    if(ret != 3)
    {
        printf("usb_bulk_read ret %d\n", ret);
    }


    /* now read the latest value from IO */
    ret = usb_bulk_read(dh, ENDPT_IN, data, 3, 1000);
    if(ret != 3)
    {
        printf("usb_bulk_read ret %d\n", ret);
        data[1] = 0x00;
        data[2] = 0x00;
    }

#ifdef DEBUG
    printf("usb_jtag_read %02x %02x\n", data[1], data[2]);
#endif

#ifdef USB_CPLD_PROG
    return data[1];
#else
    return data[2];
#endif
}



int led_usb(usb_dev_handle *dh)
{
    struct timeval start, done;
    char data[512];
    int ret, count;

    data[0] = 0x2;
    data[1] = 0x0;
    data[2] = 0x0;

    gettimeofday(&start, NULL);

    /* EP1 3 bytes  10k times: 1.2s
       EP1 64 bytes 10k times: 1.2s

       EP2 64 bytes 10k times : 1.2s
       EP2 512 bytes 10k times: 1.4s

       EP2 510 bytes, 10k times, output 254 bytes to IOA: 13s
       EP2 510 bytes, 10k times, output 254 bytes to IOA*2: 14-18s

       EP2 512 bytes, 10k times, output 512 bytes to IOA*2: 20s
    */
    for(count = 0; count < 10001; count++)
    {
        ret = usb_bulk_write(dh, ENDPT_OUT, data, 512, 1000);
        if(ret != 512)
        {
            printf("usb_bulk_write ret %d\n", ret);
            break;
        }

#if 0
        printf("usb_bulk_write %02x %02x %02x\n", data[0], data[1], data[2]);
#endif

#if 0
        ret = usb_bulk_read(dh, 1, data, 2, 1000);
        if(ret != 2)
        {
            printf("usb_bulk_read ret %d\n", ret);
        }

        printf("usb_bulk_read %02x %02x\n", data[0], data[1]);
#endif

        /*data[0] ^= 0x3;*/
    }

    gettimeofday(&done, NULL);

    printf("%u %u - %u %u\n",
           (unsigned int)start.tv_sec,
           (unsigned int)start.tv_usec,
           (unsigned int)done.tv_sec,
           (unsigned int)done.tv_usec);

    return 0;
}


int xsvf_main(int argc, char **argv);

int main(int argc, char *argv[])
{

    struct usb_device *d;
    int ret;

    usb_init();
    usb_find_busses();
    usb_find_devices();

    d = find_dev();
    if(d)
    {
        dh = usb_open(d);
        if(dh)
        {
            ret = download_usb(dh, "fw.ihx");

            ret = usb_claim_interface(dh, 0);
            if(ret < 0)
            {
                return ret;
            }

            usb_set_altinterface(dh, 1);
            
            usb_write_len = 0;

#if 0
            led_usb(dh);
#else
            xsvf_main(argc, argv);
#endif
            usb_release_interface(dh, 0);

            usb_close(dh);
        }
        else
        {
            printf("failed to open usb device!\n");
        }
    }
    else
    {
        printf("no xilinx usb device found!\n");
    }

    return 0;
}

