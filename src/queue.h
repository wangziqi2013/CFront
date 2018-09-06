
#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct queue_node_t {
  void *p;
  struct queue_node_t *next;
} queue_node_t;

typedef struct queue_t {
  // Pointer points from deq to enq
  queue_node_t *enq;
  queue_node_t *deq;
  int size;
} queue_t;

queue_t *queue_init();
void queue_free(queue_t *queue);

#endif
