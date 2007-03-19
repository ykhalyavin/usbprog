## Process this file with automake to produce Makefile.in

## Created by Anjuta

INCLUDES = \
	$(WX_CXXFLAGS) \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(PACKAGE_CFLAGS)

AM_CFLAGS =\
	 -Wall \
	 -g \
	 $(WX_CPPFLAGS)

bin_PROGRAMS = usbprog-online

usbprog_online_SOURCES = \
	GUIFrame.cpp    \
	usbprogonlineApp.cpp    \
	usbprogonlineMain.cpp    \
	GUIFrame.h    \
	usbprogonlineApp.h    \
	usbprogonlineMain.h    \
	CsvReader.cpp    \
	CsvReader.h    \
	OnlineVersions.cpp    \
	OnlineVersions.h    \
	usbprog.cpp    \
	usbprog.h    

usbprog_online_LDADD = \
	$(PACKAGE_LIBS) \
	$(WX_LIBS) \
	-lavrupdate 

