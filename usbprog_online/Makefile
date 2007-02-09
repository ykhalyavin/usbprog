#
# File:		makefile.unx
# Author: 	Benedikt Sauter	
# Created: 2006	
# Updated:	
# Copyright:	(c) 2006 Benedikt Sauter 
#
# Makefile for penguin example (UNIX).

CXX = $(shell wx-config --cxx)

FILES = main.o window.o


SVNDEF := -D'SVN_REV="$(shell svnversion -n .)"'

CFLAGS := $(SVNDEF) `wx-config --cxxflags`

# implementation
#
.SUFFIXES:      .o .cpp
#
.cpp.o :
	$(CXX) -c `wx-config --cxxflags` -o $@ $<
#
vScope: $(FILES)
	$(CXX) -o usbprog-online \
	$(FILES) \
	`wx-config --libs`

main.o: main.cpp
	$(CXX) $(CFLAGS) -c main.cpp

window.o: window.cpp
	$(CXX) $(CFLAGS) -c window.cpp



clean: 
	rm -f *.o usbprog-online 

