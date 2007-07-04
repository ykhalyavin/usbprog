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
	OnlineVersions.cpp\
	OnlineVersions.h\
	usbprogonlineApp.cpp\
	usbprogonlineApp.h\
	MainFrameGenerated.cpp\
	MainFrameGenerated.h\
	MainFrame.cpp\
	MainFrame.h\
	USBDevices.cpp\
	USBDevices.h

usbprog_online_LDFLAGS = \
	-lavrupdate

usbprog_online_LDADD =  \
	$(WX_LIBS)

