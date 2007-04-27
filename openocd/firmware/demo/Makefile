all:
	gcc -g -c main.c -I ../lib/ -pg
	gcc -g -o demo main.o -lusb -I ../lib/  -ljtagusb -pg

install:
	cp jtagusb /usr/bin
	
clean:
	rm *.o jtagusb
