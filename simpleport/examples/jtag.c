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

  //simpleport_set_direction(sp_handle,0xFF);

  int i;
  
  #define TDI 7
  #define TDO 6
  #define TMS 5
  #define TCK 4
  #define TRST 1


  simpleport_set_pin_dir(sp_handle,(1<<TRST),1);
  for(;;){
    simpleport_set_port(sp_handle,(1 << TRST),0xFF);
    simpleport_set_port(sp_handle,0,0xFF);
    simpleport_get_port(sp_handle);
    //simpleport_set_pin(sp_handle,8,1);
    //simpleport_set_pin(sp_handle,8,0);
  }

  simpleport_close(sp_handle);

  return 0;
}
