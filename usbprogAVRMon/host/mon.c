/*  
  Copyright (C) 1999 by Denis Chertykov (denisc@overta.ru)
  STK200 communication added by Tor Ringstad (torhr@pvv.ntnu.no).

  You can redistribute it and/or modify it under the terms of the GNU
  General Public License as published by the Free Software Foundation;
  either version 2, or (at your option) any later version.

  This program works with the connection scheme used in Atmels "STK200
  Starter Kit". The connection allows to program the AVR with Uros
  Platise `uisp' in `-dstk200' mode.

  Bugs: For me this work only if Parellel port in EPP mode.
        May be ECP mode also work? 
*/

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <fcntl.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "mon.h"

int mon_printf (const char * fmp, ...);
int mon_read (unsigned char *data, unsigned short addr, const char * err);

int debug_p;

#define MON_USAGE "\
Commands:\n\
r ADDRESS [COUNT] - read COUNT bytes from data memory\n\
w ADDRESS BYTE    - write BYTE to data memory\n\
p ADDRESS [COUNT] - read COUNT bytes from program memory\n\
P ADDRESS [COUNT] - read COUNT commands from program memory\n\
i ADDRESS         - read byte from IO space\n\
o ADDRESS BYTE    - write BYTE to IO space\n\
R                 - show registers\n\
X=NN              - set register X to NN\n\
n                 - execute instruction by PC\n\
bp ADDRESS        - set breakpoint.\n\
bc ADDRESS        - clear breakpoint. -1 as ADDRESS - clear all breakpoints.\n\
q                 - quit monitor (running or stepping AVR program)\n"

/* STK200 Direct Parallel Access */
#define STK2_ENA1   0x04        /* D2 (base) - enable for RESET, MISO */
#define STK2_ENA2   0x08        /* D3 (base) - enable for SCK, MOSI */
#define STK2_SCK    0x10        /* D4 (base) */
#define STK2_DOUT   0x20        /* D5 (base) */
#define STK2_RESET  0x80        /* D7 (base) */
#define STK2_DIN    0x40        /* ACK (base + 1) */

/* 
 * bit defines for 8255 status port
 * base + 1
 * accessed with LP_S(minor), which gets the byte...
 */
#define LP_PBUSY	0x80  /* inverted input, active high */
#define LP_PACK		0x40  /* unchanged input, active low */
#define LP_POUTPA	0x20  /* unchanged input, active high */
#define LP_PSELECD	0x10  /* unchanged input, active high */
#define LP_PERRORP	0x08  /* unchanged input, active low */

/* 
 * defines for 8255 control port
 * base + 2 
 * accessed with LP_C(minor)
 */
#define LP_PINTEN	0x10  /* high to read data in or-ed with data out */
#define LP_PSELECP	0x08  /* inverted output, active low */
#define LP_PINITP	0x04  /* unchanged output, active low */
#define LP_PAUTOLF	0x02  /* inverted output, active low */
#define LP_PSTROBE	0x01  /* short high output on raising edge */

#define _LP_BASE 0x378

#define RD_PORT (inb (_LP_BASE+1))
#define SET_CLK SetClk();
#define CLR_CLK ClrClk();
//#define RESET_AVR ResetAVR();
#define OUT_DATA_BIT(bit) OutDataBit(bit);


/* address of registers array inside chip
 r[31], sreg, sp, pc */
unsigned int regs_array;

struct sreg_bits_s sreg_bits[8] =
{
    {'C',1},
    {'Z',2},
    {'N',4},
    {'V',8},
    {'S',0x10},
    {'H',0x20},
    {'T',0x40},
    {'I',0x80}
};

/*  #define DEBUG */

#ifdef DEBUG
#  define Dprintf(x) printf (x)
#else
#  define Dprintf(x)
#endif

int tty_p;
FILE *status_f; /* torhr */


int mon_printf(const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  if (debug_p && !tty_p)
    vfprintf (stderr, fmt, args);
  return vprintf (fmt, args);
}


