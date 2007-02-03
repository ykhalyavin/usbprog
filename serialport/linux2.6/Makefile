obj-m    := usbprog.o 

KDIR    := /lib/modules/$(shell uname -r)/build
KMISC   := /lib/modules/$(shell uname -r)/kernel/drivers/usb/serial
PWD    := $(shell pwd)

EXTRA_CFLAGS += -I/usr/src/linux/drivers/usb/serial -I/usr/src/linux/include/linux/

modules:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) $(EXTRA_CFLAGS) modules

install: modules
	install -d $(KMISC)
	install -m 644 -c usbprog.ko $(KMISC)
	/sbin/depmod -a

clean:
	rm -f *.mod.c *.mod *.o *.ko .*.cmd
	rm -rf $(PWD)/.tmp_versions
	rm Module.symvers
	
