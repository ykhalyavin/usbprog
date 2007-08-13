#include <stdio.h>    // to get "printf" function

#include "usbprog.h"

int main(int argc, char **argv)
{
  usbprog_init();
  usbprog_print_devices();

  return 0;
}
