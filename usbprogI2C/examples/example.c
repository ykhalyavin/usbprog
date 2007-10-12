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

  int i;
  for(i=0;i<4;i++){
    simpleport_set_port(sp_handle,0x80,0xFF);
    sleep(1);
    simpleport_set_port(sp_handle,0x00,0xFF);
    sleep(1);
  }

  simpleport_set_pin_dir(sp_handle,11,1);
  for(i=0;i<4;i++){
    simpleport_set_pin(sp_handle,11,1);
    sleep(1);
    simpleport_set_pin(sp_handle,11,0);
    sleep(1);
  }

  int j;
  for(j=0;j<3;j++) {
    for(i=0;i<200;i++){
      simpleport_set_pin(sp_handle,11,1);
      simpleport_set_pin(sp_handle,11,0);
      simpleport_set_pin(sp_handle,11,0);
      simpleport_set_pin(sp_handle,11,0);
      simpleport_set_pin(sp_handle,11,0);
    }
  
    for(i=0;i<200;i++){
      simpleport_set_pin(sp_handle,11,1);
      simpleport_set_pin(sp_handle,11,1);
      simpleport_set_pin(sp_handle,11,1);
      simpleport_set_pin(sp_handle,11,1);
      simpleport_set_pin(sp_handle,11,0);
    }
  }

  simpleport_close(sp_handle);

  return 0;
}