/*------------------------------------------------------------------*/
/* Low level printer port manipulation                              */
/*------------------------------------------------------------------*/

/* Stores the last value written to _LP_BASE. Makes it possible for
   different parts of the program to change some bits without altering
   the rest. */

int port_val;

/* Enable this to get a debug output for every transition on any of
   the communication lines. */

void PrintPort(void)
{
#if 0
  fprintf(stderr, "ENA1: %s  ", (port_val & STK2_ENA1) ? "*" : " ");
  fprintf(stderr, "ENA2: %s  ", (port_val & STK2_ENA2) ? "*" : " ");
  fprintf(stderr, "RST: %s  ", (port_val & STK2_RESET) ? "*" : " ");
  fprintf(stderr, "SCK: %s  ", (port_val & STK2_SCK) ? "*" : " ");
  fprintf(stderr, "DOUT: %s  ", (port_val & STK2_DOUT) ? "*" : " ");
  fprintf(stderr, "\n");
#endif
}

/* Resets the AVR and bring the target connection to a known initial
   state where the ISP is activated and RESET is inactive. Also
   initializes port_val. */

#if 0
void ResetAVR(void)
{
  Dprintf("Reset AVR...");

  outb(STK2_RESET | STK2_ENA1 | STK2_ENA2, _LP_BASE);
  usleep(1000);
  outb(STK2_RESET, _LP_BASE);
  usleep(1000);
  outb(0, _LP_BASE);
  usleep(1000);
  outb(STK2_RESET, _LP_BASE);
  port_val = STK2_RESET;

  Dprintf("ok\n");
  PrintPort();
}

void SetClk(void)
{
/*   Dprintf("-> CLK HI\n"); */
/*   getc(stdin); */
  port_val = port_val | STK2_SCK;
  outb(port_val, _LP_BASE);
  PrintPort();
}

void ClrClk(void)
{
/*   Dprintf("-> CLK LOW\n"); */
/*   getc(stdin); */
  port_val = port_val & (~STK2_SCK);
  outb(port_val, _LP_BASE);
  PrintPort();
}

void OutDataBit(unsigned char bit)
{
  int b = bit;
/*   Dprintf("-> OUT_DATA_BIT\n"); */
/*   getc(stdin); */
  if (b) {
    port_val = port_val | STK2_DOUT;
  } else {
    port_val = port_val & (~STK2_DOUT);
  }
  outb(port_val, _LP_BASE);
  PrintPort();
}
#endif
/*------------------------------------------------------------------*/
/* Time related subroutines                                         */
/*------------------------------------------------------------------*/

#include <signal.h>
#include <sys/time.h>

unsigned long n_per_100ms;
static volatile int yes_alarm;

static void my_alarm( int k ){
  yes_alarm = 0;
}

void DelayAtom(){
  volatile int i;
  for (i=0;i<1000;i++);
}

void Delay_ms(unsigned long ms) {
  unsigned long n;
  n = (ms * n_per_100ms) / 100;
  while (n) { DelayAtom(); n--; }
}

void Delay_10us(unsigned long t) {
  unsigned long n;
  n = (t * n_per_100ms) / 10000;
  while (n) { DelayAtom(); n--; }
}

unsigned long CalcDelay()
{ /* pause x ms */
  long n = 0;
  struct itimerval it;
  it.it_interval.tv_sec = 0;
  it.it_value.tv_sec = 0;
  it.it_interval.tv_usec = 0;
  it.it_value.tv_usec = 100000;  /* 100 ms */
  signal(SIGALRM, my_alarm);
  setitimer(ITIMER_REAL, &it, NULL);
  yes_alarm = 1;
  while (yes_alarm){
    DelayAtom(); n++;
  }
  return n;
}

/*------------------------------------------------------------------*/

