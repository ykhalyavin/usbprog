all:
	gcc -c example.c -I ../lib/ 
	gcc -c jtag.c -I ../lib/ 
	gcc -o example example.o -lusb -I ../lib -lsimpleport 
	gcc -o jtag jtag.o -lusb -I ../lib -lsimpleport 

clean:
	rm -f *.o *.pyc
