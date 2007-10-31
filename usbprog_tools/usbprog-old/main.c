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
#include <stdlib.h>
#include <usb.h>
#include "../lib/avrupdate.h"

#define USB_DEBUGLEVEL 0

static inline void wait_after_mode_switch(void)
{
#if _WIN32
        Sleep(7000);
#else
        sleep(3);
#endif
}

int main(int argc, char **argv)
{
    int             ret;
    struct device   usb_dev;

    // start with the destination of your bin file
    avrupdate_init(USB_DEBUGLEVEL);

    struct usb_dev_handle* usb_handle;
    printf("--- usbprog --------------------------------------------------\n");
    printf("    GNU GPLv2 Update Tool\n");
    printf("    Author         : Benedikt Sauter <sauter@ixbat.de>, 2007\n");
    printf("    Firmware Pool  : http://www.embedded-projects.net/usbprog\n");
    printf("--------------------------------------------------------------\n\n");

    // find a device
    ret = avrupdate_find_usbdevice(&usb_dev);
    if (ret < 0) {
        printf("Error: Can't find vaild usbprog adapter on usb bus.\n"
                "Be sure that you are root or have enough permissions to "
                "access usb.\n");
        goto err;
    }

    printf("=> Usbprog found with: %s\n", usb_dev.description);

    if (argc != 2) {
        printf("ERROR: Firmware file missing! (Usage: usbprog <firmware>\n");
        goto err;
    }

    if (avrupdate_need_switch_update_mode(&usb_dev)) {
        printf("=> Starting update mode...\n");
        ret = avrupdate_start_with_vendor_request(&usb_dev);
        if (ret < 0)
            return EXIT_FAILURE;
        printf("   Please wait some seconds...\n");
        wait_after_mode_switch();
    }

    // look again, that's just for devices that change their USB ID
    // after switching to update mode
    ret = avrupdate_find_usbdevice(&usb_dev);
    if (ret < 0) {
        printf("Error: No suitable device found after switching to update mode\n");
        goto err;
    }

    printf("=> Using device '%s' to update\n", usb_dev.description);
    printf("=> Starting update to %s\n", argv[1]);

    usb_handle = avrupdate_open(&usb_dev);
    if (!usb_handle) {
        printf("Not possible to open USB device.\n");
        goto err;
    }
    ret = avrupdate_flash_bin(usb_handle,argv[1]);
    if (ret < 0)
        goto err;
    printf("=> Starting new firmware\n\n");
    avrupdate_startapp(usb_handle);
    avrupdate_close(usb_handle);

    printf("Have fun with your new adapter!\n");


    return EXIT_SUCCESS;

err:
    return EXIT_FAILURE;
#if 0
    char *url = "http://www.ixbat.de/versions.conf";
    printf("Found %i version(s)\n",avrupdate_net_versions(url));

    struct avrupdate_info *tmp = avrupdate_net_get_version_info(url,1);

    printf("titel %s\n",tmp->title);
    printf("version %s\n",tmp->version);
    printf("file %s\n",tmp->file);
    printf("description %s\n",tmp->description);


    avrupdate_net_flash_version(url,0);
#endif
}
