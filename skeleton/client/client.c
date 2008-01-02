/* Speedtest functions for USBprog */

#include <stdio.h>
#include <usb.h>

usb_dev_handle *locate_usbprog(void);

int main (int argc,char **argv)
{
	struct usb_dev_handle *usbprog_handle;
	struct usb_device *usbprog_device;
	int send_status;
	int open_status;
	unsigned char send_data[10000];
	unsigned char receive_data[64];

	usb_init();
	usb_set_debug(2);

 	if ((usbprog_handle = locate_usbprog())==0) 
	{
		printf("Could not open the USBprog device\n");
		return (-1);
	}  

	open_status = usb_set_configuration(usbprog_handle,1);
	printf("conf_stat=%d\n",open_status);
	
	open_status = usb_claim_interface(usbprog_handle,0);
	printf("claim_stat=%d\n",open_status);
	
	open_status = usb_set_altinterface(usbprog_handle,0);
	printf("alt_stat=%d\n",open_status);


	int k;
	for(k=0;k<10000;k++)
		send_data[k]=(unsigned char)k;


	send_data[0] = 0x77;
	send_data[1] = 0x88;

	usb_bulk_write(usbprog_handle,2,send_data,320,500);
	usb_bulk_read(usbprog_handle,2,receive_data,320,500);	

	usb_close(usbprog_handle);
}	

usb_dev_handle *locate_usbprog(void) 
{
	unsigned char located = 0;
	struct usb_bus *bus;
	struct usb_device *dev;
	usb_dev_handle *device_handle = 0;
 		
	usb_find_busses();
	usb_find_devices();
 
 	for (bus = usb_busses; bus; bus = bus->next)
	{
		for (dev = bus->devices; dev; dev = dev->next)	
		{
			if (dev->descriptor.idVendor == 0x1786) 
			{	
				located++;
				device_handle = usb_open(dev);
				printf("USBprog Device Found @ Address %s \n", dev->filename);
				printf("USBprog Vendor ID 0x0%x\n",dev->descriptor.idVendor);
				printf("USBprog Product ID 0x0%x\n",dev->descriptor.idProduct);
			}
			else printf("** usb device %s found **\n", dev->filename);			
		}	
  }

  if (device_handle==0) return (0);
  else return (device_handle);  	
}




   

