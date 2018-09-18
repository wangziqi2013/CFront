
#ifndef _BIN_TREE_H
#define _BIN_TREE_H

#include "hashtable.h"

#define BT_NOTFOUND ((void *)-1)

// Binary tree node type
typedef struct btnode {
  void *key, *value;
  struct btnode *left, *right;
} btnode_t;

// The good thing about a binary tree search structure is that the physical size
// grows proportionally with the logical size, which is desirable for structures
// that are usually small, but sometimes huge
typedef struct {
  int size;
  cmp_cb_t cmp;
  btnode_t *root;
} bintree_t;

btnode_t *btnode_alloc(void *key, void *value);
void btnode_free(btnode_t *node);
bintree_t *bt_alloc(cmp_cb_t cmp);
void bt_free(bintree_t *bt);
bintree_t *bt_str_alloc();
void *bt_insert(bintree_t *bt, void *key, void *value);
btnode_t *_bt_insert(bintree_t *bt, btnode_t *node, void *key, void *value, btnode_t **found);
void *bt_find(bintree_t *bt, void *key);
void *_bt_find(bintree_t *bt, btnode_t *node, void *key);

#endif