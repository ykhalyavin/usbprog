#include <stdio.h>

#include "simpleport.h"

int main()
{
  struct simpleport * sp_handle;

  printf("libsimpleport Demo\n");
	
  /* open connection to simpleport */
  sp_handle = simpleport_open();
	
  if(sp_handle==0)
    fprintf(stderr,"unable to open device\n");

  simpleport_set_direction(sp_handle,0xFF);

  while(1){
    //simpleport_set_port(sp_handle,0x80);
    //simpleport_set_port(sp_handle,0x00);
    //simpleport_set_bit(sp_handle,6,1);
    //simpleport_set_bit(sp_handle,6,0);
    simpleport_set_port(sp_handle,0xFF,0xFF);
    sleep(1);
    simpleport_set_port(sp_handle,0x00,0xFF);
    sleep(1);
  }

/*
  int i,j;
  for(j=0;j<7;j++) {
    i = simpleport_get_bit(sp_handle,j);
    printf("Pin %i: %i\n",j,i);
  }
*/
  simpleport_close(sp_handle);

  return 0;
}
