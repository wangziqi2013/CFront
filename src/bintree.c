
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

bintree_t *bt_alloc(cmp_cb_t cmp) {
  bintree_t *bt = (bintree_t *)malloc(sizeof(bintree_t));
  if(bt == NULL) syserror(__func__);
  bt->cmp = cmp;
  bt->root = NULL;
  bt->size = 0;
  return bt;
}
void bt_free(bintree_t *bt) { free(bt); }
bintree_t *bt_str_alloc() { return bt_alloc(strcmp_cb, streq_cb); }

// Insert the key, or return an existing key
btnode_t *bt_insert(bintree_t *bt, void *key, void *value) {
  btnode_t *found = NULL;
  bt->root = _bt_insert(bt, bt->root, key, value, &found);
  return found;
}
btnode_t *_bt_insert(bintree_t *bt, btnode_t *node, void *key, void *value, btnode_t **found) {
  if(node == NULL) { bt->size++; *found = btnode_alloc(key, value); return *found; } // Creates a new node
  int cmp = bt->cmp(key, node->key);
  if(cmp == 0) *found = node;
  else if(cmp < 0) node->left = _bt_insert(bt, node->left, key, value);
  else node->right = _bt_insert(bt, node->right, key, value);
  return node;
}
