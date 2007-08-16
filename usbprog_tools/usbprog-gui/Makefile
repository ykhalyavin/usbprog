#
# File:		makefile.unx
# Author: 	Benedikt Sauter	
# Created: 2006	
# Updated:	
# Copyright:	(c) 2006 Benedikt Sauter 
#
# Makefile for penguin example (UNIX).

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
	$(CXX) -o usbprog-gui \
	$(FILES) \
	`wx-config --libs`


http_error_codes.o: http_error_codes.c
	$(CXX) $(CFLAGS) -c http_error_codes.c

http_fetcher.o: http_fetcher.c
	$(CXX) $(CFLAGS) -c  http_fetcher.c
	

usbprogApp.o: usbprogApp.cpp
	$(CXX) $(CFLAGS) -c usbprogApp.cpp

usbprog.o: usbprog.cpp
	$(CXX) $(CFLAGS) -c usbprog.cpp

usbprogFrm.o: usbprogFrm.cpp:
	$(CXX) $(CFLAGS) -c usbprogFrm.cpp

xmlParser.o: xmlParser.cpp
	$(CXX) $(CFLAGS) -c xmlParser.cpp

clean: 
	rm -f *.o usbprog-gui

