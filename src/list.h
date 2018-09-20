
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

list_t *list_init();
list_t *list_free(list_t *list);
listnode_t *listnode_alloc();
void listnode_free(listnode_t *node);

#endif