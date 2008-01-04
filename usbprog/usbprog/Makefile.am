MAINTAINERCLEANFILES  = Makefile.in
INCLUDES                = -I$(top_builddir) -I$(top_srcdir)

pkginclude_HEADERS    = usbprog.h \
			downloader.h \
			inifile.h \
			util.h \
			date.h \
			firmwarepool.h \
			devices.h \
			digest.h \
			stringutil.h

lib_LTLIBRARIES       = libusbprog.la
libusbprog_la_SOURCES  = inifile.cc \
			 util.cc \
			 downloader.cc \
			 date.cc \
			 firmwarepool.cc \
			 devices.cc \
			 usbprog.cc \
			 digest.cc \
			 stringutil.cc
libusbprog_la_LIBADD   = ../md5/libmd5.a
libusbprog_la_LDFLAGS  = -version-info 1:0:1
