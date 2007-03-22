## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

INCLUDES = \
	$(WX_CXXFLAGS)

AM_CXXFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = usbprog-online

usbprog_online_SOURCES = \
	CsvReader.cpp\
	CsvReader.h\
	GUIFrame.cpp\
	GUIFrame.h\
	OnlineVersions.cpp\
	OnlineVersions.h\
	usbprog.cpp\
	usbprog.h\
	usbprogonlineApp.cpp\
	usbprogonlineApp.h\
	usbprogonlineMain.cpp\
	usbprogonlineMain.h

usbprog_online_LDFLAGS = \
	-lavrupdate

usbprog_online_LDADD =  \
	$(WX_LIBS)

