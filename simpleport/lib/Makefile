all:
	gcc -c simpleport.c
	ar rcs libsimpleport.a simpleport.o

clean:
	rm *.o

install:
	cp libsimpleport.a /lib/libsimpleport.a


all:
        gcc -c main.c -I ../lib/ -pg
	gcc -o demo main.o -lusb -I -lsimpleport -pg

install:
	cp simpleport /usr/bin

clean:
	rm *.o simpleport demo


