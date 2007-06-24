bin_PROGRAMS=usbprog
usbprog_SOURCES=usbprog.c usbprog.1 xmalloc.c system.h aclocal.m4 getopt.c getopt1.c getopt.h

#  uncomment the following if usbprog requires the math library
#usbprog_LDADD=-lm

EXTRA_DIST=usbprog.lsm.in usbprog.spec.in usbprog.texinfo

#  if you write a self-test script named `chk', uncomment the
#  following and add `chk' to the EXTRA_DIST list
#TESTS=chk

#  build and install the .info pages
info_TEXINFOS = usbprog.texinfo
usbprog_TEXINFOS = gpl.texinfo

#  install the man pages
man_MANS=usbprog.1
