all:	usbprogavr32

usbprogavr32:	cmd.o cmd_flash.o cmd_tap_jtag.o main.o usbprog.o
	g++ -o usbprogavr32 cmd.o cmd_flash.o cmd_tap_jtag.o main.o usbprog.o -lusb

cmd.o:	cmd.cpp cmd.h
	g++ -c cmd.cpp

cmd_flash.o: cmd_flash.cpp cmd_flash.h
	g++ -c cmd_flash.cpp

main.o: main.cpp
	g++ -c main.cpp

usbprog.o: usbprog.cpp usbprog.h
	g++ -c usbprog.cpp

clean:
	rm *.o
