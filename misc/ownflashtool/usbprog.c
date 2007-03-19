/* 
 * Benedikt Sauter <sauter@ixbat.de> 2006-04-10
 *
 *  Using:
 *  usbprog -t at89 -r	  // reset 
 *  usbprog -t at89 -e	  // erase
 *  usbprog -t at89 -u test.bin // upload 
 *
 *  Types:
 *  at89 	- AT89...
 *  m8		- atmega8
 *	m32		- atmega32
 *
 *	Message to usbprog Hardware:
 *
 * 	type cmd 
 * 	
 *	upload:
 * 	type cmd startaddrhigh staraddrlow len 
 *
 *	erase:
 *	type cmd
 * 
 *  reset:
 *	type cmd

 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h> /* tolower(), toupper(), isalpha() */
#include <stdio.h>
#include <usb.h>

#include "config.h"

usb_dev_handle *locate_usbprog(void);
void usbprog_open(usb_dev_handle * usb_handle);

void show_help(void) {
	printf("\nAuthor: Benedikt Sauter <sauter@ixbat.de>"\
		"\nLicense: GNU General Public License V2\n"\
		"\nusbprog is software for programming Microcontrollers.\n"\
   		"\nUsage: usbprog [OPTION]\n"\
       	"\t-r  reset controller\n"\
       	"\t-e  erase code memory\n"\
       	"\t-u  load .bin into code memory\n "\
       	"\t-h  Show this help message\n"
       	"\t-v  version\n\n");
}


int getoptown(char *argument, char *option) {
   if( argument[0]=='-' && argument[1]==option[0] )
      return 1;
   return 0;
}

int main (int argc,char **argv)
{
  	struct usb_dev_handle *usb_handle;
  	struct usb_device *usb_device;
  	int send_status;
  	long erg,port,pin,value;
  	unsigned char send_data[4];
  	unsigned char receive_data=0;


  	usb_init();
  	//usb_set_debug(2);
  	if ((usb_handle = locate_usbprog())) {
   		usbprog_open(usb_handle);
  	}  
	else {
    	fprintf(stderr,"Could not open usbprog\n");
	}


   int counter=3;
   char buffer[BUF];
   size_t len=0;

   if(argc == 1 || getoptown(argv[1],"h") == TRUE ) {
      show_help();
      return EXIT_FAILURE;
   }

   else if(getoptown(argv[1],"v") == TRUE) {
      printf("Version 1.0\n");
      return EXIT_SUCCESS;
   }
   else if(argc < 2) {
      show_help();
      return EXIT_FAILURE;
   }
   if(getoptown(argv[1],"r") == TRUE)
      at89_reset(usb_handle,0);
   else if(getoptown(argv[1],"e") == TRUE)
      at89_erase(usb_handle,0);
   else if(getoptown(argv[1],"u") == TRUE)
   {
	if(argc < 3) {
		printf("upload file missing\n");
		return EXIT_FAILURE;
	}
    at89_upload(usb_handle,0,argv[2]);
   }
   else
      show_help();
   return EXIT_SUCCESS;


  usb_close(usb_handle);
 	
}	


void usbprog_open(usb_dev_handle * usb_handle)
{
  usb_set_configuration(usb_handle,1);
  usb_claim_interface(usb_handle,0);
  usb_set_altinterface(usb_handle,0);
 
}

usb_dev_handle *locate_usbprog(void) 
{
  unsigned char located = 0;
  struct usb_bus *bus;
  struct usb_device *dev;
  usb_dev_handle *device_handle = 0;
 		
  usb_find_busses();
  usb_find_devices();
 
  for (bus = usb_busses; bus; bus = bus->next)
  {
    for (dev = bus->devices; dev; dev = dev->next)	
    {
      if (dev->descriptor.idVendor == 0x0400) 
      {	
	located++;
	device_handle = usb_open(dev);
      }
    }	
  }

  if (device_handle==0) return (0);
  else return (device_handle);  	
}




   

