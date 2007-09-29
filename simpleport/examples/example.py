
import sys
import time

if __name__ == "__main__":
    sys.path.append('../lib')
    import simpleport
    
    sp_handle = simpleport.simpleport_open()
    simpleport.simpleport_set_pin_dir(sp_handle, 11, 1)

    while 1:
	simpleport.simpleport_set_pin(sp_handle, 11, 1)
	time.sleep(1)
	simpleport.simpleport_set_pin(sp_handle, 11, 0)
	time.sleep(1)

    # close handle (never reached in this case)
    simpleport.simpleport_close(sphand)
