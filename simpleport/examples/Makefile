all:
	gcc -c main.c -I ../lib/ -pg
	gcc -o demo main.o -lusb -I ../lib/  -lsimpleport -pg

install:
	cp simpleport /usr/bin
	
clean:
	rm *.o simpleport demo