int make_socket (struct sockaddr_in * name, unsigned short int port)
{
  int sock;
  int tmp;
  struct protoent *protoent;
     
  /* Create the socket.  */
  sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    {
      perror ("socket");
      exit (EXIT_FAILURE);
    }
  /* Allow rapid reuse of this port. */
  tmp = 1;
  setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (char *)&tmp, sizeof(tmp));
  
  /* Enable TCP keep alive process. */
  tmp = 1;
  setsockopt (sock, SOL_SOCKET, SO_KEEPALIVE, (char *)&tmp, sizeof(tmp));
     
  if (bind (sock, (struct sockaddr *) name, sizeof (*name)) < 0)
    {
      perror ("bind");
      exit (EXIT_FAILURE);
    }

  protoent = getprotobyname ("tcp");
  if (!protoent)
    {
      perror ("getprotobyname (\"tcp\")");
      exit (EXIT_FAILURE);
    }
  
  tmp = 1;
  if (setsockopt (sock, protoent->p_proto, TCP_NODELAY,
		  (char *)&tmp, sizeof(tmp)))
    {
      perror ("setsockopt");
      exit (EXIT_FAILURE);
    }
  return sock;
}
/*
   Here is another example, showing how you can fill in a `sockaddr_in'
   structure, given a host name string and a port number:
   */

void init_sockaddr (struct sockaddr_in *name,
     	       const char *hostname, unsigned short int port)
{
  struct hostent *hostinfo;
     
  name->sin_family = AF_INET;
  name->sin_port = htons (port);
  hostinfo = gethostbyname (hostname);
  if (hostinfo == NULL)
    {
      fprintf (stderr, "Unknown host %s.\n", hostname);
      exit (EXIT_FAILURE);
    }
  name->sin_addr = *(struct in_addr *) hostinfo->h_addr;
}


/*------------------------------------------------------------------*/
/* STK200 communication routines                                    */
/*------------------------------------------------------------------*/

#define CLK_DELAY 3    /* Appx Tx/Rx clock period/2 */
#define TX_SYNC 0xbd   /* 10111101 */
#define RX_SYNC 0xb9   /* 10111001 */

#if 0
int RxBit(void) {
  int bit;
  SetClk();
  Delay_10us(CLK_DELAY);
  bit = !!(RD_PORT & STK2_DIN);
/*    printf("Got %d\n", bit); */
  ClrClk();
  Delay_10us(CLK_DELAY);
  return bit;
}

void TxBit(int b) {
  if (b) { OUT_DATA_BIT(1); /*  printf("Sent 1\n"); */ }
  else { OUT_DATA_BIT(0); /*  printf("Sent 0\n"); */ }
  Delay_10us(CLK_DELAY);
  SetClk();
  Delay_10us(CLK_DELAY);
  ClrClk();
  OUT_DATA_BIT(0)
}

int RxByte(void) {
  int i;
  int b = 0;
  for(i = 0; i < 8; i++) {
    b = (b << 1) & 0xff;
    b |= RxBit();
  }
  return b;
}

void TxByte(int b) {
  int i;
  for(i = 0; i < 8; i++) {
    TxBit(b & 0x80);
    b <<= 1;
  }
}

int HuntTxSync(void) {
  int word = 0;
  while(1) {
    word = (word << 1) & 0xff;
    word |= RxBit();
    if (word == TX_SYNC) return(1);
    if (word == RX_SYNC) return(-1);
  }
}

int HuntRxSync(void) {
  int word = 0;
  while(1) {
    word = (word << 1) & 0xff;
    word |= RxBit();
    if (word == RX_SYNC) return(1);
    if (word == TX_SYNC) return(-1);
  }
}

#endif


int RecvByte(void) {
  int b;
#if 0
  if (HuntRxSync() < 0) return -1;
  b = RxByte();
/*    printf("RecvByte() = %02x\n", b); */
#endif
  printf("RecvByte() = %02x\n", (unsigned char)b);
  return b;
}

