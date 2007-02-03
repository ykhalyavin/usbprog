/* 
 * Benedikt Sauter <sauter@ixbat.de> 2006-04-10
 *
 */
#include "atmega.h"


void atmega_reset(usb_dev_handle * usb_handle,char type)
{
  char send[1];
  send[0]=RESET;
  printf("start reset\n");
  usb_bulk_write(usb_handle,2,send,1,1);
  printf("ready\n");
}

void atmega_erase(usb_dev_handle * usb_handle,char type)
{
  char send[1];
  send[0]=ERASE;
  printf("start erasure\n");
  usb_bulk_write(usb_handle,2,send,1,1);
  printf("ready\n");
}

void atmega_upload(usb_dev_handle * usb_handle,char type,char *file)
{
  char send[MAXPACKETSIZE]; 
  FILE *fd;
  int addr=0;
  int index=4;
  int len;

  printf("start upload\n");
  // cmd startaddrhigh staraddrlow len 
  // split in 60 byte packages
  send[0]=UPLOAD;

  fd = fopen(file, "r");
  if(!fd) {
    fprintf(stderr, "Unable to open file %s, ignoring.\n", file);
  }
 
  while(!feof(fd))
  { 
    len = 0;
    while(!feof(fd))
    {
      send[index++]=fgetc(fd);
      if(index==(MAXPACKETSIZE-1))
	break;
    }
    len = index-4;

    send[1]=(char)(addr>>8);
    send[2]=(char)addr;
    send[3]=(char)len;
    usb_bulk_write(usb_handle,2,send,MAXPACKETSIZE,1000);
    addr = addr+len;

    index=4;
  }

  fclose(fd);
 
  printf("ready\n");

}



