all:
	gcc -g -c main.c -I ../lib/
	gcc -g -o usbprog main.o -lusb -I ../lib/  -lavrupdate

install:
	cp usbprog /usr/bin
	
clean:
	rm *.o usbprog
