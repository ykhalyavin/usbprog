#include <stdio.h>    // to get "printf" function

#include "usbprog.h"

int main(int argc, char **argv)
{
  /**** initialize usbprog lib!!!! *****/
  printf("\n\nInitialize usbprog lib\n++++++++++++++++++++++++++++\n");
  struct usbprog_context usbprog;
  usbprog_init(&usbprog);

  /**** device select field !!!! *****/
  printf("\n\nGet number of devices\n++++++++++++++++++++++++++++\n");
  int i = usbprog_get_numberof_devices(&usbprog);
  printf("Found %i devices\n",i);

  char *buf[i];
  usbprog_print_devices(&usbprog,buf);

  int j=0;
  for(i;i>0;i--){
    printf("%i %s\n",j,buf[i-1]);
    j++;
  }
  
  /**** get online versions.xml !!!! *****/
  printf("\n\nGet versions.xml\n++++++++++++++++++++++++++++\n");

  char url[] = "http://www.ixbat.de/usbprog/versions.xml";

  /* erst datei herunterladen, (wird in arbeitsspeicher im context abgelegt) */
  if(usbprog_online_get_netlist(&usbprog,url)<=0){
    // sag hier bescheid dann kann ich die datei mal kurz verschieben, obs klappt */
    printf("Download Error!"); 
  } else {
    //printf("%s",usbprog.versions_xml);
  }
 
  
  /**** get number of firmware which are in versions.xml *****/
  printf("\n\nget number of online versions\n++++++++++++++++++++++++++++\n");
  i = usbprog_online_numberof_firmwares(&usbprog);
  printf("%i versions are online\n",i);
 
 
  /****  print firmware lables *****/
  printf("\n\nprint firmware labels\n++++++++++++++++++++++++++++\n");
  char *versions[i];
  usbprog_online_print_netlist(&usbprog, versions,i);
  j=0;
  for(i;i>0;i--){
    printf("%i %s\n",j,versions[i-1]);
    j++;
  }


  /**** activate update modus ****/
  
  if(argc > 1){
    printf("\n\nactivate update mode\n++++++++++++++++++++++++++++\n");
    usbprog_update_mode_number(&usbprog,atoi(argv[1]));
    //usbprog_flash_firmware(&usbprog,"openocd.bin");
    usbprog_flash_netfirmware(&usbprog,0);
    usbprog_stop_updatemode(&usbprog);
  }


  
  //if(usbprog.error_str!=NULL);
  //  printf("%s\n",usbprog.error_str);

  return 0;
}
