#include <stdio.h>

#include <usbprogjtag.h>

int main()
{
  struct usbprogjtag * usbprogjtag_handle;
  usbprogjtag_handle = usbprog_jtag_open();

  usbprog_jtag_init(usbprogjtag_handle);

  char seq[] = {0,1,0,1,0};
  while(1){
  usbprog_jtag_tap_shift_register(usbprogjtag_handle,seq,NULL,5);
  sleep(1);
  }


  usbprog_jtag_close(usbprogjtag_handle);

  return 0;
}
