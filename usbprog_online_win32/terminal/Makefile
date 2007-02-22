all:
	gcc -g -c ../lib/avrupdate.c
	gcc -c main.c -lusb
	gcc -g -o avrupdate main.o -lusb -lavrupdate

install:
	cp avrupdate /usr/bin
	
clean:
	rm *.o avrupdate
