all:
	gcc -g -c ../lib/avrupdate.c
	gcc -c main.c -I ../lib/
	gcc -g -o avrupdate main.o -lusb -I ../lib/  -lavrupdate

install:
	cp avrupdate /usr/bin
	
clean:
	rm *.o avrupdate
