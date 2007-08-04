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

CC=gcc-4.1

all:


wrap-python:
	swig -python simpleport.i
	$(CC) -fPIC -c simpleport.c
	$(CC) -fPIC -c simpleport_wrap.c -I /usr/include/python2.4
	$(CC) -fPIC -shared simpleport_wrap.o simpleport.o /usr/lib/libusb.so -o _simpleport.so
	mv _simpleport.so ./python
	mv simpleport.py ./python

wrap-python-cygwin:
	swig -python simpleport.i
	$(CC) -c simpleport.c
	$(CC) -c simpleport_wrap.c -I /usr/include/python2.5 -I /include
	$(CC) -shared simpleport_wrap.o simpleport.o /usr/lib/libusb.dll.a -L/usr/lib/python2.5/config -lpython2.5 -o _simpleport.dll

wrap-java:
	swig -java simpleport.i
	$(CC) -fPIC -c simpleport.c
	$(CC) -fPIC -c simpleport_wrap.c -I /usr/lib/jvm/java-1.5.0-sun-1.5.0.10/include/ -I /usr/lib/jvm/java-1.5.0-sun-1.5.0.10/include/linux/
	$(CC) -fPIC -shared simpleport_wrap.o simpleport.o /usr/lib/libusb.so -o _simpleport.so
	mv _simpleport.so ./java
	mv simpleport.java ./java
	mv simpleportJNI.java ./java
	mv SWIGTYPE_p_simpleport.java ./java

wrap-java-cygwin:
	swig -java simpleport.i
	$(CC) -c simpleport.c
	$(CC) -c simpleport_wrap.c -I /usr/include/python2.5 -I /include
	$(CC) -shared simpleport_wrap.o simpleport.o /usr/lib/libusb.dll.a -L/usr/lib/python2.5/config -lpython2.5 -o _simpleport.dll

clean:
	rm -f *.o
	rm -f *.so
	rm -f *_wrap*
