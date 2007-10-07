all:
	mingw32-gcc -c ..\lib\simpleport.c -I c:\Programme\LibUSB-Win32\include
	ar rc libsimpleport.a simpleport.o
	mingw32-gcc -shared -o simpleport.dll ..\lib\simpleport.c -Wl,--out-implib,libsimpleport.a -I c:\Programme\LibUSB-Win32\include -L c:\Programme\LibUSB-Win32\lib\gcc -lusb
