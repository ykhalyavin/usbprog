all:
	gcc -g -c main.c -I ../lib/ -pg
	gcc -g -o demo main.o -lusb -I ../lib/  -lsimpleport -pg

install:
	cp simpleport /usr/bin
	
clean:
	rm *.o simpleport
