#ifndef FIFO_H_
#define FIFO_H_

#include <stdint.h>

typedef struct {
	volatile uint8_t _tail;
	volatile uint8_t _head;
	volatile uint8_t* _buffer;
} FIFO_t;

typedef struct {
	volatile uint8_t _tail;
	volatile uint8_t _head;
	volatile uint8_t _buffer[64];
} FIFO64_t;

typedef struct {
	uint8_t _tail;
	uint8_t _head;
	uint8_t _buffer[128];
} FIFO128_t;

#define FIFO_init(fifo)			{ fifo._tail = 0; fifo._head = 0; }

#define FIFO_available(fifo)	( fifo._tail != fifo._head )

#define FIFO_length(fifo, size)	( fifo._head >= fifo._tail ? fifo._head - fifo._tail : size - fifo._tail + fifo._head )

#define FIFO_read(fifo, size) (																	\
	(FIFO_available(fifo)) ?																	\
		fifo._buffer[fifo._tail = (fifo._tail + 1) & (size-1)] :								\
		0																						\
)

#define FIFO_write(fifo, data, size) {															\
	uint8_t tmphead = ( fifo._head + 1 ) & (size-1); 	/* calculate buffer index */			\
	if(tmphead != fifo._tail) {							/* if buffer is full, return false */	\
		fifo._head = tmphead;							/* store new index */					\
		fifo._buffer[tmphead] = data;					/* store data in buffer */				\
	}																							\
}

#define FIFO64_read(fifo)				FIFO_read(fifo, 64)
#define FIFO64_write(fifo, data)		FIFO_write(fifo, data, 64)

#endif /*FIFO_H_*/
