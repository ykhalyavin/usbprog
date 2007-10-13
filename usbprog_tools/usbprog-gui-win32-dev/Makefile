#
# File:		makefile.unx
# Author: 	Benedikt Sauter	
# Created: 2006	
# Updated:	
# Copyright:	(c) 2007 Benedikt Sauter 
#
CXX = $(shell wx-config --cxx)

FILES = http_error_codes.o http_fetcher.o usbprogApp.o usbprog.o  usbprogFrm.o xmlParser.o

SVNDEF := -D'SVN_REV="$(shell svnversion -n .)"'

CFLAGS := $(SVNDEF) `wx-config --cxxflags`

# implementation
#
.SUFFIXES:      .o .cpp
#
.cpp.o :
	$(CXX) -c `wx-config --cxxflags` -o $@ $<
#
usbprog-gui: $(FILES)
	$(CXX) -g -o usbprog-gui -lusb -I ../lib2 -L ../lib2 \
	$(FILES) \
	`wx-config --libs`

usbprogFrm.o: usbprogFrm.cpp
	$(CXX) $(CFLAGS) -g -c usbprogFrm.cpp

usbprogApp.o: usbprogApp.cpp
	$(CXX) $(CFLAGS) -g -c usbprogApp.cpp


http_error_codes.o: ../lib2/http_error_codes.c
	$(CXX) $(CFLAGS) -c ../lib2/http_error_codes.c

http_fetcher.o: ../lib2/http_fetcher.c
	$(CXX) $(CFLAGS) -c  ../lib2/http_fetcher.c
	
usbprog.o: ../lib2/usbprog.cpp
	$(CXX) $(CFLAGS) -c ../lib2/usbprog.cpp

xmlParser.o: ../lib2/xmlParser.cpp
	$(CXX) $(CFLAGS) -c ../lib2/xmlParser.cpp

clean: 
	rm -f *.o usbprog-gui

