#ifndef __USBPROG_H__
#define __USBPROG_H__

#include <usb.h>
//#include "../include/basic.h"

#define DEV_VID								0x1786
#define DEV_PID								0x0C63
#define EP_OUT								0x02
#define EP_IN								0x82
#define USB_PACKAGE_SIZE					64

#define DEFAULT_TIMEOUT						500

#define USBPROG_STATUS_OK					 0
#define USBPROG_STATUS_NOT_FOUND			-1
#define USBPROG_STATUS_SET_FAILED			-2
#define USBPROG_STATUS_TIMEOUT				-3
#define USBPROG_STATUS_INVALID_PARAM		-4
#define USBPROG_STATUS_SEND_ERROR			-5
#define USBPROG_STATUS_RECEIVE_ERROR		-6
#define USBPROG_STATUS_EXECUTE_ERROR		-7
#define USBPROG_STATUS_INVALID_OPTIONS		-8
#define USBPROG_STATUS_ERROR				-9


void usbprog_init(void);
int  usbprog_open(void);
void usbprog_close(void);

int  usbprog_send(char *buf, int size, int timeout = DEFAULT_TIMEOUT);
int  usbprog_receive(char *buf, int max_size, int timeout = DEFAULT_TIMEOUT);

#endif  /* __USBPROG_H__ */

