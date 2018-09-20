
#ifndef _LIST_H
#define _LIST_H

typedef struct listnode {
  void *key, *value;
  struct listnode *next;
} listnode_t;

typedef struct {
  listnode_t *head;
  int size;
} list_t;

#endif