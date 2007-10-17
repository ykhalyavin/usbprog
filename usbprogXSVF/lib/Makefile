CC = gcc
AR = ar
APPNAME = xsvfplayer
OBJECTS = $(APPNAME).o
LIBS = -lxsvfprog -L. -lusb
CFLAGS = -DSIM

$(APPNAME): $(OBJECTS) libxsvfprog.a
	$(CC) $(CFLAGS) $(OBJECTS) -o $(APPNAME) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

libxsvfprog.a: xsvfprog.o
	$(AR) rsv $@ $?

clean:
	rm -f $(OBJECTS) $(APPNAME) libxsvfprog.a xsvfprog.o

all: $(APPNAME)