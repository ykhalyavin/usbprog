#include <stdio.h>    // to get "printf" function

#include "http_fetcher.h"
//extern "C" int http_fetch(const char *url, char **fileBuf);


int main(int argc, char **argv)
{
 
  char url[] = "http://www.ixbat.de/usbprog/versions.conf";
  char * buffer;

  http_fetch(url, &buffer); 

  printf("%s",buffer);

  
  return 0;
}
