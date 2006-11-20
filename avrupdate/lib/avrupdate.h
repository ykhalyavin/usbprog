#include <stdio.h>
#include <usb.h>


void avrupdate_start_with_vendor_request(short vendorid, short productid);


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

char *logfile;
char * avrupdate_log();
void avrupdate_setlog(char *msg);

struct avrupdate_info {
	char *title;
	char *version;
	char *file;
	char *description;
};

/* online functions */
int avrupdate_net_get_versionfile(char * url, char **buffer);
int avrupdate_net_versions(char * url);
void avrupdate_net_flash_version(char * url,int number,int vendorid, int productid);
struct avrupdate_info * avrupdate_net_get_version_info(char * url,int number);



/* implicit functionst */

void avrupdate_flash_version(int version);





/*
 * avrupdate
 * 	-P ProductID 
 * 	-V VendorID 
 * 	-f file to upload
 * 	-v actual version in mc
 * 	-U URL of version file
 * 	-s show online versions
 * 	-d version, download version
 *
 */

