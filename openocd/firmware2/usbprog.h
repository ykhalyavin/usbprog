#include <stdint.h>

#define LED_PIN     PA4
#define LED_PORT    PORTA

#define LED_on     (LED_PORT   |=  (1 << LED_PIN))   // red led
#define LED_off    (LED_PORT   &= ~(1 << LED_PIN))


volatile struct usbprog_t
{
  int long_index;
  int long_bytes;
  int long_running;
  int datatogl;
} usbprog;