int SendByte(int b) {
#if 0
  if (HuntTxSync() < 0) return -1;
  TxByte(b);
#endif
  printf("SendByte(%02x)\n", b); 
  return 1;
}

/* ----------------------------------------------------------------- */

static struct packet_s packet, back_packet;
int
packet_action ()
{
  /* Send packet */
  SendByte (packet.cmd);
  SendByte (packet.data);
  SendByte (packet.u.raw.lo);
  SendByte (packet.u.raw.hi);

  /* Receive back packet */
  back_packet.cmd = RecvByte ();
  back_packet.data = RecvByte ();
  back_packet.u.raw.lo = RecvByte ();
  back_packet.u.raw.hi = RecvByte ();
/*    printf(" back_packet.cmd = '%c'\n", back_packet.cmd); */
  return back_packet.cmd == 'E';
}

void
dump_avr_registers (void)
{
  unsigned short sp;
  unsigned char reg[32];
  unsigned char sreg;
  unsigned short pc=0;
  int i,j;

  for (i = 0; i < 37; ++i)
    {
      unsigned char data;
      packet.cmd = 'r';
      packet.u.addr = regs_array + i;
      if (mon_read (&data, regs_array + i, "Read registers"))
	return;
      if (i <= 31)
	reg[i] = data;
      else if (i == 32)
	sreg = data;
      else if (i == 33)
	sp = data;
      else if (i == 34)
	sp |= data << 8;
      else if (i == 35)
	pc |= data;
      else if (i == 36)
	pc |= data << 8;
    }
  j = 1;
  for (i = 0; i < 32; ++i)
    {
      char s[30];
      sprintf (s, "r%-2d=%02x", i, reg[i]);
      if ((j++ % 8) == 0)
	mon_printf ("%s\n", s);
      else
	mon_printf ("%-10s", s);
    }

  mon_printf ("X=%04x  Y=%04x  Z=%04x\n",
	  reg[26] | reg[27] << 8,
	  reg[28] | reg[29] << 8,
	  reg[30] | reg[31] << 8);
	  
  mon_printf ("SREG=%02x   ", sreg);
  for (i=0; i < 8; ++i)
    mon_printf (" %c=%d", sreg_bits[i].name, !! (sreg & sreg_bits[i].mask));
  mon_printf ("\nPC=%05x  SP=%04x\n", pc*2, sp);
}

int
mon_write (unsigned char data, unsigned short addr, const char * err)
{
  packet.cmd = 'w';
  packet.u.addr = addr;
  packet.data = data;
  if (packet_action())
    {
      fprintf (stderr, ("Error in `write' packet.\n"
			"Command: %s\n"), err);
      return -1;
    }
  return 0;
}

