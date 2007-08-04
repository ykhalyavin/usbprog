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

	# periodacally set entire port to '11111111' and '00000000'
	while 1:
		simpleport.simpleport_set_port(sp_handle, 0xFF, 0xFF)
		time.sleep(1)
		simpleport.simpleport_set_port(sp_handle, 0x00, 0xFF)
		time.sleep(1)

	# close handle (never reached in this case)
	simpleport.simpleport_close(sphand)

