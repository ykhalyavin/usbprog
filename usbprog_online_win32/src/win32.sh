

export PATH=$PATH:/c/mingw/bin
export PKG_CONFIG_PATH=/c/mingw/lib/pkgconfig

gcc \
-g -o usbprog-online \
`pkg-config --cflags gtk+-win32-2.0` \
-mno-cygwin -mms-bitfields \
-DPACKAGE_DATA_DIR="\"/mingw/share\"" \
-DPACKAGE="\"usbprog-online\"" \
*.c -lusb -lwsock32 -lws2_32 \
`pkg-config --libs gtk+-win32-2.0`
