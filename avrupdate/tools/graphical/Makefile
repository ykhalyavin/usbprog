#
# File:		makefile.unx
# Author: 	Benedikt Sauter	
# Created: 2006	
# Updated:	
# Copyright:	(c) 2006 Benedikt Sauter 
#
# Makefile for penguin example (UNIX).

CXX = $(shell wx-config --cxx)

FILES = main.o


SVNDEF := -D'SVN_REV="$(shell svnversion -n .)"'

CFLAGS := $(SVNDEF) `wx-config --cxxflags ` 

# implementation
#
.SUFFIXES:      .o .cpp
#
.cpp.o :
	$(CXX) -c `wx-config --cxxflags` `wx-config --libs`-o $@ $<
#
avrupdatx: $(FILES)
	$(CXX) -o avrupdate-gui \
	$(FILES) \
	`wx-config --libs`

main.o: main.cpp
	$(CXX) $(CFLAGS) -c main.cpp

clean: 
	rm -f *.o avrupdate-gui

