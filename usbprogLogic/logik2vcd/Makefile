all:
	gcc -Wall -g -c logic.c
	gcc -Wall -g -o logic2vcd main.c logic.o -lusb

install:
	cp logicc /usr/bin
	chmod o+rx /usr/bin/logicc
	cat usb.usermap >> /etc/hotplug/usb.usermap
	cp hotplug.logic /etc/hotplug/usb/logic
	chmod o+x /etc/hotplug/usb/logic
	
clean:
	rm *.o
