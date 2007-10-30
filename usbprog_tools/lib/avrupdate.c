#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <usb.h>

#include "avrupdate.h"

#if WITHNETWORKSUPPORT
#include "http_fetcher.h"
#endif

#define STARTAPP       0x01
#define WRITEPAGE      0x02
#define GETVERSION     0x03
#define SETVERSION     0x04
#define STOPPROGMODE   0x05

/*
 * Must be called as first function
 */
void avrupdate_init(int debuglevel)
{
    usb_init();
    usb_find_busses();

    if (debuglevel > 0)
        usb_set_debug(1);
}

/*
 * Helper function to find a USB device.
 */
static struct usb_device *avrupdate_find_device(uint16_t vendorid,
                                                uint16_t prodid)
{
    struct usb_device   *dev;
    struct usb_bus      *bus;

    usb_find_devices();
    for (bus = usb_get_busses(); bus; bus = bus->next)
        for (dev = bus->devices; dev; dev = dev->next)
            if (dev->descriptor.idVendor == vendorid &&
                    dev->descriptor.idProduct == prodid)
                return dev;

    return NULL;
}

int avrupdate_flash_bin(struct usb_dev_handle* usb_handle,char *file)
{
    char buf[64];
    char cmd[64];
    int ret;

    FILE *fd;


    // get page size
    // get flash size

    // send every page in a usb package
    // 1. header ( action, page number)
    // 2. data   

    // if you have got a controller with blocksize greater than 64
    // work with the easyavr like you have an 64 page size flash
    // the controller manages the rest

    //printf("send %s to easyavr\r\n",file);

    int page=0;
    int offset=0;

    // open bin file
    fd = fopen(file, "r+b");
    if(!fd) {
        fprintf(stderr, "Unable to open file %s, ignoring.\n", file);
        return -1;
    }

    while(!feof(fd))
    {
        buf[offset]=fgetc(fd);

        offset++;
        if(offset == 64)
        {
            //printf("send package\n");
            // command message
            cmd[0]=WRITEPAGE;
            cmd[1]=(char)page; // page number
            ret = usb_bulk_write(usb_handle,2,cmd,64,100);
            if (ret < 0) {
                printf("Error while writing to USB device: %s\n", usb_strerror());
                return -1;
            }


            // data message
            ret = usb_bulk_write(usb_handle,2,buf,64,100);
            if (ret < 0) {
                printf("Error while writing to USB device: %s\n", usb_strerror());
                return -1;
            }
            offset = 0;
            page++;
        }
    }
    if(offset > 0)
    {
        //printf("rest\n");
        // command message
        cmd[0]=WRITEPAGE;
        cmd[1]=(char)page; // page number
        usb_bulk_write(usb_handle,2,cmd,64,100);
        if (ret < 0) {
            printf("Error while writing to USB device: %s\n", usb_strerror());
            return -1;
        }

        // data message
        usb_bulk_write(usb_handle,2,buf,64,100);
        if (ret < 0) {
            printf("Error while writing to USB device: %s\n", usb_strerror());
            return -1;
        }
    }   

    return 0;
}


void avrupdate_startapp(struct usb_dev_handle* usb_handle)
{
    char buf[64];
    char *ptr = buf;

//     buf[0]=STOPPROGMODE;
//     usb_bulk_write(usb_handle,2,ptr,64,100);

     buf[0]=STARTAPP;
     usb_bulk_write(usb_handle,2,ptr,64,100);
}

static const char *get_device_name_for_type(int type)
{
    const char *devices[] = {
        "USBprog in update mode",                   /* 0 */
        "USBprog with blinkdemo",                   /* 1 */
        "USBprog",                                  /* 2 */
        "USBprog with AVRisp MKII",                 /* 3 */
        "USBprog with JTAG ICE MKII",               /* 4 */
        "USBprog with OpenOCD",			    /* 5 */
        "USBprog with Simpleport",		    /* 6 */
        "USBprog with XSVF Player"		    /* 7 */
    };

    if (type == UNKNOWN)
        return "Unknown device";
    if (type >= 0 || type < array_size(devices))
        return devices[type];

    return NULL;
}

