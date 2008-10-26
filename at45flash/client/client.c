/* Speedtest functions for USBprog */

#include <stdio.h>
#include <usb.h>

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>



#define CMD_PUT	  0x80
#define PUT_INC	  0x10

#define CMD_READ  0x40
#define CMD_WRITE 0x20

void cmd(int fd, uint8_t c, uint8_t d) {
  uint8_t buf[2]={c, d};
  uint8_t ibuf;
  int res;
  
  do {
    write(fd, buf, 2);
  
    /* page erase & programming time */
    if(buf[0] & CMD_WRITE)
      usleep(25000);
    
    res = read(fd,&ibuf,1);
    
    //printf("%02x %02x: %02x (%d)\n",buf[0], buf[1], ibuf, res);
    if(res>0)
      assert(buf[0] == ibuf);

    buf[0]&=~PUT_INC;
  } while(res<1);
}




usb_dev_handle *locate_usbprog(void);

int main (int argc,char **argv)
{
	struct usb_dev_handle *usbprog_handle;
	struct usb_device *usbprog_device;
	int send_status;
	int open_status;
	unsigned char send_data[10000];
	unsigned char receive_data[10000];

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

	int paddr=0,res, i;
	/* foreach page */
	while((res=read(0,&send_data[2],264))>0) {
          printf("%d\n",paddr);
	  send_data[0] = (unsigned char)paddr;
	  send_data[1] = (unsigned char)(paddr>>8);

	  //write_page(fd, paddr, pagebuf, res);
	  if( usb_bulk_write(usbprog_handle,2,send_data,266,500) < 0 );
	  else printf("usb write error\n");
	  if( usb_bulk_read(usbprog_handle,2,receive_data,320,500) < 0);	
	  else printf("usb read error\n");
/*
	  for(i=0;i< 264; i++)
	  {
	    if(send_data[i]!=receive_data[i])
	      printf("Error Byte %i Data Out %02x Data In %02x\n",send_data[i], receive_data[i]);
	  }
*/
    	  paddr++;
    	}

	//int k;
	//for(k=0;k<10000;k++)
//		send_data[k]=(unsigned char)k;

	send_data[0] = 0x77;
	send_data[1] = 0x88;


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




   

