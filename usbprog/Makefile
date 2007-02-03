all:
	gcc -c lib/at89.c
	gcc -c usbprog.c 
	gcc -o usbprog usbprog.o at89.o -lusb
clean:
	rm usbprog *.o
