#include <stdio.h>
#include <usb.h>
#include "../lib/avrupdate.h"

#define AVRUPDATE   0x00
#define BLINKDEMO   0x01
#define USBPROG     0x02
#define AVRISPMKII  0x03


int main(int argc, char **argv)
{
	
 	// start with the destination of your bin file
	
 	struct usb_dev_handle* usb_handle;

	// find a device
	int device = avrupdate_find_usbdevice();

	switch(device) {
		case AVRUPDATE:
			printf("usbprog is ready for an upgrade\n");
		break;
		case BLINKDEMO:
			printf("usbprog firmware found: blinkdemo\n");
			avrupdate_start_with_vendor_request(0x1781,0x0c62);
			sleep(3);
		break;
		case USBPROG:
			printf("usbprog firmware found: usbprog (Benes ISP)\n");
			avrupdate_start_with_vendor_request(0x1781,0x0c62);
			sleep(3);
		break;
		case AVRISPMKII:
			printf("usbprog firmware found: AVRISP mk2 Klon\n");
			avrupdate_start_with_vendor_request(0x03eb,0x2104);
			sleep(3);
		break;
		default:
			printf("Error: Can't find vaild usbprog adapter on usb bus.\n \
			Be sure that you are root or have enough permissions to access usb.\n");
			return -1;
	}

 	if(argc!=2) {
		printf("Error: Firmware file missing! (usbprog avrispmk2.bin)\n");
		return -1;
	}


	usb_handle = avrupdate_open(0x1781,0x0c62);
 	avrupdate_flash_bin(usb_handle,argv[1]);
 	avrupdate_startapp(usb_handle);
 	avrupdate_close(usb_handle);


	return;
#if 0
	
	avrupdate_start_with_vendor_request(0x03eb,0x2104);
	
	sleep(2);

	usb_handle = avrupdate_open(0x1781,0x0c62);
 
  	if(argc==2)
    	avrupdate_flash_bin(usb_handle,argv[1]);


  	avrupdate_startapp(usb_handle);
  	avrupdate_close(usb_handle);
#endif
/*
	char *url = "http://www.ixbat.de/versions.conf";
	printf("Found %i version(s)\n",avrupdate_net_versions(url));	

	struct avrupdate_info *tmp = avrupdate_net_get_version_info(url,1);

	printf("titel %s\n",tmp->title);
	printf("version %s\n",tmp->version);
	printf("file %s\n",tmp->file);
	printf("description %s\n",tmp->description);


	avrupdate_net_flash_version(url,0);
*/	

  	return 0;
}
