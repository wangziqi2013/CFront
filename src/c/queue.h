
#ifndef _QUEUE_H
#define _QUEUE_H

typedef struct queue_t {
  // Pointer points from deq to enq
  struct queue_t *enq;
  struct queue_t *deq;
  int size;
} queue_t;

#endif
