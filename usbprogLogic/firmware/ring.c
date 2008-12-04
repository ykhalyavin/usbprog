#include "ring.h"

void ring_init (ring_t *f, char *buffer, const int size)
{
  f->count = 0;
  f->pread = f->pwrite = buffer;
  f->read2end = f->write2end = f->size = size;
}

int ring_put (ring_t *f, const char data)
{
  return _inline_ring_put (f, data);
}

char ring_get_wait (ring_t *f)
{
  while (!f->count);
    return _inline_ring_get (f);	
}

char ring_get_nowait (ring_t *f)
{
  if (!f->count) return -1;
  return _inline_ring_get (f);	
}
