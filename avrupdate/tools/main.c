#include <stdio.h>
#include <usb.h>
//#include "avrupdate.h"


int main(int argc, char **argv)
{

  // start with the destination of your bin file
	
  struct usb_dev_handle* usb_handle;
  usb_handle = avrupdate_open(0x0400,0x5dc3);
 
  if(argc==2)
    avrupdate_flash_bin(usb_handle,argv[1]);


  avrupdate_startapp(usb_handle);
  avrupdate_close(usb_handle);

  return 0;
}
