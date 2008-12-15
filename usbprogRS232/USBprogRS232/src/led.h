#ifndef LED_H_
#define LED_H_

#define LED_init()		DDRA |= _BV(DDA4)
#define LED_on()		PORTA |= _BV(PA4)
#define LED_off()		PORTA &= ~_BV(PA4)
#define LED_toggle()	PORTA ^= _BV(PA4)

#endif /*LED_H_*/
