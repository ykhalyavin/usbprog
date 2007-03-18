AUTOMAKE_OPTIONS=nostdinc subdir-objects
AM_CPPFLAGS=-DDATADIR="\"$(pkgdatadir)\"" -DSYSCONFDIR="\"$(sysconfdir)\""
INCLUDES=-I.
AM_CXXFLAGS=-Wall -Wundef -Wpointer-arith -Wcast-qual -Wcast-align \
	-Wwrite-strings -Werror

noinst_LIBRARIES=libcommon.a
bin_PROGRAMS=odyssey

libcommon_a_SOURCES=ConfigFile.cxx ConfigFile.h \
	Util.cxx Util.h DataBuffer.cxx DataBuffer.h \
	HexFile.cxx HexFile.h HexFile_ihx16.cxx HexFile_ihx8.cxx \
	ParallelPort.cxx ParallelPort.h IO.cxx IO.h \
	Device.cxx Device.h device/PicDevice.h device/PicDevice.cxx \
	device/Pic16Device.cxx device/Pic16f8xxDevice.cxx \
	device/Pic16f81xDevice.cxx \
	device/Pic16f7xDevice.cxx device/Pic18Device.cxx \
	device/Pic12f6xxDevice.cxx device/Pic16f6xx.cxx \
	device/Pic16f87xADevice.cxx device/Pic18f2xx0Device.cxx
EXTRA_libcommon_a_SOURCES=DirectPPIO.cxx DirectPPIO.h \
	LinuxPPDevIO.cxx LinuxPPDevIO.h

libcommon_a_LIBADD=
if ENABLE_LINUX_PPDEV
libcommon_a_LIBADD+=LinuxPPDevIO.o
endif
if ENABLE_DIRECTIO
libcommon_a_LIBADD+=DirectPPIO.o
endif

odyssey_SOURCES=main.cxx
odyssey_LDADD=libcommon.a
