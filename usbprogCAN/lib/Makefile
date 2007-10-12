all:
	gcc -c simpleport.c
	ar rcs libsimpleport.a simpleport.o

clean:
	rm *.o *.pyc


python:
	swig -python simpleport.i
	gcc -c simpleport.c simpleport_wrap.c -I/usr/include/python2.4
	#ld -shared simpleport.o simpleport_wrap.o /usr/lib/libusb.so -o _simpleport.so 
	gcc -fPIC -shared simpleport_wrap.o simpleport.o /usr/lib/libusb.so -o _simpleport.so



java:
	swig -java simpleport.i
	gcc -fpic -c simpleport.c simpleport_wrap.c -I/usr/lib/jvm/java-1.5.0-sun-1.5.0.10/include \
	-I/usr/lib/jvm/java-1.5.0-sun-1.5.0.10/include/linux/
	gcc -shared simpleport.o  simpleport_wrap.o  -lusb -o libsimpleport.so

install:
	cp libsimpleport.a /lib/libsimpleport.a




