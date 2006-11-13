#include <stdio.h>
#include <usb.h>


void avrupdate_flash_bin(struct usb_dev_handle* usb_handle,char *file);
void avrupdate_startapp(struct usb_dev_handle* usb_handle);
struct usb_dev_handle* avrupdate_open();
void avrupdate_close(struct usb_dev_handle* usb_handle);
