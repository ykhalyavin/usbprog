#include <stdio.h>

#include <usbprogjtag.h>

int main()
{
  struct usbprog_jtag * usbprogjtag_handle;
  usbprogjtag_handle = usbprog_jtag_open();

  usbprog_jtag_init(usbprogjtag_handle);

  while(1){
    char seq[] = {0,1,0,1,0};
    usbprog_jtag_tap_shift_register(usbprogjtag_handle,seq,NULL,5);
    sleep(1);
  }


  usbprog_jtag_close(usbprogjtag_handle);

  return 0;
}
