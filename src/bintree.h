
#ifndef _BIN_TREE_H
#define _BIN_TREE_H

#include "hashtable.h"

// Binary tree node type
typedef struct btnode {
  void *key, *value;
  struct btnode *left, *right;
} btnode_t;

typedef struct {
  int size;
  cmp_cb_t cmp;
  eq_cb_t eq;
  btnode_t *root;
} bintree_t;

btnode_t *btnode_alloc(void *key, void *value);
void btnode_free(btnode_t *node);
bintree_t *bt_alloc(cmp_cb_t cmp, eq_cb_t eq);
void bt_free(bintree_t *bt);

#endif