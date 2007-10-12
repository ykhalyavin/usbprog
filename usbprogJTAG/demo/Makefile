all:
	gcc -g -c main.c 
	gcc -g -o main main.o -lusb -lusbprogjtag 

install:
	cp jtagusb /usr/bin
	
clean:
	rm *.o main
