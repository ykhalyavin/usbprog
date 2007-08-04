all: libsimpleport simpleportjava-swig simpleportpython-swig simpleportjava simpleportpython example examplejava examplepython

libsimpleport:

simpleportjava-swig:

simpleportjava:

simpleportpython-swig:

simpleportpython:

example:

examplejava:

examplepython


all:
	gcc -c main.c -I ../lib/ -pg
	gcc -o demo main.o -lusb -I ../lib/  -lsimpleport -pg

	
clean:
	rm *.o simpleport demo
