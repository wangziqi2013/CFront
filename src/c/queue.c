
#include "queue.h"

queue_t *queue_init() {
  queue_t *queue = (queue_t *)malloc(sizeof(queue_t));
  if(queue == NULL) perror(__func__);
  queue->enq = queue->deq = NULL;
  queue->size = 0;
  return queue;
}

void queue_free(queue_t *queue) {
  while(queue->deq != NULL) {
    queue->enq = queue->deq->next;
    free(deq);
    queue->deq = queue->enq;
  }
  free(queue);
}