MAINTAINERCLEANFILES    = Makefile.in
INCLUDES                = -I$(top_builddir) -I$(top_srcdir)

EXTRA_DIST              = usbprog.rc usbprog.pod usbprog.1
bin_PROGRAMS            = usbprog
usbprog_SOURCES  	= io.h io.cc \
			  usbprog.h usbprog.cc \
			  configuration.h \
			  configuration.cc \
			  shell.h shell.cc \
			  commands.cc commands.h \
			  main.cc
usbprog_LDADD		= ../usbprog/libusbprog.la
man_MANS		= usbprog.1


usbprog.1: usbprog.pod
	pod2man usbprog.pod             \
		--center="USBprog" 	\
		--release=$(VERSION)	\
		> usbprog.1
