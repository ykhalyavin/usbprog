#include "usbprog.h"

#include <stdio.h>

/* the device handle */
static usb_dev_handle *g_dev = NULL;

void usbprog_init(void)
{
	g_dev = NULL;
}

int  usbprog_open(void)
{
	struct usb_bus *bus;
	struct usb_device *dev;

	usb_init();
	//usb_set_debug(2);
	usb_find_busses();
	usb_find_devices();

    g_dev = NULL;
	for(bus = usb_get_busses(); bus; bus = bus->next)
		for(dev = bus->devices; dev; dev = dev->next)
			if ( dev->descriptor.idVendor == DEV_VID
				 && dev->descriptor.idProduct == DEV_PID )
				g_dev = usb_open(dev);

	if(g_dev == NULL) {
		printf("Error: device not found!\n");
		return USBPROG_STATUS_NOT_FOUND;
	}

	if (usb_set_configuration(g_dev, 1) < 0) {
		printf("Error: setting config 1 failed!\n");
		usb_close(g_dev);
		g_dev = NULL;
		return USBPROG_STATUS_SET_FAILED;
	}

	if (usb_claim_interface(g_dev, 0) < 0) {
		printf("Error: claiming interface 0 failed!\n");
		usb_close(g_dev);
		g_dev = NULL;
		return USBPROG_STATUS_SET_FAILED;
	}

	return USBPROG_STATUS_OK;
}

void usbprog_close(void)
{
	usb_release_interface(g_dev, 0);
	usb_close(g_dev);
}

int usbprog_send(char *buf, int size, int timeout)
{
	return usb_bulk_write(g_dev, EP_OUT, buf, size, timeout);
}

int usbprog_receive(char *buf, int max_size, int timeout)
{
	return usb_bulk_read(g_dev, EP_IN, buf, max_size, timeout);
}

