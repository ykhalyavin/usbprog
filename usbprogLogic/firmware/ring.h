#ifndef _RING_H_
#define _RING_H_

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
} ring_t;



void ring_init (ring_t* f, char* buf, const int size);
int ring_put (ring_t* f, const char data);
char ring_get_wait (ring_t* f);
char ring_get_nowait (ring_t* f);

// swap function for spinlock mechanism
static inline void
_inline_swap_ring(char*x, char*y)
{
  char t;
  t = *x;
  *x = *y;
  *y = t;
}


// spinlock for ring access 
static inline void 
_inline_ring_lock(ring_t *f)
{
  char open=0x00;	
  while(open==0x00)
    _inline_swap_ring(&open,&f->spinlock);
}


//spinlock fpr ring access
static inline void 
_inline_ring_unlock(ring_t *f)
{
  f->spinlock=0xFF;
}

static inline int 
_inline_ring_put (ring_t *f, const char data)
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
	//_inline_ring_lock(f);
	f->count++;
	//_inline_ring_unlock(f);
	//SREG = sreg;
	
	return 1;
}

static inline int 
_inline_ring_get (ring_t *f)
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
	//_inline_ring_lock(f);
	//f->count--;
	//_inline_ring_unlock(f);
	//SREG = sreg;
	
	return data;
}

#endif /* _FIFO_H_ */
