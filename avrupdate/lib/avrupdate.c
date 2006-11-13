#include <stdio.h>
#include <usb.h>
#include "avrupdate.h"

#define STARTAPP  0x01
#define WRITEPAGE 0x02

void avrupdate_flash_bin(struct usb_dev_handle* usb_handle,char *file)
{
  	char buf[64];
  	char cmd[64];

  	FILE *fd;


  	// get page size
  	// get flash size
 
  	// send every page in a usb package
  	// 1. header ( action, page number)
  	// 2. data   

  	// if you have got a controller with blocksize greater than 64
  	// work with the easyavr like you have an 64 page size flash
  	// the controller manages the rest

  	//printf("send %s to easyavr\r\n",file);

  	int page=0;
  	int offset=0;

 	// open bin file
  	fd = fopen(file, "r");
  	if(!fd) {
    	fprintf(stderr, "Unable to open file %s, ignoring.\n", file);
  	}

  	while(!feof(fd))
  	{
    	buf[offset]=fgetc(fd);

    	offset++;
    	if(offset == 64)
    	{
      		//printf("send package\n");
      		// command message
      		cmd[0]=WRITEPAGE;
      		cmd[1]=(char)page; // page number
      		usb_bulk_write(usb_handle,2,cmd,64,100);

      		// data message 
      		usb_bulk_write(usb_handle,2,buf,64,100);
      		offset = 0;
      		page++;
    	}
  	}
  	if(offset > 0)
  	{
    	//printf("rest\n");
    	// command message
    	cmd[0]=WRITEPAGE;
    	cmd[1]=(char)page; // page number
    	usb_bulk_write(usb_handle,2,cmd,64,100);

    	// data message 
    	usb_bulk_write(usb_handle,2,buf,64,100);
  	}	 
}


void avrupdate_startapp(struct usb_dev_handle* usb_handle)
{
  	char buf[64];
  	char *ptr = buf;

 
   	buf[0]=STARTAPP;
   	usb_bulk_write(usb_handle,2,ptr,64,100);
   	//printf("TX stat=%d\n",stat);
}


struct usb_dev_handle* avrupdate_open(short vendorid, short productid)
{
	struct usb_bus *busses;

  	usb_set_debug(2);
  	usb_init();
  	usb_find_busses();
  	usb_find_devices();

  	busses = usb_get_busses();

 	struct usb_dev_handle* usb_handle;
  	struct usb_bus *bus;


  	unsigned char send_data=0xff;

  	for (bus = busses; bus; bus = bus->next)
  	{
    	struct usb_device *dev;

    	for (dev = bus->devices; dev; dev = dev->next){
      		if (dev->descriptor.idVendor == vendorid){
        		int i,stat;
        		printf("vendor: %i\n",dev->descriptor.idVendor);
        		usb_handle = usb_open(dev);
        		stat = usb_set_configuration (usb_handle,1);
				return usb_handle;
      		}
    	}	
  	}
}


void avrupdate_close(struct usb_dev_handle* usb_handle)
{
  	usb_close(usb_handle);
}
