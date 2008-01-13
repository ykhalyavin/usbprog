auxdir                = $(top_srcdir)/$(ac_aux_dir)
AUX_DIST              = $(auxdir)/install-sh $(auxdir)/missing \
                        $(auxdir)/mkinstalldirs
AUX_DIST_EXTRA        = $(auxdir)/readline.m4 $(auxdir)/sys_errlist.m4 \
                        $(auxdir)/sys_siglist.m4
EXTRA_DIST            = bootstrap win32/libusbprog.dev win32/usbprog.dev \
			win32/usbprog-gui.dev win32/libmd5.dev \
			win32/README RELEASEPLAN \
			src/usbprog.pod gui/usbprog-gui.pod

AUTOMAKE_OPTIONS      = foreign
SUBDIRS               = md5 usbprog src gui
MAINTAINERCLEANFILES  = Makefile.in aclocal.m4 configure config.h.in \
			config/depcomp config/missing config/config.guess \
			config/config.sub config/install-sh \
                        $(AUX_DIST)

ACLOCAL               = aclocal -I $(auxdir)