int avrupdate_find_usbdevice(struct device *device)
{
    struct usb_device   *dev, *found_dev = NULL;
    enum DeviceType     type = UNKNOWN;
    struct usb_bus 	    *bus;

    //usb_set_debug(2);

    if (!device)
        return -EINVAL;

    /* rescan */
    usb_find_devices();
    for (bus = usb_get_busses(); bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
            switch (dev->descriptor.idVendor) {
                case USB_VID_ATMEL:
                    if(dev->descriptor.idProduct == USB_PID_AVRISPMKII)
                        type = AVRISPMKII;
                    else if (dev->descriptor.idProduct == USB_PID_JTAGICEMKII)
                        type = JTAGICEMKII;
                    found_dev = dev;
                    break;

                case USB_VID_USBPROG:
                    if (is_between(dev->descriptor.idProduct,
                                USB_PID_USBPROG_L, USB_PID_USBPROG_H)) {
                        if (dev->descriptor.bcdDevice==AVRUPDATE)
                            type = AVRUPDATE;
                        else if (dev->descriptor.bcdDevice==0x0200)
                            type = USBPROG;
                        else if (dev->descriptor.bcdDevice ==BLINKDEMO)
                            type = BLINKDEMO;
                        else if (dev->descriptor.idProduct ==0x0c63)
                            type = OPENOCD;
                        else
                            type = BLINKDEMO;
                        found_dev = dev;
                    }
                    break;

                default:
                    break;
            }
        }
    }

    if (found_dev) {
        device->type = type;
        device->productid = found_dev->descriptor.idProduct;
        device->vendorid = found_dev->descriptor.idVendor;
        device->description = get_device_name_for_type(device->type);
        return 0;
    }

    return -1;
}

int avrupdate_start_with_vendor_request(struct device *usb_device)
{
    struct usb_dev_handle   *usb_handle = NULL;
    struct usb_device       *dev;
    int                     usb_interface;
    int                     ret;

    // usb_set_debug(2);

    /*
     * no need to run the procedure here
     */
    if (usb_device->type == USBPROG)
        return 0;

    dev = avrupdate_find_device(usb_device->vendorid,
            usb_device->productid);
    if (!dev) {
        printf("Device %s doesn't exist\n", usb_device->description);
        return -ENODEV;
    }

    usb_handle = usb_open(dev);
    if (!usb_handle) {
        printf("Could not open USB device: %s", usb_strerror());
        goto err;
    }

    usb_set_configuration (usb_handle, dev->config[0].bConfigurationValue);

    usb_interface = dev->config[0].interface[0].altsetting[0].bInterfaceNumber;
    ret = usb_claim_interface(usb_handle, usb_interface);
    if (ret < 0) {
        printf("Error when claiming interface %d: %s\n",
                usb_interface, usb_strerror());
        goto err;
    }

    /* needed ?*/
    usb_set_altinterface(usb_handle, 0);
    if (ret < 0) {
        printf("Error when setting altinterface to 0: %s\n", usb_strerror());
        goto err;
    }

    int timeout = 6;

    while(usb_control_msg(usb_handle, 0xC0, 0x01, 0, 0, NULL, 8, 1000) < 0){
        if (--timeout == 0) {
            printf("Timeout exceeded, it's possible that switching to "
                    "update mode failed \n(%s)\n", usb_strerror());
            break;
        }
        sleep(1);
    }

    usb_release_interface(usb_handle, usb_interface);
    usb_close(usb_handle);

    return 0;

err:
    if (usb_handle)
        usb_release_interface(usb_handle, usb_interface);
    return -1;
}

struct usb_dev_handle* avrupdate_open(struct device *usb_device)
{
    struct usb_dev_handle   *usb_handle;
    struct usb_device       *dev;
    int                     usb_interface;
    int                     ret;

    dev = avrupdate_find_device(usb_device->vendorid,
            usb_device->productid);
    if (!dev) {
        printf("Device %s doesn't exist\n", usb_device->description);
        goto err;
    }

    usb_handle = usb_open(dev);
    if (!usb_handle) {
        printf("usb_open failed: %s\n", usb_strerror());
        goto err;
    }

    usb_set_configuration (usb_handle, dev->config[0].bConfigurationValue);

    usb_interface = dev->config[0].interface[0].altsetting[0].bInterfaceNumber;
    ret = usb_claim_interface(usb_handle, usb_interface);
    if (ret < 0) {
        printf("Claiming interface failed: %s\n", usb_strerror());
        goto err_close;
    }

