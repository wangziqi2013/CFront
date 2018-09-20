
#ifndef _LIST_H
#define _LIST_H

#include "hashtable.h"

#define LIST_NOTFOUND ((void *)-1)  // Return value for find()

typedef struct listnode {
  void *key, *value;
  struct listnode *next;
} listnode_t;

typedef struct {
  listnode_t *head, *tail;
  int size;
  eq_cb_t eq;   // Call back function for comparing keys
} list_t;

list_t *list_init(eq_cb_t eq);
list_t *list_str_init();
void list_free(list_t *list);
listnode_t *listnode_alloc();
void listnode_free(listnode_t *node);
void list_insert(list_t *list, void *key, void *value);
void list_insert_nodup(list_t *list, void *key, void *value);
void *list_find(list_t *list, void *key);
void *list_remove(list_t *list, void *key);

#endif