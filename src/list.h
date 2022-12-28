
#ifndef _LIST_H
#define _LIST_H

#include "hashtable.h"

#define LIST_NOTFOUND ((void *)-1)  // Return value for find()

typedef struct listnode {
  void *key;               // No ownership
  void *value;             // No ownership
  struct listnode *next;
} listnode_t;

typedef struct {
  listnode_t *head;
  listnode_t *tail;
  int size;
} list_t;

inline static listnode_t *list_head(list_t *list) { return list->head; }
inline static listnode_t *list_tail(list_t *list) { return list->tail; }
inline static listnode_t *list_next(listnode_t *node) { return node->next; }
inline static void *list_key(listnode_t *node) { return node->key; }
inline static void *list_value(listnode_t *node) { return node->value; }

list_t *list_init();
void list_free(list_t *list);
int list_size(list_t *list);
listnode_t *listnode_alloc();
void listnode_free(listnode_t *node);
void *list_insert(list_t *list, void *key, void *value);
listnode_t *list_insertat(list_t *list, void *key, void *value, int index);
void *list_insert_nodup(list_t *list, void *key, void *value, eq_cb_t eq);
void *list_find(list_t *list, void *key, eq_cb_t eq);
const listnode_t *list_findat(list_t *list, int index);
void *list_remove(list_t *list, void *key, eq_cb_t eq);
void *list_removeat(list_t *list, int index, void **key);

#endif