    /* ret = usb_set_altinterface(usb_handle, 0); */
    if (ret < 0) {
        printf("Setting alternative configuration to 0 failed: %s\n",
                usb_strerror());
        goto err_release;
    }

    return usb_handle;

err_release:
    usb_release_interface(usb_handle, usb_interface);
err_close:
    usb_close(usb_handle);
err:
    return NULL;
}



char avrupdate_get_version(struct usb_dev_handle* usb_handle)
{
  char cmd[2];
  char buf[2];
  cmd[0]=GETVERSION;
  usb_bulk_write(usb_handle,1,cmd,64,100);
   usb_bulk_read(usb_handle,1,buf,64,100);
  
  return buf[0];
}


void avrupdate_set_version(char version, struct usb_dev_handle* usb_handle)
{
}



void avrupdate_close(struct usb_dev_handle* usb_handle)
{
/*
    usb_reset(usb_handle);
    usb_set_configuration(usb_handle,1);
    sleep(2);
    */
    usb_release_interface(usb_handle, 1);
    usb_close(usb_handle);
}

size_t _write_data(void *data, size_t size, size_t nmemb, void *userp)
{
  strcpy(userp,data);
  return size*nmemb;
}


int avrupdate_net_get_versionfile(const char *url, char **buffer)
{
#if WITHNETWORKSUPPORT
  return http_fetch(url, buffer); 
#else
  return 0;  
#endif

}

int avrupdate_net_versions(const char *url)
{
  char *buffer;
  int size = avrupdate_net_get_versionfile(url, &buffer);
  //printf("data %i\n",size);
  int i; int star=0;

  // count semicollons
  for(i=0;i<size;i++)
  {
    if(buffer[i]=='*')
      star++;
  }
  return star;
}


void avrupdate_net_flash_version(const char     *url,
                                 int            number,
                                 struct device  *usb_device)
{
#if WITHNETWORKSUPPORT
    struct avrupdate_info * tmp = avrupdate_net_get_version_info(url,number);

    int ret;
    char *buffer;
    ret = http_fetch(tmp->file, &buffer);   /* Downloads page */
    if (ret == -1)                          /* All HTTP Fetcher functions return */
        http_perror("http_fetch");          /*  -1 on error. */
    else {
        printf("Page successfully downloaded. (%s)\n", url);
        FILE *fp;
        fp = fopen("flash.bin", "w+b");
        int i;
        for(i=0;i<ret;i++)
            fputc(buffer[i], fp);
        //fprintf(fp,"%s",buffer);
        fclose(fp);

        //while(avrupdate_find_usbdevice()!=AVRUPDATE);

        struct usb_dev_handle* usb_handle = avrupdate_open(usb_device);
        avrupdate_flash_bin(usb_handle,"flash.bin");
        avrupdate_startapp(usb_handle);
        avrupdate_close(usb_handle);

        remove("flash.bin");
    }
#endif
}


void split_string(char *c_str, char **ret, char *delim)
{
  int i = 0;
  char* tmp;    
  char str[strlen(c_str)+1];
  strcpy(str, c_str);

  tmp = strtok(str, delim);
  while (tmp) {
    ret[i] = malloc(strlen(tmp) * sizeof(char) +1);
    strcpy(ret[i++], tmp);
    tmp = strtok(NULL, delim);
  }
}


struct avrupdate_info * avrupdate_net_get_version_info(const char *url, int number)
{
  char *buffer;

#if WITHNETWORKSUPPORT
  avrupdate_net_get_versionfile(url,&buffer);
#endif

  struct avrupdate_info * tmp = (struct avrupdate_info *)malloc(sizeof(struct avrupdate_info));
#if WITHNETWORKSUPPORT
  int rows = avrupdate_net_versions(url);

  char **line = malloc(rows* sizeof(char*));
    split_string(buffer, line, "*");

  char **fields = malloc(4* sizeof(char*));
    split_string(line[number],fields, ";");

  tmp->title = fields[0];
  tmp->version = fields[1];
  tmp->file = fields[2];
  tmp->description= fields[3];
#endif
  
  return tmp;
}

/*void avrupdate_setlog(char *msg)
{
  logfile = msg;
}*/


/*char * avrupdate_log()
{
  if(logfile!=NULL)
    logfile = NULL;
  
  return logfile;
}*/