int
mon_read (unsigned char *data, unsigned short addr, const char * err)
{
  packet.cmd = 'r';
  packet.u.addr = addr;
  if (packet_action())
    {
      fprintf (stderr, ("Error in `read' packet.\n"
			"Command: %s\n"), err);
      return -1;
    }
  *data = back_packet.data;
  return 0;
}
int
main(int argc, char **argv)
{
  int c;
  int gdb_session_p=0;
  char linebuf[1000];
  int sock,size,status;
  struct sockaddr_in clientname, name;
  char tstr[10];
  int main_argc = argc;
  char **main_argv = argv;

  tty_p = isatty (STDIN_FILENO);

  if (main_argc >= 2)
    {
      if (strcmp (main_argv[1], "-h") == 0)
	{
	  fprintf (stderr, "Usage: mon [option] [host] [port]\n");
	  return 0;
	}
      else if (strcmp (main_argv[1], "-d") == 0)
	{
	  --main_argc;
	  ++main_argv;
	  debug_p = 1;
	  mon_printf("Debug on\n");
	}
    }
  if (main_argc >= 2)
    {
      int PORT=11111;
      int ret;
      if (main_argc >= 3)
	{
	  char *tail;
	  int p = strtol (main_argv[2], &tail, 10);
	  if (main_argv[2] != tail)
	    PORT = p;
	  else
	    fprintf (stderr, "mon: error in port `%s' recognition.\n"
		     , main_argv[2]);
	}
      fprintf (stderr, "mon: restarted. port:host = %s:%d\n",
	       main_argv[1], PORT);
      init_sockaddr (&name, main_argv[1], PORT);
      sock = make_socket ( &name, PORT );
      if (listen (sock, 1) < 0)
	{
	  perror ("listen");
	  exit (EXIT_FAILURE);
	}
      /* Connection request on original socket.  */
      size = sizeof (clientname);
      if ((status = accept (sock, (struct sockaddr *) &clientname, &size)) < 0)
	{
	  perror ("accept");
	  exit (EXIT_FAILURE);
	}
      fprintf(stderr, "Server: connect from host %s, port %hd.\n",
	      inet_ntoa (clientname.sin_addr), ntohs (clientname.sin_port));

      /* torhr: Changed the socket from being non-blocking to being
         blocking. This is the only way I seem to get the redirection
         scheme to work... */

/*       ret = fcntl (status, F_SETFL, fcntl (status, F_GETFL, 0) */
/* 		   | O_NONBLOCK | FASYNC); */

      ret = fcntl (status, F_SETFL, (fcntl (status, F_GETFL, 0)
		   | FASYNC) & (~O_NONBLOCK));
      if (ret < 0)
	{
	  perror ("fcntl(NONBLOCK) for status");
	  exit (1);
	}

      /* torhr: The original way of redirecting stdin and stdout
         didn't work with my system. Rewrote it to use dup2() instead,
         which I believe is more portable. */

      ret = dup2(status, STDIN_FILENO);
      if (ret < 0)
	{
	  perror("dup2 stdin");
	  exit (EXIT_FAILURE);
	}
      ret = dup2(status, STDOUT_FILENO);
      if (ret < 0)
	{
	  perror("dup2 stdout");
	  exit (EXIT_FAILURE);
	}
      gdb_session_p = 1;
      tty_p = 0;
    }

  n_per_100ms = CalcDelay();  /* calibrate busy-wait loops */

#ifdef DEBUG
  fprintf(stderr, "n_per_100ms = %ld\n", n_per_100ms);
/*   fprintf(stderr, "5 sec test pause start....\n"); */
/*   Delay_10us(5 * 100000); */
/*   fprintf(stderr, "....end\n"); */
#endif /* DEBUG */

  if (ioperm (_LP_BASE, 8, 1) == 0)
    {

      //RESET_AVR;

      while (1)
	{
	  c = RecvByte ();
	  if (c != 0xfc)
	    {
	      if (gdb_session_p)
		fputc (c, stderr);
	      else
		putchar (c);
	    }
	  else
	    {
	      char *line;
	      int addr,data,ft=1;
	      static int first_time;
	      regs_array = RecvByte ();
	      regs_array |= RecvByte () << 8;
	      if (!first_time)
		{
		  mon_printf ("\nMonitor. (type `?' for help)\n");
		  fprintf (stderr,"mon: address of `regs' = 0x%x\n",
			   regs_array);
		  first_time = 1;
		}
	      
	      while (1)
		{
		  char * to_free = NULL;

		  to_free = line = readline ("db> ");
		  
		  if (!line)
		    exit (0);

		  if (gdb_session_p && debug_p)
		    {
		      if (debug_p)
			fprintf (stderr, "We got: %s\n", line);
		    }
		  
		  while (isblank (*line))
		    ++line;
		  if (!*line || *line == '\n' || *line == '\r')
		    continue;
		  packet.cmd = *line;
		  
		  if (sscanf (line, "w %i %i", &addr, &data) == 2
		      || sscanf (line, "o %i %i", &addr, &data) == 2)
		    {
		      if (*line == 'o')
			addr += 32; /* Skip mapped registers */
		      if (addr > 0x60 && *line == 'o')
			fprintf (stderr, "IO access out of range\n");
		      else
			mon_write (data, addr, line);
		    }
		  else if (sscanf (line, "%5[^=]=%i", tstr, &data) == 2
			   && (sscanf (tstr, "r%i", &addr) == 1
			       || (addr = 32, strcasecmp (tstr, "SREG") == 0)
			       || (addr = 33, strcasecmp (tstr, "SP") == 0)
			       || (addr = 35, strcasecmp (tstr, "PC") == 0)))
		    {
		      if (addr == 35)
			{
			  if (data & 1)
			    fprintf (stderr,
				     "Error: PC value must be even.\n");
			  data >>= 1;
			}
		      mon_write (data, addr + regs_array, line);
		      if (addr >= 33)
			mon_write (data >> 8, addr + regs_array + 1, line );
		    }
		  else if ((ft = sscanf (line, "r %i %i", &addr, &data)) >= 1
			   || (ft = sscanf (line, "i %i %i",
					    &addr, &data)) >= 1)
		    {
		      int i, j=0;
		      if (*line == 'i')
			addr += 32; /* Skip mapped registers */
		      if (ft == 1)
			data = 1;
		      if (addr & 0x8000000)
			{
			  packet.cmd = 'p';
			  addr ^= 0x8000000;
			}
		      else
			packet.cmd = 'r';
		      for (i=0; i < data; ++i)
			{
			  packet.u.addr = addr++;
			  if (packet_action())
			    fprintf (stderr, ("Error in `read' packet.\n"
					      "Command: %s\n"), line);
			  else if (gdb_session_p)
			    {
			      if (!j)
				{
				  mon_printf ("0x%04x", *line == 'i'
					  ? back_packet.u.addr - 32
					  : back_packet.u.addr);
				  j = 1;
				}
			      mon_printf (" 0x%x",back_packet.data);
			    }
			  else
			    {
			      mon_printf ("0x%04x 0x%02x  '%c'\n",
				      *line == 'i' ? back_packet.u.addr - 32
				      : back_packet.u.addr,
				      back_packet.data,
				      isprint (back_packet.data) ?
				      back_packet.data
				      : '.');
			    }
			}
		      if (gdb_session_p)
			mon_printf ("\n");
		    }
		  else if ((ft = sscanf (line, "p %i %i",
					 &addr, &data)) >= 1)
		    {
		      int i;
		      if (ft == 1)
			data = 1;
		      packet.cmd = 'p';
		      for (i=0; i < data; ++i)
			{
			  packet.u.addr = addr++;
			  if (packet_action()) {
			    fprintf (stderr, ("Error in `read' packet.\n"
					      "Command: %s\n"), line);
			  }
			  else
			    mon_printf ("0x%04x 0x%02x  '%c'\n",
				    back_packet.u.addr,
				    back_packet.data,
				    isprint (back_packet.data) ?
				    back_packet.data
				    : '.');
			}
		    }
		  else if ((ft = sscanf (line, "P %i %i",
					 &addr, &data)) >= 1)
		    {
		      int i;
		      if (ft == 1)
			data = 1;
		      packet.cmd = 'p';
		      addr -= addr % 2;
		      for (i=0; i < data; ++i)
			{
			  int cmd;
			  packet.u.addr = addr++;
			  if (packet_action())
			    {
			      fprintf (stderr, ("Error in `read' packet.\n"
						"Command: %s\n"), line);
			      break;
			    }
			  cmd = back_packet.data;
			  packet.u.addr = addr++;
			  if (packet_action())
			    {
			      fprintf (stderr, ("Error in `read' packet.\n"
						"Command: %s\n"), line);
			      break;
			    }
			  cmd += back_packet.data << 8;
			  mon_printf ("0x%04x 0x%04x\n",
				  back_packet.u.addr,
				  cmd);
			}
		    }
		  else if (sscanf (line, "bp %i", &addr) == 1)
		    {
		      int i;
		      unsigned char t,t1;
		      int done=0;

		      if (addr & 1)
			fprintf (stderr, "Address bust be even.\n");
		      else
			{
			  addr >>= 1;
			  for (i=0; i < NUM_BREAKPOINTS; ++i)
			    {
			      unsigned short avr_addr =
				regs_array + BREAKPOINTS_OFFSET + i * 2;
			      if (!mon_read (&t, avr_addr, line)
				  && !mon_read (&t1, avr_addr + 1, line))
				{
				  mon_write (addr, avr_addr, line);
				  mon_write (addr >> 8, avr_addr+1, line);
				  done = 1;
				  break;
				}
			    }
			  if (!done)
			    fprintf (stderr, "No room for breakpoint `%s'\n",
				     line);
			}
		    }
		  else if (sscanf (line, "bc %i", &addr) == 1)
		    {
		      int i;
		      unsigned char t,t1;
		      int done=0;
		      
		      if ((addr & 1) && addr != -1)
			fprintf (stderr, "Address bust be even.\n");
		      else
			{
			  addr >>= 1;
			  for (i=0; i < NUM_BREAKPOINTS; ++i)
			    {
			      unsigned short avr_addr =
				regs_array + BREAKPOINTS_OFFSET + i * 2;
			      if (!mon_read (&t, avr_addr, line)
				  && !mon_read (&t1, avr_addr + 1, line))
				if (addr == ((t1 << 8) | t) || addr == -1)
				  {
				    mon_write (0, avr_addr, line);
				    mon_write (0, avr_addr+1, line);
				    done = 1;
				    if (addr != -1)
				      break;
				  }
			    }
			  if (!done)
			    fprintf (stderr, "Can't find breakpoint `%s'\n",
				     line);
			}
		    }
		  else if (*line == 'q')
		    {
		      int i;
		      char t,t1;
		      int mode = 0;

		      for (i = 0; i < NUM_BREAKPOINTS; ++i)
			{
			  unsigned short avr_addr =
			    regs_array + BREAKPOINTS_OFFSET + i * 2;
			  if (!mon_read (&t, avr_addr, line)
			      && !mon_read (&t1, avr_addr + 1, line))
			    if (t || t1)
			      {
				mode = 2;
				break;
			      }
			}
		      packet.cmd = 'q';
		      packet.data = mode;
		      if (packet_action ())
			fprintf (stderr, "Error in 'q'\n");
		      else if (debug_p)
			{
			  if (mode == 0)
			    fprintf (stderr, "Continue...\n");
			  else
			    fprintf (stderr, "Stepping...\n");
			}
		      break;
		    }
		  else if (*line == 'n')
		    {
		      packet.cmd = 'q';
		      packet.data = 1;
		      if (packet_action ())
			fprintf (stderr, "Error in 'n'\n");
		      else if (debug_p)
			fprintf (stderr, "Step...\n");
		      break;
		    }
		  else if (*line == 'R')
		    {
		      dump_avr_registers ();
		    }
		  else if (*line == '?')
		    fprintf (stderr, MON_USAGE);
		  else
		    fprintf (stderr, "Error in command: %s\n", line);
		  if (to_free)
		    free (to_free);
		}
	    }
	  fflush (stdout);
	  fflush (stderr);
	}
    }
  else
    perror ("ioperm");
  return 0;
}

int gstrob;

#define strobe() (gstrob=(!!(RD_PORT & LP_POUTPA)))
#define pr_strobe() printf ("%d\n", gstrob)
test_lpp()
{
  int i;
  
  for (i=0; 1/*  i< 3000 */; ++i)
    {
      if (strobe())
	{
	  pr_strobe ();
	  while (strobe ());
	  pr_strobe ();
	}
      else
	{
	  pr_strobe ();
	  while (!strobe ());
	  pr_strobe ();
	}
    }
}
