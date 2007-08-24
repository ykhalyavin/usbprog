#
# demo.py
# Simple demo that shows how to
# access usbprog with python,
# using '_simpleport.so'
# wrapped by SWIG.
#
# author: Michael Kaiser
#

import simpleport
import time

if __name__ == "__main__":

    # call simpleport_open() to retrive a handle
    sp_handle = simpleport.simpleport_open()
    simpleport.simpleport_set_pin_dir(sp_handle, 11, 1)

    # periodacally set entire port to '11111111' and '00000000'
    while 1:
	simpleport.simpleport_set_pin(sp_handle, 11, 1)
	time.sleep(1)
	simpleport.simpleport_set_pin(sp_handle, 11, 0)
	time.sleep(1)

    # close handle (never reached in this case)
    simpleport.simpleport_close(sphand)

