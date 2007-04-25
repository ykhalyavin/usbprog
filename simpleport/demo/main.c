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
    simpleport_set_port(sp_handle,0xFF);
    simpleport_set_port(sp_handle,0x00);
  }


  simpleport_close(sp_handle);

  return 0;
}
