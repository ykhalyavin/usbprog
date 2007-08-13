#include <stdio.h>    // to get "printf" function

#include "usbprog.h"

int main(int argc, char **argv)
{
  struct usbprog_context usbprog;
  usbprog_init(&usbprog);

  int i = usbprog_get_numberof_devices(&usbprog);
  printf("%i devices\n",i);

  char *buf[i];
  usbprog_print_devices(&usbprog,buf);

  for(i;i>0;i--)
    printf("%i %s\n",i,buf[i-1]);

  //if(usbprog.error_str!=NULL);
  //  printf("%s\n",usbprog.error_str);

  return 0;
}
