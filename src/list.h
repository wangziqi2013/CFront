
#ifndef _LIST_H
#define _LIST_H

typedef struct listnode {
  void *key, *value;
  struct listnode *next;
} listnode_t;

typedef struct {
  listnode_t *head, *tail;
  int size;
} list_t;

list_t *list_init();
void list_free(list_t *list);
listnode_t *listnode_alloc();
void listnode_free(listnode_t *node);
void list_insert(list_t *list, void *key, void *value);

#endif