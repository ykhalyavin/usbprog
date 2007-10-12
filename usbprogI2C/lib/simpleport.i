%module simpleport

%{
#include "simpleport.h"
#include <usb.h>
#define VID 0x1781
#define PID 0x0c62

#define UNKOWN_COMMAND  0x00
#define PORT_DIRECTION  0x01
#define PORT_SET        0x02
#define PORT_GET        0x03
#define PORT_SETBIT     0x04
#define PORT_GETBIT     0x05

extern struct simpleport* simpleport_open();
extern void simpleport_close(struct simpleport *);
extern unsigned char simpleport_message(struct simpleport *, char *, int );
extern void simpleport_set_direction(struct simpleport *, unsigned char );
extern void simpleport_set_port(struct simpleport *,unsigned char , unsigned char );
extern unsigned char simpleport_get_port(struct simpleport *);
extern void simpleport_set_pin_dir(struct simpleport *,int , int );
extern void simpleport_set_pin(struct simpleport *,int , int );
extern int simpleport_get_pin(struct simpleport *, int );
%}

extern struct simpleport* simpleport_open();
extern void simpleport_close(struct simpleport *);
extern unsigned char simpleport_message(struct simpleport *, char *, int );
extern void simpleport_set_direction(struct simpleport *, unsigned char );
extern void simpleport_set_port(struct simpleport *,unsigned char , unsigned char );
extern unsigned char simpleport_get_port(struct simpleport *);
extern void simpleport_set_pin_dir(struct simpleport *,int , int );
extern void simpleport_set_pin(struct simpleport *,int , int );
extern int simpleport_get_pin(struct simpleport *, int );

