#include "wait.h"
void wait_ms(int ms)
{
  	int i;
    for(i=0;i<ms;i++)
		_delay_ms(1);
}
