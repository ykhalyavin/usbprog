all:
	gcc -g -c main.c -I ../lib/ -pg
	gcc -g -o usbprog main.o -lusb -I ../lib/  -lavrupdate -pg

install:
	cp usbprog /usr/bin
	
clean:
	rm *.o usbprog
