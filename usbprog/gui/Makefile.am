MAINTAINERCLEANFILES  = Makefile.in
INCLUDES                = -I$(top_builddir) -I$(top_srcdir)

EXTRA_DIST              = usbprog.xpm usbprog_icon.xpm \
			  pins.xpm pins.svg \
			  usbprog-gui.pod usbprog-gui.1
if GUI
bin_PROGRAMS            = usbprog-gui
usbprog_gui_SOURCES  	= usbprogFrm.h usbprogFrm.cc \
			  usbprogApp.h usbprogApp.cc \
			  infodialog.cc infodialog.h \
			  pindialog.cc pindialog.h
usbprog_gui_LDADD	= ../usbprog/libusbprog.la \
			  @WX_LIBS@
man_MANS		= usbprog-gui.1
endif

icondir = $(datadir)/pixmaps
icon_DATA = usbprog_icon.xpm

usbprog-gui.1: usbprog-gui.pod 
	pod2man usbprog-gui.pod         \
		--center="USBprog GUI" 	\
		--release=$(VERSION)	\
		> usbprog-gui.1
