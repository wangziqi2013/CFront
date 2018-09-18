
#include "bintree.h"
#include "error.h"

btnode_t *btnode_alloc(void *key, void *value) {
  btnode_t *node = (btnode_t *)malloc(sizeof(btnode_t));
  if(node == NULL) syserror(__func__);
  node->key = key, node->value = value;
  node->left = node->right = NULL;
  return node;
}

void btnode_free(btnode_t *node) { free(node); }
bintree_t *bt_alloc(cmp_cb_t cmp, eq_cb_t eq) {
  bintree_t *bt = (bintree_t *)malloc(sizeof(bintree_t));
  if(bt == NULL) syserror(__func__);
  bt->cmp = cmp, bt->eq = eq;
  bt->root = NULL;
  bt->size = 0;
  return bt;
}

