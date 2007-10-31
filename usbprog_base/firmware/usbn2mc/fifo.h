#ifndef _FIFO_H_
#define _FIFO_H_

// Benedikt Sauter <sauter@ixbat.de> (2006-03-26)

// mainly taken from http://www.roboternetz.de/wissen/index.php/FIFO_mit_avr-gcc
// Author:  removed avr specially things, and added spinlock mechanism

typedef struct
{
  int volatile count;       // number of data in buffer 
  int size;                 // buffer-size 
  char *pread;              // read pointer 
  char *pwrite;             // write pointer 
  int read2end, write2end;  // number of free signs till overlow
  char spinlock;
} fifo_t;



void fifo_init (fifo_t*, char* buf, const int size);
int fifo_put (fifo_t*, const char data);
char fifo_get_wait (fifo_t*);
int fifo_get_nowait (fifo_t*, char *entry);

// swap function for spinlock mechanism
static inline void
_inline_swap(char*x, char*y)
{
  char t;
  t = *x;
  *x = *y;
  *y = t;
}


// spinlock for fifo access 
static inline void 
_inline_fifo_lock(fifo_t *f)
{
  char open=0x00;	
  while(open==0x00)
    _inline_swap(&open,&f->spinlock);
}


//spinlock fpr fifo access
static inline void 
_inline_fifo_unlock(fifo_t *f)
{
  f->spinlock=0xFF;
}

static inline int 
_inline_fifo_put (fifo_t *f, const char data)
{
	if (f->count >= f->size)
		return 0;
		
	char * pwrite = f->pwrite;
	
	*(pwrite++) = data;
	
	int write2end = f->write2end;
	
	if (--write2end == 0)
	{
		write2end = f->size;
		pwrite -= write2end;
	}
	
	f->write2end = write2end;
	f->pwrite = pwrite;

	//uint8_t sreg = SREG;
	//cli();-
	//_inline_fifo_lock(f);
	f->count++;
	//_inline_fifo_unlock(f);
	//SREG = sreg;
	
	return 1;
}

static inline int 
_inline_fifo_get (fifo_t *f)
{
	char *pread = f->pread;
	char data = *(pread++);
	int read2end = f->read2end;
	
	if (--read2end == 0)
	{
		read2end = f->size;
		pread -= read2end;
	}
	
	f->pread = pread;
	f->read2end = read2end;
	
	//uint8_t sreg = SREG;
	//cli();
	//_inline_fifo_lock(f);
	f->count--;
	//_inline_fifo_unlock(f);
	//SREG = sreg;
	
	return data;
}

#endif /* _FIFO_H_ */
