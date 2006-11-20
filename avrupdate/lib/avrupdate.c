#include <stdio.h>
#include <string.h>
#include <usb.h>

#include "avrupdate.h"
//#include <curl/curl.h>
#include "http_fetcher.h"

#define STARTAPP  0x01
#define WRITEPAGE 0x02

void avrupdate_flash_bin(struct usb_dev_handle* usb_handle,char *file)
{
  	char buf[64];
  	char cmd[64];

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
  	fd = fopen(file, "r");
  	if(!fd) {
    	fprintf(stderr, "Unable to open file %s, ignoring.\n", file);
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
      		usb_bulk_write(usb_handle,2,cmd,64,100);

      		// data message 
      		usb_bulk_write(usb_handle,2,buf,64,100);
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

    	// data message 
    	usb_bulk_write(usb_handle,2,buf,64,100);
  	}	 
}


void avrupdate_startapp(struct usb_dev_handle* usb_handle)
{
  	char buf[64];
  	char *ptr = buf;

 
   	buf[0]=STARTAPP;
   	usb_bulk_write(usb_handle,2,ptr,64,100);
   	//printf("TX stat=%d\n",stat);
}



void avrupdate_start_with_vendor_request(short vendorid, short productid)
{
	struct usb_bus *busses;

  	usb_set_debug(2);
  	usb_init();
  	usb_find_busses();
  	usb_find_devices();

  	busses = usb_get_busses();

 	struct usb_dev_handle* usb_handle;
  	struct usb_bus *bus;


  	unsigned char send_data=0xff;

  	for (bus = busses; bus; bus = bus->next)
  	{
    	struct usb_device *dev;

    	for (dev = bus->devices; dev; dev = dev->next){
      		if (dev->descriptor.idVendor == vendorid){
        		int i,stat;
        		printf("vendor: %i\n",dev->descriptor.idVendor);
        		usb_handle = usb_open(dev);
        		stat = usb_set_configuration (usb_handle,1);
				usb_control_msg(usb_handle, 0xC0, 0x01, 0, 0, NULL,8, 100);
				usb_close(usb_handle);
				//return usb_handle;
      		}
    	}	
  	}
}

struct usb_dev_handle* avrupdate_open(short vendorid, short productid)
{
	struct usb_bus *busses;

  	usb_set_debug(2);
  	usb_init();
  	usb_find_busses();
  	usb_find_devices();

  	busses = usb_get_busses();

 	struct usb_dev_handle* usb_handle;
  	struct usb_bus *bus;


  	unsigned char send_data=0xff;

  	for (bus = busses; bus; bus = bus->next)
  	{
    	struct usb_device *dev;

    	for (dev = bus->devices; dev; dev = dev->next){
      		if (dev->descriptor.idVendor == vendorid){
        		int i,stat;
        		printf("vendor: %i\n",dev->descriptor.idVendor);
        		usb_handle = usb_open(dev);
        		stat = usb_set_configuration (usb_handle,1);
				return usb_handle;	
      		}
    	}	
  	}
}



char avrupdate_get_version()
{

}


void avrupdate_set_version(char version)
{

}



void avrupdate_close(struct usb_dev_handle* usb_handle)
{
  	usb_close(usb_handle);
}

size_t _write_data(void *data, size_t size, size_t nmemb, void *userp)
{
	strcpy(userp,data);
	return size*nmemb;
}


int avrupdate_net_get_versionfile(char * url, char **buffer)
{
	return http_fetch(url, buffer); 
}

int avrupdate_net_versions(char * url)
{
	char *buffer;
	int size = avrupdate_net_get_versionfile(url,&buffer);
	printf("data %i\n",size);
	int i; int star=0;

	// count semicollons
	for(i=0;i<size;i++)
	{
		if(buffer[i]=='*')
			star++;
	}
	return star;
}


void avrupdate_net_flash_version(char * url,int number, int vendorid, int productid)
{
	struct avrupdate_info * tmp = avrupdate_net_get_version_info(url,number);
	
	int ret;
	char *buffer;
	ret = http_fetch(tmp->file, &buffer);    /* Downloads page */
	if(ret == -1)                       /* All HTTP Fetcher functions return */
		http_perror("http_fetch");      /*  -1 on error. */
	else {
   		printf("Page successfully downloaded. (%s)\n", url);
		FILE *fp;
		fp = fopen("flash.bin", "w");
		int i;
		for(i=0;i<ret;i++)
			fputc(buffer[i], fp);
			//fprintf(fp,"%s",buffer);
		fclose(fp);

		struct usb_dev_handle* usb_handle = avrupdate_open(vendorid,productid);
		avrupdate_flash_bin(usb_handle,"flash.bin");
		avrupdate_close(usb_handle);

		//remove("flash.bin");
	}
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


struct avrupdate_info * avrupdate_net_get_version_info(char * url,int number)
{
	char *buffer;

	avrupdate_net_get_versionfile(url,&buffer);
	struct avrupdate_info * tmp = (struct avrupdate_info *)malloc(sizeof(struct avrupdate_info));

	int rows = avrupdate_net_versions(url);

	char **line = malloc(rows* sizeof(char*));
    split_string(buffer, line, "*");

	char **fields = malloc(4* sizeof(char*));
    split_string(line[number],fields, ";");

	tmp->title = fields[0];
	tmp->version = fields[1];
	tmp->file = fields[2];
	tmp->description= fields[3];

	
	return tmp;
}

void avrupdate_setlog(char *msg)
{
	logfile = msg;
}


char * avrupdate_log()
{
	if(logfile!=NULL)
		logfile = NULL;
	
	return logfile;
}
