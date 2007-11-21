MAINTAINERCLEANFILES  = Makefile.in
INCLUDES                = -I$(top_builddir) -I$(top_srcdir)

EXTRA_DIST              = usbprog.xpm usbprog_icon.xpm
bin_PROGRAMS            = usbprog-gui
usbprog_gui_SOURCES  	= usbprogFrm.h usbprogFrm.cc \
			  usbprogApp.h usbprogApp.cc
usbprog_gui_LDADD	= ../usbprog/libusbprog.la \
			  @WX_LIBS@
