
#include <stdio.h>
#include <stdlib.h>
#include "error.h"
#include "list.h"

list_t *list_init() {
  list_t *list = (list_t *)malloc(sizeof(list_t));
  if(list == NULL) syserror(__func__);
  list->size = 0;
  list->head = NULL;
  return list;
}

list_t *list_free(list_t *list) {
  listnode_t *node = list->head, *prev = node;
  if(node != NULL)  {
    node = node->next;
    while(node != NULL) {
      listnode_free(prev);
      prev = node;
      node = node->next;
    }
    listnode_free(prev); // The last node is freed here
  }
  free(list);
  return;
}

// Allocate a node. All fields are uninitialized
listnode_t *listnode_alloc() {
  listnode_t *node = (listnode_t *)malloc(sizeof(listnode_t));
  if(node == NULL) syserror(__func__);
  return node;
}
void listnode_free(listnode_t *node) { free(node); }