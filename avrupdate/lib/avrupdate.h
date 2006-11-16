#include <stdio.h>
#include <usb.h>

/* start usb connection to device */
struct usb_dev_handle* avrupdate_open(short vendorid, short productid);
void avrupdate_close(struct usb_dev_handle* usb_handle);

/* lowlevel funktions for avr */
void avrupdate_flash_bin(struct usb_dev_handle* usb_handle,char *file);
void avrupdate_startapp(struct usb_dev_handle* usb_handle);
char avrupdate_get_version();
void avrupdate_set_version(char version);

/* check state of the controller -> ready for programming */
int avrupdate_uc_state();


/* online functions */
void avrupdate_net_version_list(char * url);
void avrupdate_net_get_version();



/* implicit functionst */

void avrupdate_flash_version(int version);


