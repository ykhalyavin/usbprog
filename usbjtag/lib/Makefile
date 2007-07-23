all:
	gcc -c usbprogjtag.c
	ar rcs libusbprogjtag.a usbprogjtag.o

clean:
	rm *.o *.a

install:
	cp libusbprogjtag.a /lib/libusbprogjtag.a
	cp usbprogjtag.h /usr/include/

