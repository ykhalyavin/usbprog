all:
	gcc -c simpleport.c
	ar rcs libsimpleport.a simpleport.o

clean:
	rm *.o


python:
	swig -python simpleport.i
	gcc -c simpleport.c simpleport_wrap.c -I/usr/include/python2.4
	ld -shared simpleport.o simpleport_wrap.o -lusb -o _simpleport.so 

install:
	cp libsimpleport.a /lib/libsimpleport.a




