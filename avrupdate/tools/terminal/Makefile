all:
	gcc -g -c ../../lib/avrupdate.c
	gcc -g -c ../../lib/http_error_codes.c
	gcc -g -c ../../lib/http_fetcher.c
	gcc -c main.c -I ../../lib/
	gcc -g -o avrupdate avrupdate.o main.o http_error_codes.o http_fetcher.o -lusb -I ../../lib/ -lcurl

install:
	cp vlabc /usr/bin
	chmod o+rx /usr/bin/vlabc
	cat usb.usermap >> /etc/hotplug/usb.usermap
	cp hotplug.vlab /etc/hotplug/usb/vlab
	chmod o+x /etc/hotplug/usb/vlab
	
clean:
	rm *.o avrupdate
