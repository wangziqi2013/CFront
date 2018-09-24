
#include "bintree.h"

btnode_t *btnode_alloc(void *key, void *value) {
  btnode_t *node = (btnode_t *)malloc(sizeof(btnode_t));
  SYSEXPECT(node != NULL);
  node->key = key, node->value = value;
  node->left = node->right = NULL;
  return node;
}
void btnode_free(btnode_t *node) { free(node); }

bintree_t *bt_init(cmp_cb_t cmp) {
  bintree_t *bt = (bintree_t *)malloc(sizeof(bintree_t));
  SYSEXPECT(bt != NULL);
  bt->cmp = cmp;
  bt->root = NULL;
  bt->size = 0;
  return bt;
}
void bt_free(bintree_t *bt) { _bt_free(bt->root); free(bt); }
void _bt_free(btnode_t *node) {
  if(node == NULL) return;
  _bt_free(node->left);
  _bt_free(node->right);
  btnode_free(node);
  return;
}
bintree_t *bt_str_init() { return bt_init(strcmp_cb); }

int bt_size(bintree_t *bt) { return bt->size; }

// Insert the key, or return an existing key
void *bt_insert(bintree_t *bt, void *key, void *value) {
  btnode_t *found = NULL; // Set to new node if inserted, otherwise set to 
  bt->root = _bt_insert(bt, bt->root, key, value, &found);
  return found->value;
}
btnode_t *_bt_insert(bintree_t *bt, btnode_t *node, void *key, void *value, btnode_t **found) {
  if(node == NULL) { bt->size++; *found = btnode_alloc(key, value); return *found; } // Creates a new node
  int cmp = bt->cmp(key, node->key);
  if(cmp == 0) *found = node;
  else if(cmp < 0) node->left = _bt_insert(bt, node->left, key, value, found);
  else node->right = _bt_insert(bt, node->right, key, value, found);
  return node;
}

// Return BT_NOTFOUND if not found, otherwise return the value
void *bt_find(bintree_t *bt, void *key) { return _bt_find(bt, bt->root, key); }
void *_bt_find(bintree_t *bt, btnode_t *node, void *key) {
  if(node == NULL) return BT_NOTFOUND;
  int cmp = bt->cmp(key, node->key);
  if(cmp == 0) return node->value;
  else if(cmp < 0) return _bt_find(bt, node->left, key);
  else return _bt_find(bt, node->right, key);
}

// Removes the given key, and returns the value if the key exists; otherwise return BT_NOTFOUND
void *bt_remove(bintree_t *bt, void *key) { 
  void *found = BT_NOTFOUND;
  bt->root = _bt_remove(bt, bt->root, key, &found); 
  return found;
}

// Returns the child after performing remove
void *_bt_remove(bintree_t *bt, btnode_t *node, void *key, void **found) {
  if(node == NULL) { *found = BT_NOTFOUND; return NULL; }
  int cmp = bt->cmp(key, node->key);
  if(cmp == 0) { *found = node->value; bt->size--; return _bt_remove_node(bt, node); }
  else if(cmp < 0) node->left = _bt_remove(bt, node->left, key, found);
  else node->right = _bt_remove(bt, node->right, key, found);
  return node;
}

// Internal function only called by bt_remove()
void *_bt_remove_node(bintree_t *bt, btnode_t *node) {
  btnode_t *left = node->left, *right = node->right;
  if(left == NULL) { btnode_free(node); return right; } // This also covers the leaf node case
  else if(right == NULL) { btnode_free(node); return left; }
  if(right->left == NULL) {
    btnode_free(node);
    right->left = left;
    return right;
  }
  do { left = right; right = right->left; } while(right->left);
  node->key = right->key; node->value = right->value;
  left->left = right->right;
  btnode_free(right);
  return node;
}