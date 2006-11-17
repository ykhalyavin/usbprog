all:
	gcc -c ../lib/avrupdate.c
	gcc -c main.c
	gcc -o avrupdate avrupdate.o main.o -lusb -I ../lib/

install:
	cp vlabc /usr/bin
	chmod o+rx /usr/bin/vlabc
	cat usb.usermap >> /etc/hotplug/usb.usermap
	cp hotplug.vlab /etc/hotplug/usb/vlab
	chmod o+x /etc/hotplug/usb/vlab
	
clean:
	rm *.o avrupdate
