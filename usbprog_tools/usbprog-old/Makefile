CC 	= gcc
CFLAGS 	= -W -Wall -Wno-unused-parameter
PREFIX	= /usr
BINDIR	= $(PREFIX)/bin

all:
	$(CC) $(CFLAGS) -c main.c -I ../lib/ -pg
	$(CC) -o usbprog main.o -lusb -L../lib  -lavrupdate -pg

install:
	install -m 0755 usbprog $(BINDIR)
	
clean:
	rm -f *.o usbprog
