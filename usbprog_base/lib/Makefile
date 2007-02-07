all:
	gcc -c avrupdate.c
	gcc -c http_fetcher.c
	gcc -c http_error_codes.c
	ar rcs libavrupdate.a avrupdate.o http_fetcher.o http_error_codes.o

clean:
	rm *.o

install:
	cp libavrupdate.a /lib/libavrupdate.a
	cat usb.usermap >> /etc/hotplug/usb.usermap
	cp hotplug.avrupdate /etc/hotplug/usb/avrupdate
	chmod o+x /etc/hotplug/usb/avrupdate

