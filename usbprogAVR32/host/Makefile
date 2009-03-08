CXXFLAGS=-O0 -ggdb

all:	usbprogavr32

usbprogavr32:	cmd.o cmd_flash.o cmd_tap_jtag.o main.o usbprog.o
	$(CXX) -o $@ -lusb $^

clean:
	rm *.o
