MAINTAINERCLEANFILES  = Makefile.in
INCLUDES                = -I$(top_builddir) -I$(top_srcdir)

bin_PROGRAMS            = usbprog
usbprog_SOURCES  	= io.h io.cc \
			  usbprog.h usbprog.cc \
			  configuration.h \
			  configuration.cc \
			  optionparser.h optionparser.cc \
			  shell.h shell.cc \
			  commands.cc commands.h \
			  main.cc
usbprog_LDADD		= ../usbprog/libusbprog.la
