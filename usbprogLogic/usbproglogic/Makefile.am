bin_PROGRAMS=usbproglogic
usbproglogic_SOURCES=usbproglogic.c xmalloc.c system.h aclocal.m4 usbproglogic.1

#  uncomment the following if usbproglogic requires the math library
#usbproglogic_LDADD=-lm

EXTRA_DIST=usbproglogic.lsm.in usbproglogic.spec.in usbproglogic.texinfo

#  if you write a self-test script named `chk', uncomment the
#  following and add `chk' to the EXTRA_DIST list
#TESTS=chk

#  build and install the .info pages
info_TEXINFOS = usbproglogic.texinfo
usbproglogic_TEXINFOS = gpl.texinfo

#  install the man pages
man_MANS=usbproglogic.1
