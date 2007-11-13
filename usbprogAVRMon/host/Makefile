CC = gcc
RM = rm

#DEBUG = -g -DDEBUG
CFLAGS = $(DEBUG) -O

mon: mon.c mon.h
	$(CC) $(CFLAGS) mon.c -o mon -lreadline

clean:
	$(RM) mon
