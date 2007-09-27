/* JTAG GNU/Linux Xilinx USB IO 

Copyright (C) 2006 inisyn research

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */



#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include <sys/time.h>
#include <unistd.h>

#include <usb.h>

#include "ioxusb.h"

using namespace std;



#define ENDPT_OUT 2
#define ENDPT_IN 6


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



int IOXUSB::usb_jtag_write_real(void)
{
    int ret, count;

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

    ret = usb_bulk_write(dh, ENDPT_OUT,
                         usb_write_buf,
                         usb_write_len,
                         1000);

    if(ret != usb_write_len)
    {
        printf("usb_bulk_write ret %d want %d\n",
               ret, usb_write_len);
    }

    usb_write_len = 0;

    return 0;
}


int IOXUSB::usb_jtag_write(unsigned char val)
{
    int ret;

    if(usb_write_len == USB_WRITE_BUF_LEN)
    {
        usb_jtag_write_real();
    }

    if((usb_write_len & (USB_WRITE_BUF_LEN-1)) == 0)
    {
        /* auto clocking */
        usb_write_buf[usb_write_len] = 0x80;
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



unsigned char IOXUSB::usb_jtag_read(void)
{
    char data[3];
    int ret;

    /* force writes to cable to ensure read coherence */
    usb_jtag_write_real();

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
        data[2] = 0x00;
    }

#ifdef DEBUG
    printf("usb_jtag_read %02x\n", data[1]);
#endif

    return data[2];
}



IOXUSB::IOXUSB(const char *device_name) : IOBase()
{
    struct usb_device *d;
    int ret;

    usb_write_len = 0;

    error=true;

    usb_init();
    usb_find_busses();
    usb_find_devices();

    d = find_dev();
    if(d)
    {
        dh = usb_open(d);
        if(dh)
        {
            ret = download_usb(dh, "/usr/share/xup/fw.ihx");
            if(ret == 0)
            {
                ret = usb_claim_interface(dh, 0);
                if(ret >= 0)
                {
                    usb_set_altinterface(dh, 1);

                    error = false;
                }
                else
                {
                    usb_close(dh);
                    dh = NULL;
                }
            }
            else
            {
                printf("failed to download xusb 8051 firmware!\n");
                usb_close(dh);
                dh = NULL;
            }
        }
        else
        {
            printf("failed to open usb device!\n");
        }
    }
    else
    {
        printf("no xilinx usb device found!\n");
        dh = NULL;
    }
}


bool IOXUSB::txrx(bool tms, bool tdi)
{
    unsigned char c;

    tx(tms, tdi);

    c = usb_jtag_read();

    return ((c & (1 << 5)) != 0);
}


void IOXUSB::tx(bool tms, bool tdi)
{
    unsigned char c;

    c = 0;

    if(tdi)
    {
        c |= (1 << 6);
    }

    if(tms)
    {
        c |= (1 << 4);
    }

    usb_jtag_write(c);

    /* auto clocked by 8051 firmware */
#if 0
    c |= (1 << 3);

    usb_jtag_write(c);
#endif
}


IOXUSB::~IOXUSB()
{
    if(dh)
    {
        usb_jtag_write_real();

        usb_release_interface(dh, 0);

        usb_close(dh);
    }
}

