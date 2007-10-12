

import sys
import time
import random

def led(timehigh,simpleport,samples):
    timelow = samples - timehigh
    simpleport.simpleport_set_pin(sp_handle, 11, 1)   
    while timehigh > 1:
        simpleport.simpleport_set_pin(sp_handle, 11, 1)   
        timehigh = timehigh-1

    while timelow > 0:
        simpleport.simpleport_set_pin(sp_handle, 11, 0)
        timelow = timelow-1


if __name__ == "__main__":
    sys.path.append('../lib')
    import simpleport
    # call simpleport_open() to retrive a handle
    sp_handle = simpleport.simpleport_open()
    simpleport.simpleport_set_pin_dir(sp_handle, 11, 1)

    # periodacally set entire port to '11111111' and '00000000'
    samples = 5;
    i=0 
    up = 1
    dowm = 0
    while 1:
        if up:
            i = i + 1
            #print i
            for p in range(1,11):
                led(i,simpleport,samples)
            if i == samples:
                down = 1; up = 0;
        else:
            i = i - 1
            #print i
            for p in range(1,11):
                led(i,simpleport,samples)
            if i == 0:
                down = 0; up = 1;


    # close handle (never reached in this case)
    simpleport.simpleport_close(sphand)



