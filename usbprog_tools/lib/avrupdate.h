#include <stdio.h>
#include <stdint.h>

#include <usb.h>

#define WITHNETWORKSUPPORT	1
#define array_size(a)	((int)(sizeof(a)/sizeof(a[0])))


/*
 * don't forget to add a symbolic description to the string table in
 * get_device_name_for_type()
 */

enum DeviceType {
    UNKNOWN	    = -1,
    AVRUPDATE   = 0x00,
    BLINKDEMO   = 0x01,
    USBPROG	    = 0x02,
    AVRISPMKII  = 0x03,
    JTAGICEMKII = 0x04,
    OPENOCD = 0x05,
    SIMPLEPORT = 0x06,
    XSVF = 0x07
};

struct device {
    enum DeviceType type;
    uint16_t	vendorid;
    uint16_t	productid;
    const char      *description;
};

#define USB_VID_ATMEL       0x03eb
#define USB_VID_USBPROG     0x1781
#define USB_PID_AVRISPMKII  0x2104
#define USB_PID_JTAGICEMKII 0x2103
#define USB_PID_USBPROG_L   0x0c62  /* lower bounds for the range */
#define USB_PID_USBPROG_H   0x0c65  /* upper bounds for the range */

#define is_between(val, a, b) \
    (((val) >= (a)) && ((val) <= (b)))

void avrupdate_init(int debuglevel);
int avrupdate_start_with_vendor_request(struct device *usb_device);

/* start usb connection to device */
struct usb_dev_handle* avrupdate_open(struct device *usb_device);
void avrupdate_close(struct usb_dev_handle* usb_handle);

/* lowlevel funktions for avr */
int avrupdate_flash_bin(struct usb_dev_handle* usb_handle,char *file);
void avrupdate_startapp(struct usb_dev_handle* usb_handle);

int avrupdate_find_usbdevice();

//char avrupdate_get_version(struct usb_dev_handle* usb_handle);
//void avrupdate_set_version(char version, struct usb_dev_handle* usb_handle);


/* check state of the controller -> ready for programming */
//int avrupdate_uc_state();

//char *logfile;
//char * avrupdate_log();
//void avrupdate_setlog(char *msg);

struct avrupdate_info {
	char *title;
	char *version;
	char *file;
	char *description;
};

/* online functions */
int avrupdate_net_get_versionfile(const char *url, char **buffer);
int avrupdate_net_versions(const char *url);
void avrupdate_net_flash_version(const char *, int, struct device *);
struct avrupdate_info * avrupdate_net_get_version_info(const char *url,int number);


/* implicit function */

void avrupdate_flash_version(int version);


/*
 * inline stuff
 */

static inline int avrupdate_need_switch_update_mode(struct device *usb_device)
{
    return usb_device->type != AVRUPDATE;
}



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

