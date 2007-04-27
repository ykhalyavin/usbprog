#include "fifo.h"

void fifo_init (fifo_t *f, char *buffer, const int size)
{
  f->count = 0;
  f->pread = f->pwrite = buffer;
  f->read2end = f->write2end = f->size = size;
  f->spinlock = 0xFF;
}

int fifo_put (fifo_t *f, const char data)
{
  return _inline_fifo_put (f, data);
}

char fifo_get_wait (fifo_t *f)
{
  while (!f->count);
    return _inline_fifo_get (f);	
}

char fifo_get_nowait (fifo_t *f)
{
  if (!f->count) return -1;
  return _inline_fifo_get (f);	
}
