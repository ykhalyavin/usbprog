#define LED_INIT()      led_timer=0; PTD=PTD_PTD0_MASK+PTD_PTD1_MASK; DDRD=0xff;   /* LED off, LED pin is output (the rest PTD pins are made outputs and brought low here so they do not float) */
#define LED_SW_ON()     PTD&=~(PTD_PTD0_MASK+PTD_PTD1_MASK)
#define LED_SW_OFF()    PTD|=(PTD_PTD0_MASK+PTD_PTD1_MASK)
#define LED_SW_TOGGLE() PTD^=(PTD_PTD0_MASK+PTD_PTD1_MASK)

#define LED_BLINK_PERIOD  14   /* duration of LED period */
#define LED_OFF_TIME      7    /* how long should the LED be off during the blink period */

typedef enum {
  LED_ON,
  LED_OFF,
  LED_BLINK
} led_state_e;

#pragma DATA_SEG Z_RAM
extern led_state_e near led_state;
#pragma DATA_SEG DEFAULT
