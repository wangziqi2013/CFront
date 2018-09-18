
#include "bintree.h"
#include "error.h"

btnode_t *btnode_alloc(void *key, void *value) {
  btnode_t *node = (btnode_t *)malloc(sizeof(btnode_t));
  if(node == NULL) syserror(__func__);
  node->key = key, node->value = value;
  node->left = node->right = NULL;
  return node;
}
