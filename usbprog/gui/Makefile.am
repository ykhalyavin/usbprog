MAINTAINERCLEANFILES  = Makefile.in
INCLUDES                = -I$(top_builddir) -I$(top_srcdir)

EXTRA_DIST              = usbprog.xpm usbprog_icon.xpm \
			  usbprog-gui.pod usbprog-gui.1
bin_PROGRAMS            = usbprog-gui
usbprog_gui_SOURCES  	= usbprogFrm.h usbprogFrm.cc \
			  usbprogApp.h usbprogApp.cc
usbprog_gui_LDADD	= ../usbprog/libusbprog.la \
			  @WX_LIBS@
man_MANS		= usbprog-gui.1

usbprog-gui.1: usbprog-gui.pod 
	$(POD2MAN) usbprog-gui.pod      \
		--center="USBprog GUI" 	\
		--release=$(VERSION)	\
		> usbprog-gui.1
