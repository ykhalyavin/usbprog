/* usbn960x.c
* Copyright (C) 2006  Benedikt Sauter
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <usb.h>
#include "../lib/avrupdate.h"

#define AVRUPDATE   0x00
#define BLINKDEMO   0x01
#define USBPROG     0x02
#define AVRISPMKII  0x03
#define JTAGICEMKII 0x04


int main(int argc, char **argv)
{
	
 	// start with the destination of your bin file
	
 	struct usb_dev_handle* usb_handle;
 	printf("usbprog GNU/GPL2 Update Tool\n");
 	printf("Author: Benedikt Sauter, sauter@ixbat.de 2007\n");
 	printf("Firmware Pool: http://www.embedded-projects.net/usbprog\n\n");

	// find a device
	int device = avrupdate_find_usbdevice();

	switch(device) {
		case AVRUPDATE:
			printf("usbprog found with: update mode\n");
		break;
		case BLINKDEMO:
			printf("usbprog found with: blinkdemo\n");
			printf("start update mode\n");
			avrupdate_start_with_vendor_request(0x1781,0x0c62);
			printf("please wait some seconds...\n");
			#if _WIN32
			Sleep(7000);
			#else
			sleep(3);
			#endif
			
		break;
		case USBPROG:
			printf("usbprog found with:usbprog (Benes ISP)\n");
			printf("start update mode\n");
			avrupdate_start_with_vendor_request(0x1781,0x0c62);
			printf("please wait some seconds...\n");
			#if _WIN32
			Sleep(7000);
			#else
			sleep(3);
			#endif
		break;
		case AVRISPMKII:
			printf("usbprog found with: AVRISP mk2 Klon\n");
			printf("start update mode\n");
			avrupdate_start_with_vendor_request(0x03eb,0x2104);
			printf("please wait some seconds...\n");
			#if _WIN32
			Sleep(7000);
			#else
			sleep(3);
			#endif

		break;
		case JTAGICEMKII:
			printf("usbprog found with: JTAGICE mk2 Klon\n");
			printf("start update mode\n");
			avrupdate_start_with_vendor_request(0x03eb,0x2103);
			printf("please wait some seconds...\n");
			#if _WIN32
			Sleep(7000);
			#else
			sleep(3);
			#endif

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
	else {
		printf("start update to %s \n",argv[1]);
	}


	usb_handle = avrupdate_open(0x1781,0x0c62);
 	avrupdate_flash_bin(usb_handle,argv[1]);
 	printf("start new firmware\n\n");
 	avrupdate_startapp(usb_handle);
 	avrupdate_close(usb_handle);

	printf("Have fun with your new adapter!\n");


	return;
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
}
